#pragma once

// Atom: SoftClipper — same paradigm as Reverb (see docs/DSP_ATOM_PARADIGM.md).

#include <cmath>
#include <cstdint>

#include <soemdsp/dsp/AtomParam.hpp>

namespace soemdsp::dynamics::atom {

struct SoftClipper
{
    static constexpr soemdsp::dsp::AtomParam kParams[] = {
      { 0, "center", "Center", 0.0, -1.0, 1.0, "", soemdsp::dsp::ParamKind::Control, "centerChanged" },
      { 1, "width", "Width", 2.0, 0.01, 4.0, "", soemdsp::dsp::ParamKind::Control, "widthChanged" },
    };
    static constexpr std::uint32_t kControlCount = 2;

    inline double& center_() noexcept { return mem[base + 0]; }
    inline double& width_() noexcept { return mem[base + 1]; }

    void centerChanged() noexcept
    {
        updateCoeffs();
    }

    void widthChanged() noexcept
    {
        updateCoeffs();
    }

    void updateCoeffs() noexcept
    {
        double w = width_();
        if (!(w > 0.0))
        {
            w = 1.0e-6;
        }
        scaleX_ = 2.0 / w;
        shiftX_ = -1.0 - (scaleX_ * (center_() - 0.5 * w));
        scaleY_ = 1.0 / scaleX_;
        shiftY_ = -shiftX_ * scaleY_;
    }

    // Host-only-writes-mem path: recompute coeffs when slots change.
    void syncControlParams() noexcept
    {
        const double c = center_();
        const double w = width_();
        if (c != lastCenter_ || w != lastWidth_)
        {
            lastCenter_ = c;
            lastWidth_ = w;
            updateCoeffs();
        }
    }

    double process(double v) noexcept
    {
        in_ = v;
        out_ = shiftY_ + scaleY_ * std::tanh(scaleX_ * in_ + shiftX_);
        return out_;
    }

    double* mem = nullptr;
    std::uint32_t base{};

    double lastCenter_{};
    double lastWidth_{ 2.0 };

    double scaleX_{ 1.0 };
    double scaleY_{ 1.0 };
    double shiftX_{};
    double shiftY_{};

    double in_{};
    double out_{};
};

} // namespace soemdsp::dynamics::atom
