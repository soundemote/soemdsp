#pragma once

// Atom: Reverb (reference composite module)
//
// Copy this shape for other modules:
//   1. kParams[]     — meta in this file (keys, ranges, Live vs Control)
//   2. mem[base+i]   — host/control storage
//   3. *Changed()    — only Control slots that rebuild derived state
//   4. process()     — pass Live values into children as args (no connect())
//
// Live (no *Changed): mix, volume, recycle, diffusionAmount, lfoAmp, echoMode, pingPong
// Control (*Changed): echoTime, numDelays, diffusionSize, seed, lfoFrequency,
//                     lfoVariation, doModulateEcho, saturate, sampleRate, filters

#include <cstdint>

#include <soemdsp/delay/ModulatedDelay.hpp>
#include <soemdsp/dsp/AtomParam.hpp>
#include <soemdsp/dynamics/SoftClipperAtom.hpp>
#include <soemdsp/filter/BiquadCascadeAtom.hpp>
#include <soemdsp/filter/OnePoleHpAtom.hpp>
#include <soemdsp/random/NoiseAtoms.hpp>

namespace soemdsp::delay {

struct Reverb
{
    static constexpr int kMaxDelays = 16;
    static constexpr int kMaxBufferSamples = 48000;
    static constexpr int kLineCount = kMaxDelays * 2 + 2;

    static constexpr soemdsp::dsp::AtomParam kParams[] = {
      { 0, "mix", "Mix", 0.43, 0.0, 1.0, "", soemdsp::dsp::ParamKind::Live, "" },
      { 1, "volume", "Volume", 1.0, 0.0, 4.0, "", soemdsp::dsp::ParamKind::Live, "" },
      { 2, "echoTime", "Echo Time", 0.35, 0.0001, 4.0, "s", soemdsp::dsp::ParamKind::Control, "echoTimeChanged" },
      { 3, "recycle", "Recycle", 0.5, 0.0, 2.0, "", soemdsp::dsp::ParamKind::Live, "" },
      { 4, "numDelays", "Num Delays", 10.0, 0.0, 16.0, "", soemdsp::dsp::ParamKind::Control, "numDelaysChanged" },
      { 5, "diffusionSize", "Diffuse Size", 0.35, 0.0001, 2.0, "", soemdsp::dsp::ParamKind::Control, "diffusionSizeChanged" },
      { 6, "diffusionAmount", "Diffuse Amt", 0.70, 0.0, 0.98, "", soemdsp::dsp::ParamKind::Live, "" },
      { 7, "seed", "Seed", 500.0, 0.0, 999.0, "", soemdsp::dsp::ParamKind::Control, "diffusionSeedChanged" },
      { 8, "lfoAmp", "LFO Amp", 0.002, 0.0, 0.5, "", soemdsp::dsp::ParamKind::Live, "" },
      { 9, "lfoFrequency", "LFO Speed", 0.5, 0.1, 90.0, "Hz", soemdsp::dsp::ParamKind::Control, "lfoFrequencyChanged" },
      { 10, "lfoVariation", "LFO Vary", 1.0, 0.0, 10.0, "", soemdsp::dsp::ParamKind::Control, "lfoVariationChanged" },
      { 11, "echoMode", "Delay Mode", 0.0, 0.0, 2.0, "", soemdsp::dsp::ParamKind::Live, "" },
      { 12, "doModulateEcho", "Mod Echo", 1.0, 0.0, 1.0, "", soemdsp::dsp::ParamKind::Control, "doModulateEchoChanged" },
      { 13, "pingPong", "Ping Pong", 0.0, 0.0, 1.0, "", soemdsp::dsp::ParamKind::Live, "" },
      { 14, "saturate", "Saturate", 1.0, 0.01, 4.0, "", soemdsp::dsp::ParamKind::Control, "clippingThresholdChanged" },
      { 15, "sampleRate", "Sample Rate", 48000.0, 8000.0, 192000.0, "Hz", soemdsp::dsp::ParamKind::Control, "sampleRateChanged" },
      { 16, "hpfFrequency", "HPF Freq", 20.0, 1.0, 2000.0, "Hz", soemdsp::dsp::ParamKind::Control, "hpfFrequencyChanged" },
      { 17, "lpfFrequency", "LPF Freq", 8000.0, 20.0, 20000.0, "Hz", soemdsp::dsp::ParamKind::Control, "lpfFrequencyChanged" },
    };
    static constexpr std::uint32_t kControlCount =
      sizeof(kParams) / sizeof(kParams[0]);

