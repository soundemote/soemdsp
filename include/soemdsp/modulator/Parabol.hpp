#pragma once

#include "soemdsp/Wire.hpp"

namespace soemdsp::modulator {
struct Parabol {
    // returns a sinusoidal sample between -1 and +1 given a value of 0 to 1
    static double sample(double x) {
        double fit = fmod(2.0 * x, 2.0) - 1.0;
        return 4.0 * fit * (1.0 - fabs(fit));
    }

    SE_DEFAULT_CONSTRUCTOR(Parabol);

    // io
    void reset() {
        phase_ = 0.0;
    }
    double run() {
        out_   = sample(phase_);
        phase_ = wrap(phase_ + increment_);

        return out_;
    }
    double out_{};

    // parameters
    Wire<double> increment_;
    WireMod<double> phase_;

    // coefficients
};
} // namespace soemdsp::modulator
