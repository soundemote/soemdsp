#pragma once

#include "soemdsp/Wire.hpp"
#include "soemdsp/sehelper.hpp"

#include <cmath>

namespace soemdsp::dynamics {

struct SoftClipper {
    SoftClipper() {
        updateCoeffs();
        oversamplesChanged();
    }

    double run(double v) {
        in_  = v;
        out_ = shiftY_ + scaleY_ * std::tanh(scaleX_ * in_ + shiftX_);
        // for (int i = 0; i < oversamples_; ++i) {
        // out_ = aafilter_.filter(out_);
        //}
        return out_;
    }

    double in_{};
    double out_{};

    // parameters
    void oversamplesChanged() { // oversamples
        // aafilter_.setup(SampleRate::freq_ * oversamples_, SampleRate::halffreq_, 3.0);
    }
    void updateCoeffs() { // center, width
        scaleX_ = 2.0 / width_;
        shiftX_ = -1.0 - (scaleX_ * (center_ - 0.5 * width_));
        scaleY_ = 1.0 / scaleX_;
        shiftY_ = -shiftX_ * scaleY_;
    }
    Wire<double> center_;       // introduce a clipping bias by offsetting this from 0
    Wire<double> width_{ 2.0 }; // > 0 gradually clips signal as it approaches -width and +width
    Wire<int> oversamples_{ 1 };

    // coefficients
    Wire<double> scaleX_{ 1.0 };
    Wire<double> scaleY_{ 1.0 }; // scale factors for input and output
    Wire<double> shiftX_;
    Wire<double> shiftY_; // shift offsets for input and output

    // connections
    void slave(SoftClipper& master) {
        center_.pointTo(master.center_);
        width_.pointTo(master.width_);
        scaleX_.pointTo(master.scaleX_);
        scaleY_.pointTo(master.scaleY_);
        shiftX_.pointTo(master.shiftX_);
        shiftY_.pointTo(master.shiftY_);
        oversamples_.pointTo(master.oversamples_);
    }

    // objects
    // Iir::Butterworth::LowPass<4> aafilter_;
};

} // namespace soemdsp::dynamics
