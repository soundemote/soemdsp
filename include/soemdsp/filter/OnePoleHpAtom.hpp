#pragma once

// Atom: OnePoleHp
// External control memory: mem + base (doubles). Hot path uses cached coeffs only.
// Pattern: accessors → *Changed() → syncControlParams() → reset() / process()

#include <cmath>
#include <cstdint>

namespace soemdsp::filter::atom {

struct OnePoleHp
{
    static constexpr std::uint32_t kControlCount = 2;

    // Slot layout:
    //   base+0 = frequencyHz
    //   base+1 = sampleRate
    inline double& frequencyHz_() noexcept { return mem[base + 0]; }
    inline double& sampleRate_() noexcept { return mem[base + 1]; }

    void sampleRateChanged() noexcept
    {
        frequencyChanged();
    }

    void frequencyChanged() noexcept
    {
        const double sr = sampleRate_() > 1.0 ? sampleRate_() : 44100.0;
        const double tauZ = 6.283185307179586 / sr;
        double w = tauZ;
        if (w > 0.000142475857)
        {
            w = 0.000142475857;
        }
        w *= frequencyHz_();
        a1_ = std::exp(-w);
        b0_ = 0.5 * (1.0 + a1_);
        b1_ = -b0_;
    }

    void syncControlParams() noexcept
    {
        const double frequencyHz = frequencyHz_();
        const double sampleRate = sampleRate_();
        if (frequencyHz != lastFrequencyHz_ || sampleRate != lastSampleRate_)
        {
            lastFrequencyHz_ = frequencyHz;
            lastSampleRate_ = sampleRate;
            frequencyChanged();
        }
    }

    void reset() noexcept
    {
        buf0_ = 0.0;
        buf1_ = 0.0;
    }

    double process(double input) noexcept
    {
        buf1_ = b0_ * input + b1_ * buf0_ + a1_ * buf1_;
        buf0_ = input;
        return buf1_;
    }

    double* mem = nullptr;
    std::uint32_t base{};

    double lastFrequencyHz_{ 20.0 };
    double lastSampleRate_{};

    double b0_{ 1.0 };
    double b1_{};
    double a1_{};
    double buf0_{};
    double buf1_{};
};

} // namespace soemdsp::filter::atom
