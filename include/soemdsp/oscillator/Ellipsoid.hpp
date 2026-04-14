#pragma once

#include "soemdsp/SampleRate.hpp"
#include "soemdsp/Wire.hpp"
#include "soemdsp/semath.hpp"

namespace soemdsp::oscillator {

class Ellipsoid {
  public:
    // A = offset -1 to +1, B = shape sinCos(-1*kPI to +1*kPI), C = scale 0 to 1
    static double getEllipsoid(double phase, double A, double B_sin, double B_cos, double C) {
        auto [sinPhase, cosPhase] = sinCos(phase * kTAU);

        double sqrt_val = sqrt(pow(A + cosPhase, 2) + pow(C * sinPhase, 2));
        double out_     = ((A + cosPhase) * B_cos + (C * sinPhase) * B_sin) / sqrt_val;

        return out_;
    }

    // shape is 0 to 1, 0 being sin, 1 being square
    static double getSineToSquare(double phase, double shape) {
        auto [sinPhase, cosPhase] = sinCos(kTAU * phase);

        double x = pow(cosPhase, 2) + pow(sinPhase * shape, 2);

        return (cosPhase / sqrt(x));
    }

    // shape is -1 > 0 < 1 ramp / saw / sine
    static double getSineToSaw(double phase, double shape) {
        return getEllipsoid(phase - 0.75, -shape, ksin_PIx1p5, kcos_PIx1p5, 1);
    }

    // io
    void run() {
        phase_ += increment_;
        phase_ = wrap(phase_);
    }
    double out_{};

    // parameters
    void shapeChanged() {
        std::tie(sin_, cos_) = sinCos(shape_ * kPI);
    }
    Wire<double> increment_;
    Wire<double> phase_;
    Wire<double> phaseOffset_;
    Wire<double> offset_;       // A -1 to +1
    Wire<double> shape_;        // shapeChanged() B -1 to +1
    Wire<double> scale_{ 1.0 }; // C 0 to 10+

    double getSineToSquare() {
        return out_ = getSineToSquare(phase_, shape_);
    }

    double getSineToSaw() {
        return out_ = getSineToSaw(phase_, shape_);
    }

    double getEllipsoid() {
        return out_ = getEllipsoid(phase_, offset_, sin_, cos_, scale_);
    }

  protected:
    double sin_{};
    double cos_{ 1.0 };
};
} // namespace soemdsp::oscillator
