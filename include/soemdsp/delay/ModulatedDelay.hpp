#pragma once

// Atom: ModulatedDelay (reference leaf)
//
// Pattern:
//   1. constexpr kParams[]  — meta next to the class
//   2. mem[base+slot]       — control storage (derived / rate)
//   3. *Changed()           — rebuild cached coeffs only
//   4. process*(in, live…)  — hot path; LIVE values passed as args (no connect())
//
// Live (passed each process call, not aliased via connect/push):
//   feedback, lfoAmp
// Control (*Changed):
//   delayTimeSeconds, diffusionRandom, lfoFrequencyHz, lfoVariation, sampleRate

#include <cmath>
#include <cstdint>

#include <soemdsp/dsp/AtomParam.hpp>

namespace soemdsp::delay {

struct ModulatedDelay
{
    static constexpr soemdsp::dsp::AtomParam kParams[] = {
      { 0, "delayTimeSeconds", "Delay Time", 0.06, 0.0, 4.0, "s", soemdsp::dsp::ParamKind::Control, "delayTimeChanged" },
      { 1, "diffusionRandom", "Diff Random", 1.0, 0.0, 1.0, "", soemdsp::dsp::ParamKind::Control, "diffusionSeedChanged" },
      { 2, "lfoFrequencyHz", "LFO Hz", 0.5, 0.0, 90.0, "Hz", soemdsp::dsp::ParamKind::Control, "lfoChanged" },
      { 3, "lfoVariation", "LFO Vary", 1.0, 0.0, 10.0, "", soemdsp::dsp::ParamKind::Control, "lfoChanged" },
      { 4, "sampleRate", "Sample Rate", 48000.0, 8000.0, 192000.0, "Hz", soemdsp::dsp::ParamKind::Control, "sampleRateChanged" },
    };
    static constexpr std::uint32_t kControlCount = 5;

    inline double& delayTimeSeconds_() noexcept { return mem[base + 0]; }
    inline double& diffusionRandom_() noexcept { return mem[base + 1]; }
    inline double& lfoFrequencyHz_() noexcept { return mem[base + 2]; }
    inline double& lfoVariation_() noexcept { return mem[base + 3]; }
    inline double& sampleRate_() noexcept { return mem[base + 4]; }

    void setBuffer(float* buffer, int capacity) noexcept
    {
        buffer_ = buffer;
        bufferCap_ = capacity > 2 ? capacity : 2;
        if (bufferSize_ > bufferCap_)
        {
            bufferSize_ = bufferCap_;
        }
    }

    static double parabol(double phase01) noexcept
    {
        const double x = phase01 - std::floor(phase01);
        const double fit = std::fmod(2.0 * x, 2.0) - 1.0;
        return 4.0 * fit * (1.0 - std::fabs(fit));
    }

    void sampleRateChanged() noexcept
    {
        double sr = sampleRate_();
        if (!(sr > 1.0))
        {
            sr = 44100.0;
        }
        bufferSize_ = static_cast<int>(sr);
        if (bufferSize_ < 2)
        {
            bufferSize_ = 2;
        }
        if (bufferSize_ > bufferCap_)
        {
            bufferSize_ = bufferCap_;
        }
        delayTimeChanged();
        lfoChanged();
    }

    void delayTimeChanged() noexcept
    {
        const double sr = sampleRate_() > 1.0 ? sampleRate_() : 44100.0;
        delaySamples_ = delayTimeSeconds_() * diffusionRandom_() * sr;
        clampDelaySamples();
    }

    void echoTimeChanged() noexcept
    {
        const double sr = sampleRate_() > 1.0 ? sampleRate_() : 44100.0;
        delaySamples_ = delayTimeSeconds_() * sr;
        clampDelaySamples();
    }

    void diffusionSeedChanged(double unipolarRandom) noexcept
    {
        double u = unipolarRandom;
        if (u < 0.0)
        {
            u = 0.0;
        }
        if (u > 1.0)
        {
            u = 1.0;
        }
        diffusionRandom_() = u;
        delayTimeChanged();
    }

    void lfoChanged() noexcept
    {
        const double sr = sampleRate_() > 1.0 ? sampleRate_() : 44100.0;
        double hz = lfoFrequencyHz_();
        if (!(hz * 0.0 == 0.0) || hz < 0.0)
        {
            hz = 0.0;
        }
        lfoInc_ = hz / sr;
    }

