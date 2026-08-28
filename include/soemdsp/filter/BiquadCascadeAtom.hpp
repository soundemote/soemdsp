#pragma once

// Atom: BiquadCascade (lowpass or peak stages)
// External control memory: mem + base (doubles). Hot path uses cached coeffs only.
// Pattern: accessors → *Changed() → syncControlParams() → reset() / process()

#include <cmath>
#include <cstdint>

namespace soemdsp::filter::atom {

struct BiquadCascade
{
    static constexpr int kMaxStages = 5;
    static constexpr std::uint32_t kControlCount = 6;

    enum class Mode : int
    {
        Lowpass = 0,
        Peak = 1
    };

    // Slot layout:
    //   base+0 = mode (0 lp, 1 peak)
    //   base+1 = frequencyHz
    //   base+2 = q
    //   base+3 = gainDb (peak)
    //   base+4 = stages (0..5)
    //   base+5 = sampleRate
    inline double& mode_() noexcept { return mem[base + 0]; }
    inline double& frequencyHz_() noexcept { return mem[base + 1]; }
    inline double& q_() noexcept { return mem[base + 2]; }
    inline double& gainDb_() noexcept { return mem[base + 3]; }
    inline double& stages_() noexcept { return mem[base + 4]; }
    inline double& sampleRate_() noexcept { return mem[base + 5]; }

    void coeffsChanged() noexcept
    {
        const double sr = sampleRate_() > 1.0 ? sampleRate_() : 44100.0;
        nStages_ = static_cast<int>(stages_() + 0.5);
        if (nStages_ < 0)
        {
            nStages_ = 0;
        }
        if (nStages_ > kMaxStages)
        {
            nStages_ = kMaxStages;
        }
        const int mode = static_cast<int>(mode_() + 0.5);
        for (int i = 0; i < kMaxStages; ++i)
        {
            if (mode == static_cast<int>(Mode::Peak))
            {
                setPeak(biquads_[i], frequencyHz_(), gainDb_(), q_(), sr);
            }
            else
            {
                setLowpass(biquads_[i], frequencyHz_(), q_(), sr);
            }
        }
    }

    void syncControlParams() noexcept
    {
        const double mode = mode_();
        const double frequencyHz = frequencyHz_();
        const double q = q_();
        const double gainDb = gainDb_();
        const double stages = stages_();
        const double sampleRate = sampleRate_();
        if (mode != lastMode_ || frequencyHz != lastFrequencyHz_ || q != lastQ_
            || gainDb != lastGainDb_ || stages != lastStages_ || sampleRate != lastSampleRate_)
        {
            lastMode_ = mode;
            lastFrequencyHz_ = frequencyHz;
            lastQ_ = q;
            lastGainDb_ = gainDb;
            lastStages_ = stages;
            lastSampleRate_ = sampleRate;
            coeffsChanged();
        }
    }

    void reset() noexcept
    {
        for (int i = 0; i < kMaxStages; ++i)
        {
            biquads_[i].z1 = 0.0;
            biquads_[i].z2 = 0.0;
        }
    }

    double process(double x) noexcept
    {
        if (nStages_ <= 0)
        {
            return x;
        }
        double y = x;
        for (int i = 0; i < nStages_; ++i)
        {
            auto& s = biquads_[i];
            const double out = s.b0 * y + s.z1;
            s.z1 = s.b1 * y - s.a1 * out + s.z2;
            s.z2 = s.b2 * y - s.a2 * out;
            y = out;
        }
        return y;
    }

    double* mem = nullptr;
    std::uint32_t base{};

    double lastMode_{};
    double lastFrequencyHz_{ 1000.0 };
    double lastQ_{ 1.0 };
    double lastGainDb_{};
    double lastStages_{ 2.0 };
    double lastSampleRate_{};

    int nStages_{ 2 };

    struct Stage
    {
        double b0{ 1.0 }, b1{}, b2{}, a1{}, a2{};
        double z1{}, z2{};
    };
    Stage biquads_[kMaxStages]{};

  private:
    static void setLowpass(Stage& s, double freq, double q, double sr) noexcept
    {
        const double w0 = 6.283185307179586 * clampFreq(freq, sr) / sr;
        const double cosw = std::cos(w0);
        const double sinw = std::sin(w0);
        const double qq = q > 0.05 ? q : 0.05;
        const double alpha = sinw / (2.0 * qq);
        const double a0 = 1.0 + alpha;
        s.b0 = ((1.0 - cosw) * 0.5) / a0;
        s.b1 = (1.0 - cosw) / a0;
        s.b2 = s.b0;
        s.a1 = (-2.0 * cosw) / a0;
        s.a2 = (1.0 - alpha) / a0;
    }

    static void setPeak(Stage& s, double freq, double gainDb, double q, double sr) noexcept
    {
        const double A = std::exp(gainDb * 0.11512925464970229);
        const double w0 = 6.283185307179586 * clampFreq(freq, sr) / sr;
        const double cosw = std::cos(w0);
        const double sinw = std::sin(w0);
        const double qq = q > 0.05 ? q : 0.05;
        const double alpha = sinw / (2.0 * qq);
        const double a0 = 1.0 + alpha / A;
        s.b0 = (1.0 + alpha * A) / a0;
        s.b1 = (-2.0 * cosw) / a0;
        s.b2 = (1.0 - alpha * A) / a0;
        s.a1 = s.b1;
        s.a2 = (1.0 - alpha / A) / a0;
    }

    static double clampFreq(double freq, double sr) noexcept
    {
        double f = freq;
        if (!(f > 1.0))
        {
            f = 1.0;
        }
        const double maxF = sr * 0.45;
        if (f > maxF)
        {
            f = maxF;
        }
        return f;
    }
};

} // namespace soemdsp::filter::atom