    enum class EchoMode : int
    {
        PostDelay = 0,
        PreDelay = 1,
        Slapback = 2
    };

    inline double& mix_() noexcept { return mem[base + 0]; }
    inline double& volume_() noexcept { return mem[base + 1]; }
    inline double& echoTimeSeconds_() noexcept { return mem[base + 2]; }
    inline double& recycle_() noexcept { return mem[base + 3]; }
    inline double& numDelays_() noexcept { return mem[base + 4]; }
    inline double& diffusionSize_() noexcept { return mem[base + 5]; }
    inline double& diffusionAmount_() noexcept { return mem[base + 6]; }
    inline double& seed_() noexcept { return mem[base + 7]; }
    inline double& lfoAmp_() noexcept { return mem[base + 8]; }
    inline double& lfoFrequencyHz_() noexcept { return mem[base + 9]; }
    inline double& lfoVariation_() noexcept { return mem[base + 10]; }
    inline double& echoMode_() noexcept { return mem[base + 11]; }
    inline double& doModulateEcho_() noexcept { return mem[base + 12]; }
    inline double& pingPong_() noexcept { return mem[base + 13]; }
    inline double& saturate_() noexcept { return mem[base + 14]; }
    inline double& sampleRate_() noexcept { return mem[base + 15]; }
    inline double& hpfFrequencyHz_() noexcept { return mem[base + 16]; }
    inline double& lpfFrequencyHz_() noexcept { return mem[base + 17]; }

    void bindChildren() noexcept
    {
        for (int i = 0; i < kMaxDelays; ++i)
        {
            delaysL_[i].mem = lineMemL_[i];
            delaysL_[i].base = 0;
            delaysR_[i].mem = lineMemR_[i];
            delaysR_[i].base = 0;
            delaysL_[i].setBuffer(delayPool_[i], kMaxBufferSamples);
            delaysR_[i].setBuffer(delayPool_[kMaxDelays + i], kMaxBufferSamples);
        }
        echoL_.mem = echoMemL_;
        echoR_.mem = echoMemR_;
        echoL_.base = 0;
        echoR_.base = 0;
        echoL_.setBuffer(delayPool_[kMaxDelays * 2], kMaxBufferSamples);
        echoR_.setBuffer(delayPool_[kMaxDelays * 2 + 1], kMaxBufferSamples);

        clipL_.mem = clipMemL_;
        clipR_.mem = clipMemR_;
        clipL_.base = clipR_.base = 0;
        hpfL_.mem = hpfMemL_;
        hpfR_.mem = hpfMemR_;
        hpfL_.base = hpfR_.base = 0;
        lpfL_.mem = lpfMemL_;
        lpfR_.mem = lpfMemR_;
        lpfL_.base = lpfR_.base = 0;
        rng_.mem = rngMem_;
        rng_.base = 0;
    }

    void initDefaults() noexcept
    {
        for (const auto& p : kParams)
        {
            mem[base + p.slot] = p.def;
        }
        bindChildren();
        activeDelays_ = -1;
        numDelaysChanged();
        sampleRateChanged();
        reset();
        diffusionSeedChanged();
        lfoFrequencyChanged();
        doModulateEchoChanged();
        clippingThresholdChanged();
        markAllSynced();
    }

