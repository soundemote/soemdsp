#pragma once

// Atom: PingPongDelay (tape-style stereo ping-pong)
// See docs/DSP_ATOM_PARADIGM.md — kParams in this file; Live vs Control.

#include <cmath>
#include <cstdint>

#include <soemdsp/dsp/AtomParam.hpp>
#include <soemdsp/dynamics/SoftClipperAtom.hpp>
#include <soemdsp/filter/OnePoleHpAtom.hpp>

namespace soemdsp::delay {

struct PingPongDelay
{
    static constexpr int kMaxBufferSamples = 1536002; // 8 s @ 192 kHz
    static constexpr double kMaxDelaySeconds = 8.0;

    static constexpr soemdsp::dsp::AtomParam kParams[] = {
      { 0, "feedback", "Feedback", 0.5, 0.0, 2.0, "", soemdsp::dsp::ParamKind::Live, "" },
      { 1, "mix", "Mix", 0.5, 0.0, 1.0, "", soemdsp::dsp::ParamKind::Live, "" },
      { 2, "level", "Level", 1.0, 0.0, 2.0, "", soemdsp::dsp::ParamKind::Live, "" },
      { 3, "timeNumerator", "Time Num", 1.0, 0.0, 64.0, "", soemdsp::dsp::ParamKind::Control, "timingChanged" },
      { 4, "timeDenominator", "Time Den", 4.0, 1.0, 64.0, "", soemdsp::dsp::ParamKind::Control, "timingChanged" },
      { 5, "timingMode", "Timing", 0.0, 0.0, 2.0, "", soemdsp::dsp::ParamKind::Control, "timingChanged" },
      { 6, "offsetMs", "Offset", 0.0, 0.0, 500.0, "ms", soemdsp::dsp::ParamKind::Live, "" },
      { 7, "lfoStyle", "LFO Style", 0.0, 0.0, 2.0, "", soemdsp::dsp::ParamKind::Live, "" },
      { 8, "lfoRate", "LFO Rate", 0.5, 0.0, 40.0, "Hz", soemdsp::dsp::ParamKind::Live, "" },
      { 9, "lfoVariation", "LFO Vary", 0.0, 0.0, 1.0, "", soemdsp::dsp::ParamKind::Live, "" },
      { 10, "saturate", "Saturate", 1.0, 0.01, 4.0, "", soemdsp::dsp::ParamKind::Control, "saturateChanged" },
      { 11, "lpfFrequency", "LPF", 8000.0, 20.0, 20000.0, "Hz", soemdsp::dsp::ParamKind::Control, "filtersChanged" },
      { 12, "hpfFrequency", "HPF", 20.0, 1.0, 2000.0, "Hz", soemdsp::dsp::ParamKind::Control, "filtersChanged" },
      { 13, "tempoBpm", "Tempo", 120.0, 20.0, 400.0, "bpm", soemdsp::dsp::ParamKind::Control, "timingChanged" },
      { 14, "sampleRate", "Sample Rate", 48000.0, 8000.0, 192000.0, "Hz", soemdsp::dsp::ParamKind::Control, "sampleRateChanged" },
    };
    static constexpr std::uint32_t kControlCount =
      sizeof(kParams) / sizeof(kParams[0]);

    inline double& feedback_() noexcept { return mem[base + 0]; }
    inline double& mix_() noexcept { return mem[base + 1]; }
    inline double& level_() noexcept { return mem[base + 2]; }
    inline double& timeNumerator_() noexcept { return mem[base + 3]; }
    inline double& timeDenominator_() noexcept { return mem[base + 4]; }
    inline double& timingMode_() noexcept { return mem[base + 5]; }
    inline double& offsetMs_() noexcept { return mem[base + 6]; }
    inline double& lfoStyle_() noexcept { return mem[base + 7]; }
    inline double& lfoRate_() noexcept { return mem[base + 8]; }
    inline double& lfoVariation_() noexcept { return mem[base + 9]; }
    inline double& saturate_() noexcept { return mem[base + 10]; }
    inline double& lpfFrequency_() noexcept { return mem[base + 11]; }
    inline double& hpfFrequency_() noexcept { return mem[base + 12]; }
    inline double& tempoBpm_() noexcept { return mem[base + 13]; }
    inline double& sampleRate_() noexcept { return mem[base + 14]; }

    void setBuffers(float* left, float* right, int capacity) noexcept
    {
        bufferL_ = left;
        bufferR_ = right;
        bufferCap_ = capacity > 2 ? capacity : 2;
    }

