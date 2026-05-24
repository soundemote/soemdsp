#pragma once

#include <soemdsp/Wire.hpp>

/* returns a sinusoidal sample between - 1 and + 1 given a value of 0 to 1 */

namespace soemdsp::modulator {
struct Parabol {

    static double sample(double x) {
        double fit = fmod(2.0 * x, 2.0) - 1.0;
        return 4.0 * fit * (1.0 - fabs(fit));
    }

    void reset() {
        phase_ = 0.0;
    }
    double run() {
        out_   = sample(phase_);
        phase_ = math::wrap(*phase_.r + *increment_.r);

        return out_;
    }
    double out_{};

    // parameters
    Wire increment_;
    Wire phase_;

    // coefficients
};
} // namespace soemdsp::modulator