    void sampleRateChanged() noexcept
    {
        const double sr = sampleRate_() > 1.0 ? sampleRate_() : 44100.0;
        for (int i = 0; i < kMaxDelays; ++i)
        {
            delaysL_[i].sampleRate_() = sr;
            delaysR_[i].sampleRate_() = sr;
            delaysL_[i].sampleRateChanged();
            delaysR_[i].sampleRateChanged();
        }
        echoL_.sampleRate_() = sr;
        echoR_.sampleRate_() = sr;
        echoL_.sampleRateChanged();
        echoR_.sampleRateChanged();

        hpfMemL_[0] = hpfFrequencyHz_();
        hpfMemR_[0] = hpfFrequencyHz_();
        hpfMemL_[1] = sr;
        hpfMemR_[1] = sr;
        hpfL_.sampleRateChanged();
        hpfR_.sampleRateChanged();

        lpfMemL_[0] = 0.0;
        lpfMemR_[0] = 0.0;
        lpfMemL_[1] = lpfFrequencyHz_();
        lpfMemR_[1] = lpfFrequencyHz_();
        lpfMemL_[2] = 0.707;
        lpfMemR_[2] = 0.707;
        lpfMemL_[3] = 0.0;
        lpfMemR_[3] = 0.0;
        lpfMemL_[4] = 2.0;
        lpfMemR_[4] = 2.0;
        lpfMemL_[5] = sr;
        lpfMemR_[5] = sr;
        lpfL_.coeffsChanged();
        lpfR_.coeffsChanged();

        echoTimeChanged();
        lfoFrequencyChanged();
    }

    void numDelaysChanged() noexcept
    {
        int n = static_cast<int>(numDelays_() + 0.5);
        if (n < 0)
        {
            n = 0;
        }
        if (n > kMaxDelays)
        {
            n = kMaxDelays;
        }
        if (n == activeDelays_)
        {
            return;
        }
        activeDelays_ = n;
        doModulateEchoChanged();
    }

    void echoTimeChanged() noexcept
    {
        echoL_.delayTimeSeconds_() = echoTimeSeconds_();
        echoR_.delayTimeSeconds_() = echoTimeSeconds_();
        echoL_.echoTimeChanged();
        echoR_.echoTimeChanged();
    }

    void diffusionSizeChanged() noexcept
    {
        for (int i = 0; i < activeDelays_; ++i)
        {
            delaysL_[i].delayTimeSeconds_() = diffusionSize_();
            delaysR_[i].delayTimeSeconds_() = diffusionSize_();
            delaysL_[i].delayTimeChanged();
            delaysR_[i].delayTimeChanged();
        }
    }

    void diffusionSeedChanged() noexcept
    {
        const auto seed = static_cast<std::uint64_t>(seed_());
        rng_.seed_() = seed == 0 ? 1ULL : seed;
        rng_.syncSeed();
        for (int i = 0; i < activeDelays_; ++i)
        {
            delaysL_[i].delayTimeSeconds_() = diffusionSize_();
            delaysR_[i].delayTimeSeconds_() = diffusionSize_();
            const double u = rng_.nextUnipolar();
            delaysL_[i].diffusionSeedChanged(u);
            delaysR_[i].diffusionSeedChanged(u);
        }
    }

    void lfoFrequencyChanged() noexcept
    {
        for (int i = 0; i < activeDelays_; ++i)
        {
            delaysL_[i].lfoFrequencyHz_() = lfoFrequencyHz_();
            delaysR_[i].lfoFrequencyHz_() = lfoFrequencyHz_();
            delaysL_[i].lfoVariation_() = lfoVariation_();
            delaysR_[i].lfoVariation_() = lfoVariation_();
            delaysL_[i].lfoChanged();
            delaysR_[i].lfoChanged();
        }
        echoL_.lfoFrequencyHz_() = lfoFrequencyHz_();
        echoR_.lfoFrequencyHz_() = lfoFrequencyHz_();
        echoL_.lfoVariation_() = lfoVariation_();
        echoR_.lfoVariation_() = lfoVariation_();
        echoL_.lfoChanged();
        echoR_.lfoChanged();
    }

    void lfoVariationChanged() noexcept
    {
        lfoFrequencyChanged();
    }

