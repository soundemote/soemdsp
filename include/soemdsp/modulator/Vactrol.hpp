#pragma once

#include <algorithm>
#include <cmath>

#include <soemdsp/SampleRate.hpp>
#include <soemdsp/Wire.hpp>

namespace soemdsp::modulator {

// Vactrol-style optical control shaper, made by VOID.
//
// Input and output are unipolar control values:
//   light in:  0.0 dark, 1.0 full light
//   out:       smoothed detector response
//
// The internal detector uses one-pole attack/release coefficients derived from
// seconds. Keep raw_ separate from out_ so responseCurve_ can shape the visible
// or control output without corrupting the one-pole state.
class Vactrol {
  public:
    Vactrol() {
        sampleRateChanged();
    }

    void sampleRateChanged() {
        timesChanged();
    }

    void timesChanged() {
        attackCoefficient_  = coefficientFromSeconds(attackTime_);
        releaseCoefficient_ = coefficientFromSeconds(releaseTime_);
    }

    void reset(double value = 0.0) {
        raw_ = sanitizeUnipolar(value);
        out_ = shapeResponse(raw_);
    }

    double run(double light) {
        in_     = light;
        target_ = sanitizeUnipolar(light * sensitivity_ + lightOffset_);

        const bool rising = target_ > raw_;
        const double coefficient = rising ? attackCoefficient_ : releaseCoefficient_;
        raw_ += (target_ - raw_) * coefficient;

        out_ = shapeResponse(raw_);
        return out_;
    }

    static double coefficientFromSeconds(double seconds) {
        if (!std::isfinite(seconds) || seconds <= 0.0) {
            return 1.0;
        }

        const double samples = std::max(1.0, SampleRate::timeToSamples(seconds));
        return 1.0 - std::exp(-1.0 / samples);
    }

    static double sanitizeUnipolar(double value) {
        if (!std::isfinite(value)) {
            return 0.0;
        }

        return std::clamp(value, 0.0, 1.0);
    }

    double shapeResponse(double value) const {
        const double curve = std::max(0.001, static_cast<double>(responseCurve_));
        const double shaped = std::pow(sanitizeUnipolar(value), curve);
        const double dark = sanitizeUnipolar(currentOffset_);
        return sanitizeUnipolar(dark + shaped * (1.0 - dark));
    }

    double in_{};
    double target_{};
    double raw_{};
    double out_{};

    // Parameters.
    Wire attackTime_{ 0.010 };   // seconds
    Wire releaseTime_{ 0.450 };  // seconds
    Wire responseCurve_{ 1.0 };  // < 1 lifts the tail, > 1 darkens low values
    Wire sensitivity_{ 1.0 };    // light input multiplier before clipping
    Wire lightOffset_{ 0.0 };    // light offset before clipping
    Wire currentOffset_{ 0.0 };  // minimum output when detector is dark

    // Coefficients.
    double attackCoefficient_{};
    double releaseCoefficient_{};
};

} // namespace soemdsp::modulator
