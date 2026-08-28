#pragma once

// Atom meta + simple process: SampleDelay
// See docs/DSP_ATOM_PARADIGM.md.

#include <cmath>
#include <cstdint>

#include <soemdsp/dsp/AtomParam.hpp>

namespace soemdsp::delay {

struct SampleDelay
{
    static constexpr int kMaxBufferSamples = 768000; // 4 s @ 192 kHz
    static constexpr double kMaxDelaySeconds = 4.0;

    static constexpr soemdsp::dsp::AtomParam kParams[] = {
      { 0, "timeSeconds", "Time", 0.0, 0.0, 4.0, "s", soemdsp::dsp::ParamKind::Live, "" },
      { 1, "samples", "Samples", 0.0, 0.0, 768000.0, "", soemdsp::dsp::ParamKind::Live, "" },
      { 2, "sampleRate", "Sample Rate", 48000.0, 8000.0, 192000.0, "Hz", soemdsp::dsp::ParamKind::Live, "" },
    };
    static constexpr std::uint32_t kControlCount = 3;

    inline double& timeSeconds_() noexcept { return mem[base + 0]; }
    inline double& samples_() noexcept { return mem[base + 1]; }
    inline double& sampleRate_() noexcept { return mem[base + 2]; }

    void setBuffer(float* buffer, int capacity) noexcept
    {
        buffer_ = buffer;
        bufferCap_ = capacity > 1 ? capacity : 1;
    }

    void initDefaults() noexcept
    {
        for (const auto& p : kParams)
        {
            mem[base + p.slot] = p.def;
        }
        reset();
    }

    void reset() noexcept
    {
        writeIndex_ = 0;
        if (buffer_)
        {
            for (int i = 0; i < bufferCap_; ++i)
            {
                buffer_[i] = 0.0f;
            }
        }
    }

    // All timing params are LIVE (modulatable delay).
    double process(double input) noexcept
    {
        if (!buffer_ || bufferCap_ < 2)
        {
            return input;
        }
        const double sr = sampleRate_() > 1.0 ? sampleRate_() : 44100.0;
        double delaySamples = timeSeconds_() * sr + samples_();
        if (!(delaySamples >= 0.0))
        {
            delaySamples = 0.0;
        }
        if (delaySamples > static_cast<double>(bufferCap_ - 1))
        {
            delaySamples = static_cast<double>(bufferCap_ - 1);
        }

        const int write = writeIndex_;
        buffer_[write] = static_cast<float>(input);
        const double readPos = static_cast<double>(write) - delaySamples;
        int i0 = static_cast<int>(std::floor(readPos));
        const double frac = readPos - std::floor(readPos);
        i0 %= bufferCap_;
        if (i0 < 0)
        {
            i0 += bufferCap_;
        }
        int i1 = i0 + 1;
        if (i1 >= bufferCap_)
        {
            i1 = 0;
        }
        const double a = (i0 == write) ? input : static_cast<double>(buffer_[i0]);
        const double b = (i1 == write) ? input : static_cast<double>(buffer_[i1]);
        writeIndex_ = write + 1;
        if (writeIndex_ >= bufferCap_)
        {
            writeIndex_ = 0;
        }
        return a + (b - a) * frac;
    }

    double* mem = nullptr;
    std::uint32_t base{};

  private:
    float* buffer_ = nullptr;
    int bufferCap_{};
    int writeIndex_{};
};

} // namespace soemdsp::delay