    void syncControlParams() noexcept
    {
        const double delayTime = delayTimeSeconds_();
        const double diffusionRandom = diffusionRandom_();
        const double lfoFrequencyHz = lfoFrequencyHz_();
        const double lfoVariation = lfoVariation_();
        const double sampleRate = sampleRate_();

        if (sampleRate != lastSampleRate_)
        {
            lastSampleRate_ = sampleRate;
            lastDelayTime_ = delayTime;
            lastDiffusionRandom_ = diffusionRandom;
            lastLfoFrequencyHz_ = lfoFrequencyHz;
            lastLfoVariation_ = lfoVariation;
            sampleRateChanged();
            return;
        }
        if (delayTime != lastDelayTime_ || diffusionRandom != lastDiffusionRandom_)
        {
            lastDelayTime_ = delayTime;
            lastDiffusionRandom_ = diffusionRandom;
            delayTimeChanged();
        }
        if (lfoFrequencyHz != lastLfoFrequencyHz_ || lfoVariation != lastLfoVariation_)
        {
            lastLfoFrequencyHz_ = lfoFrequencyHz;
            lastLfoVariation_ = lfoVariation;
            lfoChanged();
        }
    }

    void reset(double bipolarPhase = 0.0) noexcept
    {
        bufferPos_ = 0;
        lfoPhase_ = bipolarPhase;
        if (buffer_ != nullptr)
        {
            for (int i = 0; i < bufferCap_; ++i)
            {
                buffer_[i] = 0.0f;
            }
        }
        out_ = 0.0;
        lfo_ = 0.0;
    }

    // LIVE: lfoAmp passed in (original Wire read every sample).
    double processDelay(double in, double lfoAmp) noexcept
    {
        if (buffer_ == nullptr || bufferSize_ < 2)
        {
            return in;
        }
        lfo_ = advanceLfo(lfoAmp);
        double offset = delaySamples_ - (delaySamples_ * (lfo_ * lfoAmp)) + 1.0;
        if (offset < 1.0)
        {
            offset = 1.0;
        }
        bufferPos_ = (bufferPos_ + 1) % bufferSize_;
        out_ = readAt(offset);
        buffer_[bufferPos_] = static_cast<float>(in);
        return out_;
    }

    // LIVE: feedback + lfoAmp passed in (original Wire aliases).
    double processDiffuse(double in, double feedback, double lfoAmp) noexcept
    {
        if (buffer_ == nullptr || bufferSize_ < 2)
        {
            return in;
        }
        lfo_ = advanceLfo(lfoAmp);
        double offset = delaySamples_ - (delaySamples_ * (lfo_ * lfoAmp)) + 1.0;
        if (offset < 1.0)
        {
            offset = 1.0;
        }
        bufferPos_ = (bufferPos_ + 1) % bufferSize_;
        out_ = readAt(offset);
        buffer_[bufferPos_] = static_cast<float>((0.0 - in) - out_ * feedback);
        out_ = in * feedback - out_ * (1.0 - feedback * feedback);
        return out_;
    }

    double* mem = nullptr;
    std::uint32_t base{};

    float* buffer_ = nullptr;
    int bufferCap_{};
    int bufferSize_{ 2 };

    double lastDelayTime_{};
    double lastDiffusionRandom_{ 1.0 };
    double lastLfoFrequencyHz_{};
    double lastLfoVariation_{};
    double lastSampleRate_{};

    double delaySamples_{ 1.0 };
    double lfoInc_{};
    double lfoPhase_{};
    int bufferPos_{};
    double out_{};
    double lfo_{};

  private:
    void clampDelaySamples() noexcept
    {
        if (delaySamples_ < 1.0)
        {
            delaySamples_ = 1.0;
        }
        const double maxS = static_cast<double>(bufferSize_ - 1);
        if (delaySamples_ > maxS)
        {
            delaySamples_ = maxS;
        }
    }

    double advanceLfo(double lfoAmp) noexcept
    {
        lfoPhase_ += lfoInc_;
        lfoPhase_ -= std::floor(lfoPhase_);
        return lfoAmp * (0.5 * (parabol(lfoPhase_) + 1.0));
    }

    double readAt(double offset) const noexcept
    {
        const double where =
          std::fmod(static_cast<double>(bufferPos_) + static_cast<double>(bufferSize_) - offset,
                    static_cast<double>(bufferSize_));
        const int before = static_cast<int>(where) % bufferSize_;
        const int after = (before + 1) % bufferSize_;
        const double mix = where - std::floor(where);
        return static_cast<double>(buffer_[before]) * (1.0 - mix)
             + static_cast<double>(buffer_[after]) * mix;
    }
};

} // namespace soemdsp::delay