    void doModulateEchoChanged() noexcept
    {
        // Topology only: whether echo uses live lfoAmp (handled in process).
        // Cached flag avoids branching on the raw slot every line.
        echoUsesLfoAmp_ = doModulateEcho_() > 0.5;
    }

    void clippingThresholdChanged() noexcept
    {
        const double sat = saturate_() > 1.0e-6 ? saturate_() : 1.0e-6;
        clipMemL_[0] = 0.0;
        clipMemL_[1] = sat * 2.0;
        clipMemR_[0] = 0.0;
        clipMemR_[1] = sat * 2.0;
        clipL_.updateCoeffs();
        clipR_.updateCoeffs();
        feedbackCompensation_ = sat >= 1.0 ? 1.0 : sat;
    }

    void hpfFrequencyChanged() noexcept
    {
        hpfMemL_[0] = hpfFrequencyHz_();
        hpfMemR_[0] = hpfFrequencyHz_();
        hpfL_.frequencyChanged();
        hpfR_.frequencyChanged();
    }

    void lpfFrequencyChanged() noexcept
    {
        lpfMemL_[1] = lpfFrequencyHz_();
        lpfMemR_[1] = lpfFrequencyHz_();
        lpfL_.coeffsChanged();
        lpfR_.coeffsChanged();
    }

    void syncControlParams() noexcept
    {
        const double echoTime = echoTimeSeconds_();
        const double numDelays = numDelays_();
        const double diffusionSize = diffusionSize_();
        const double seed = seed_();
        const double lfoFrequencyHz = lfoFrequencyHz_();
        const double lfoVariation = lfoVariation_();
        const double doModulateEcho = doModulateEcho_();
        const double saturate = saturate_();
        const double sampleRate = sampleRate_();
        const double hpfFrequencyHz = hpfFrequencyHz_();
        const double lpfFrequencyHz = lpfFrequencyHz_();

        if (sampleRate != lastSampleRate_)
        {
            lastSampleRate_ = sampleRate;
            sampleRateChanged();
            markAllSynced();
            return;
        }
        if (seed != lastSeed_)
        {
            lastSeed_ = seed;
            diffusionSeedChanged();
        }
        if (numDelays != lastNumDelays_)
        {
            lastNumDelays_ = numDelays;
            numDelaysChanged();
        }
        if (diffusionSize != lastDiffusionSize_)
        {
            lastDiffusionSize_ = diffusionSize;
            diffusionSizeChanged();
        }
        if (echoTime != lastEchoTime_)
        {
            lastEchoTime_ = echoTime;
            echoTimeChanged();
        }
        if (lfoFrequencyHz != lastLfoFrequencyHz_)
        {
            lastLfoFrequencyHz_ = lfoFrequencyHz;
            lfoFrequencyChanged();
        }
        if (lfoVariation != lastLfoVariation_)
        {
            lastLfoVariation_ = lfoVariation;
            lfoVariationChanged();
        }
        if (doModulateEcho != lastDoModulateEcho_)
        {
            lastDoModulateEcho_ = doModulateEcho;
            doModulateEchoChanged();
        }
        if (saturate != lastSaturate_)
        {
            lastSaturate_ = saturate;
            clippingThresholdChanged();
        }
        if (hpfFrequencyHz != lastHpfFrequencyHz_)
        {
            lastHpfFrequencyHz_ = hpfFrequencyHz;
            hpfFrequencyChanged();
        }
        if (lpfFrequencyHz != lastLpfFrequencyHz_)
        {
            lastLpfFrequencyHz_ = lpfFrequencyHz;
            lpfFrequencyChanged();
        }
    }

    void reset() noexcept
    {
        fbL_ = fbR_ = 0.0;
        dryL_ = dryR_ = wetL_ = wetR_ = outL_ = outR_ = 0.0;
        for (int i = 0; i < kMaxDelays; ++i)
        {
            delaysL_[i].reset();
            delaysR_[i].reset();
        }
        echoL_.reset();
        echoR_.reset();
        hpfL_.reset();
        hpfR_.reset();
        lpfL_.reset();
        lpfR_.reset();
    }