    void initDefaults() noexcept
    {
        for (const auto& p : kParams)
        {
            mem[base + p.slot] = p.def;
        }
        bindFilters();
        sampleRateChanged();
        saturateChanged();
        filtersChanged();
        timingChanged();
        reset();
    }

    void bindFilters() noexcept
    {
        clipL_.mem = clipMemL_;
        clipR_.mem = clipMemR_;
        clipL_.base = clipR_.base = 0;
        hpfL_.mem = hpfMemL_;
        hpfR_.mem = hpfMemR_;
        hpfL_.base = hpfR_.base = 0;
    }

    void sampleRateChanged() noexcept
    {
        const double sr = sampleRate_() > 1.0 ? sampleRate_() : 44100.0;
        int required = static_cast<int>(sr * kMaxDelaySeconds) + 2;
        if (required > bufferCap_)
        {
            required = bufferCap_;
        }
        if (required < 2)
        {
            required = 2;
        }
        if (required != bufferSize_)
        {
            bufferSize_ = required;
            reset();
        }
        hpfMemL_[1] = sr;
        hpfMemR_[1] = sr;
        filtersChanged();
        timingChanged();
    }

    void saturateChanged() noexcept
    {
        const double sat = saturate_() > 0.01 ? saturate_() : 0.01;
        clipMemL_[0] = 0.0;
        clipMemL_[1] = sat * 2.0;
        clipMemR_[0] = 0.0;
        clipMemR_[1] = sat * 2.0;
        clipL_.updateCoeffs();
        clipR_.updateCoeffs();
    }

    void filtersChanged() noexcept
    {
        const double sr = sampleRate_() > 1.0 ? sampleRate_() : 44100.0;
        hpfMemL_[0] = hpfFrequency_();
        hpfMemR_[0] = hpfFrequency_();
        hpfMemL_[1] = sr;
        hpfMemR_[1] = sr;
        hpfL_.frequencyChanged();
        hpfR_.frequencyChanged();

        // Cached one-pole LP coeffs (Control).
        double w = 6.283185307179586 / sr;
        if (w > 0.000142475857)
        {
            w = 0.000142475857;
        }
        w *= lpfFrequency_();
        lpA1_ = std::exp(-w);
    }

    void timingChanged() noexcept
    {
        const double bpm = tempoBpm_() > 1.0 ? tempoBpm_() : 120.0;
        const double secondsPerWhole = 240.0 / bpm;
        const double num = timeNumerator_() > 0.0 ? timeNumerator_() : 0.0;
        const double den = timeDenominator_() > 1.0 ? timeDenominator_() : 1.0;
        double modeMul = 1.0;
        const int mode = static_cast<int>(timingMode_() + 0.5);
        if (mode == 1)
        {
            modeMul = 1.5;
        }
        else if (mode == 2)
        {
            modeMul = 2.0 / 3.0;
        }
        baseSeconds_ = (num <= 0.0) ? 0.0 : secondsPerWhole * (num / den) * modeMul;
    }

    void syncControlParams() noexcept
    {
        const double saturate = saturate_();
        const double lpf = lpfFrequency_();
        const double hpf = hpfFrequency_();
        const double num = timeNumerator_();
        const double den = timeDenominator_();
        const double mode = timingMode_();
        const double bpm = tempoBpm_();
        const double sr = sampleRate_();

        if (sr != lastSampleRate_)
        {
            lastSampleRate_ = sr;
            sampleRateChanged();
            lastSaturate_ = saturate;
            lastLpf_ = lpf;
            lastHpf_ = hpf;
            lastNum_ = num;
            lastDen_ = den;
            lastMode_ = mode;
            lastBpm_ = bpm;
            return;
        }
        if (saturate != lastSaturate_)
        {
            lastSaturate_ = saturate;
            saturateChanged();
        }
        if (lpf != lastLpf_ || hpf != lastHpf_)
        {
            lastLpf_ = lpf;
            lastHpf_ = hpf;
            filtersChanged();
        }
        if (num != lastNum_ || den != lastDen_ || mode != lastMode_ || bpm != lastBpm_)
        {
            lastNum_ = num;
            lastDen_ = den;
            lastMode_ = mode;
            lastBpm_ = bpm;
            timingChanged();
        }
    }

    void reset() noexcept
    {
        if (bufferL_ && bufferR_)
        {
            for (int i = 0; i < bufferSize_; ++i)
            {
                bufferL_[i] = 0.0f;
                bufferR_[i] = 0.0f;
            }
        }
        position_ = 0;
        lpZL_ = lpZR_ = 0.0;
        hpfL_.reset();
        hpfR_.reset();
        outL_ = outR_ = 0.0;
        lfoPhaseL_ = 0.0;
        lfoPhaseR_ = 0.37;
    }