    void process(double inL, double inR) noexcept
    {
        inL_ = inL;
        inR_ = inR;

        // LIVE values read once per sample (replaces Wire pointTo / connect()).
        const double feedback = diffusionAmount_();
        const double lfoAmp = lfoAmp_();
        const double echoLfoAmp = echoUsesLfoAmp_ ? lfoAmp : 0.0;

        const int mode = static_cast<int>(echoMode_() + 0.5);
        switch (mode)
        {
        case static_cast<int>(EchoMode::PreDelay):
            processPreDelay(feedback, lfoAmp, echoLfoAmp);
            break;
        case static_cast<int>(EchoMode::Slapback):
            processSlapback(feedback, lfoAmp, echoLfoAmp);
            break;
        case static_cast<int>(EchoMode::PostDelay):
        default:
            processPostDelay(feedback, lfoAmp, echoLfoAmp);
            break;
        }

        fbL_ *= recycle_();
        fbR_ *= recycle_();
    }

    double* mem = nullptr;
    std::uint32_t base{};

    double inL_{}, inR_{};
    double fbL_{}, fbR_{};
    double dryL_{}, dryR_{};
    double wetL_{}, wetR_{};
    double outL_{}, outR_{};

  private:
    void markAllSynced() noexcept
    {
        lastEchoTime_ = echoTimeSeconds_();
        lastNumDelays_ = numDelays_();
        lastDiffusionSize_ = diffusionSize_();
        lastSeed_ = seed_();
        lastLfoFrequencyHz_ = lfoFrequencyHz_();
        lastLfoVariation_ = lfoVariation_();
        lastDoModulateEcho_ = doModulateEcho_();
        lastSaturate_ = saturate_();
        lastSampleRate_ = sampleRate_();
        lastHpfFrequencyHz_ = hpfFrequencyHz_();
        lastLpfFrequencyHz_ = lpfFrequencyHz_();
        echoUsesLfoAmp_ = doModulateEcho_() > 0.5;
    }

    void feedbackFilter(bool reverseStereo) noexcept
    {
        fbL_ = hpfL_.process(fbL_);
        fbR_ = hpfR_.process(fbR_);
        if (reverseStereo)
        {
            const double ol = lpfL_.process(fbL_);
            const double orr = lpfR_.process(fbR_);
            fbR_ = ol;
            fbL_ = orr;
        }
        else
        {
            fbL_ = lpfL_.process(fbL_);
            fbR_ = lpfR_.process(fbR_);
        }
    }

    void dryWet() noexcept
    {
        const double dryGain = (1.0 - mix_()) * volume_();
        const double wetGain = mix_() * volume_();
        dryL_ = inL_ * dryGain;
        dryR_ = inR_ * dryGain;
        wetL_ = fbL_ * wetGain;
        wetR_ = fbR_ * wetGain;
        outL_ = dryL_ + wetL_;
        outR_ = dryR_ + wetR_;
    }

    void runEchoPair(double inToL, double inToR, double echoLfoAmp, double& outL, double& outR) noexcept
    {
        const double dL = echoL_.processDelay(inToL, echoLfoAmp);
        const double dR = echoR_.processDelay(inToR, echoLfoAmp);
        if (pingPong_() > 0.5)
        {
            outL = dR;
            outR = dL;
        }
        else
        {
            outL = dL;
            outR = dR;
        }
    }

    void processPostDelay(double feedback, double lfoAmp, double echoLfoAmp) noexcept
    {
        double eL = 0.0;
        double eR = 0.0;
        runEchoPair(fbL_, fbR_, echoLfoAmp, eL, eR);
        fbL_ = inL_ + eL;
        fbR_ = inR_ + eR;
        for (int i = 0; i < activeDelays_; ++i)
        {
            fbL_ = delaysL_[i].processDiffuse(fbL_, feedback, lfoAmp);
            fbR_ = delaysR_[i].processDiffuse(fbR_, feedback, lfoAmp);
        }
        dryWet();
        feedbackFilter(false);
        fbL_ = clipL_.process(fbL_) * feedbackCompensation_;
        fbR_ = clipR_.process(fbR_) * feedbackCompensation_;
    }

    void processPreDelay(double feedback, double lfoAmp, double echoLfoAmp) noexcept
    {
        feedbackFilter(false);
        fbL_ = clipL_.process(inL_ + fbL_) * feedbackCompensation_;
        fbR_ = clipR_.process(inR_ + fbR_) * feedbackCompensation_;
        for (int i = 0; i < activeDelays_; ++i)
        {
            fbL_ = delaysL_[i].processDiffuse(fbL_, feedback, lfoAmp);
            fbR_ = delaysR_[i].processDiffuse(fbR_, feedback, lfoAmp);
        }
        double eL = 0.0;
        double eR = 0.0;
        runEchoPair(fbL_, fbR_, echoLfoAmp, eL, eR);
        fbL_ = eL;
        fbR_ = eR;
        dryWet();
    }

    void processSlapback(double feedback, double lfoAmp, double echoLfoAmp) noexcept
    {
        double eL = 0.0;
        double eR = 0.0;
        runEchoPair(fbL_, fbR_, echoLfoAmp, eL, eR);
        fbL_ = eL;
        fbR_ = eR;
        const double scale = activeDelays_ > 0 ? 1.0 / static_cast<double>(activeDelays_) : 1.0;
        const double L = inL_ * scale;
        const double R = inR_ * scale;
        for (int i = 0; i < activeDelays_; ++i)
        {
            fbL_ = delaysL_[i].processDiffuse(L + fbL_, feedback, lfoAmp);
            fbR_ = delaysR_[i].processDiffuse(R + fbR_, feedback, lfoAmp);
        }
        dryWet();
        feedbackFilter(true);
        fbL_ = clipL_.process(fbL_) * feedbackCompensation_;
        fbR_ = clipR_.process(fbR_) * feedbackCompensation_;
    }

    int activeDelays_{ 10 };
    bool echoUsesLfoAmp_{ true };
    double feedbackCompensation_{ 1.0 };

    double lastEchoTime_{};
    double lastNumDelays_{};
    double lastDiffusionSize_{};
    double lastSeed_{};
    double lastLfoFrequencyHz_{};
    double lastLfoVariation_{};
    double lastDoModulateEcho_{};
    double lastSaturate_{};
    double lastSampleRate_{};
    double lastHpfFrequencyHz_{};
    double lastLpfFrequencyHz_{};

    double lineMemL_[kMaxDelays][ModulatedDelay::kControlCount]{};
    double lineMemR_[kMaxDelays][ModulatedDelay::kControlCount]{};
    double echoMemL_[ModulatedDelay::kControlCount]{};
    double echoMemR_[ModulatedDelay::kControlCount]{};
    double clipMemL_[2]{ 0.0, 2.0 };
    double clipMemR_[2]{ 0.0, 2.0 };
    double hpfMemL_[2]{ 20.0, 48000.0 };
    double hpfMemR_[2]{ 20.0, 48000.0 };
    double lpfMemL_[6]{};
    double lpfMemR_[6]{};
    std::uint64_t rngMem_[2]{};
    float delayPool_[kLineCount][kMaxBufferSamples]{};

    ModulatedDelay delaysL_[kMaxDelays]{};
    ModulatedDelay delaysR_[kMaxDelays]{};
    ModulatedDelay echoL_{};
    ModulatedDelay echoR_{};
    dynamics::atom::SoftClipper clipL_{};
    dynamics::atom::SoftClipper clipR_{};
    filter::atom::OnePoleHp hpfL_{};
    filter::atom::OnePoleHp hpfR_{};
    filter::atom::BiquadCascade lpfL_{};
    filter::atom::BiquadCascade lpfR_{};
    random::atom::SplitMix64 rng_{};
};

} // namespace soemdsp::delay