    // LIVE: feedback, mix, level, offsetMs, lfo* read from mem each call.
    void process(double input) noexcept
    {
        const double sr = sampleRate_() > 1.0 ? sampleRate_() : 44100.0;
        const double dry = input;
        const double fb = feedback_();
        const double mix = mix_();
        const double level = level_();
        const double driftSec = (offsetMs_() > 0.0 ? offsetMs_() : 0.0) * 0.001;
        const double hz = lfoRate_();
        const double vary = lfoVariation_();

        const double rateL = hz * (1.0 + vary * 0.31);
        const double rateR = hz * (1.0 - vary * 0.27);
        const double modL = driftSec > 1.0e-9 ? advanceParabol(lfoPhaseL_, rateL, sr) : 0.0;
        const double modR = driftSec > 1.0e-9 ? advanceParabol(lfoPhaseR_, rateR, sr) : 0.0;

        double delayL = (baseSeconds_ + driftSec * modL) * sr;
        double delayR = (baseSeconds_ + driftSec * modR) * sr;
        if (delayL < 1.0)
        {
            delayL = 1.0;
        }
        if (delayR < 1.0)
        {
            delayR = 1.0;
        }
        const double maxD = static_cast<double>(bufferSize_ - 2);
        if (delayL > maxD)
        {
            delayL = maxD;
        }
        if (delayR > maxD)
        {
            delayR = maxD;
        }

        position_ = (position_ + 1) % bufferSize_;
        const double readL = readAt(bufferL_, position_, delayL);
        const double readR = readAt(bufferR_, position_, delayR);

        double writeL = clipL_.process(dry + readR * fb);
        double writeR = clipR_.process(readL * fb);
        writeL = hpfL_.process(writeL);
        writeR = hpfR_.process(writeR);
        writeL = lpZL_ = (1.0 - lpA1_) * writeL + lpA1_ * lpZL_;
        writeR = lpZR_ = (1.0 - lpA1_) * writeR + lpA1_ * lpZR_;

        bufferL_[position_] = static_cast<float>(clamp8(writeL));
        bufferR_[position_] = static_cast<float>(clamp8(writeR));

        outL_ = (dry * (1.0 - mix) + readL * mix) * level;
        outR_ = (dry * (1.0 - mix) + readR * mix) * level;
    }

    double* mem = nullptr;
    std::uint32_t base{};
    double outL_{};
    double outR_{};

  private:
    static double clamp8(double v) noexcept
    {
        if (v > 8.0)
        {
            return 8.0;
        }
        if (v < -8.0)
        {
            return -8.0;
        }
        return v;
    }

    static double advanceParabol(double& phase, double hz, double sr) noexcept
    {
        phase += hz / sr;
        phase -= std::floor(phase);
        const double fit = std::fmod(2.0 * phase, 2.0) - 1.0;
        return 4.0 * fit * (1.0 - std::fabs(fit)); // bipolar-ish; caller scales
    }

    double readAt(const float* buf, int pos, double delaySamples) const noexcept
    {
        double where = static_cast<double>(pos) + static_cast<double>(bufferSize_) - delaySamples;
        where = std::fmod(where, static_cast<double>(bufferSize_));
        if (where < 0.0)
        {
            where += static_cast<double>(bufferSize_);
        }
        const int before = static_cast<int>(where) % bufferSize_;
        const int after = (before + 1) % bufferSize_;
        const double mix = where - std::floor(where);
        return static_cast<double>(buf[before]) * (1.0 - mix)
             + static_cast<double>(buf[after]) * mix;
    }

    float* bufferL_ = nullptr;
    float* bufferR_ = nullptr;
    int bufferCap_{};
    int bufferSize_{ 2 };
    int position_{};

    double baseSeconds_{};
    double lpA1_{};
    double lpZL_{};
    double lpZR_{};
    double lfoPhaseL_{};
    double lfoPhaseR_{};

    double lastSaturate_{};
    double lastLpf_{};
    double lastHpf_{};
    double lastNum_{};
    double lastDen_{};
    double lastMode_{};
    double lastBpm_{};
    double lastSampleRate_{};

    double clipMemL_[2]{ 0.0, 2.0 };
    double clipMemR_[2]{ 0.0, 2.0 };
    double hpfMemL_[2]{ 20.0, 48000.0 };
    double hpfMemR_[2]{ 20.0, 48000.0 };

    dynamics::atom::SoftClipper clipL_{};
    dynamics::atom::SoftClipper clipR_{};
    filter::atom::OnePoleHp hpfL_{};
    filter::atom::OnePoleHp hpfR_{};
};

} // namespace soemdsp::delay
