#pragma once

#include <algorithm>
#include <cmath>

#include <soemdsp/SampleRate.hpp>
#include <soemdsp/Wire.hpp>

namespace soemdsp::dynamics {

// FlowerChild-style signed envelope follower, made runnable by VOID.
//
// This ports the FlowerChild envelope idea as a proper envelope follower:
// rectified input followed by attack, hold, and decay slew behavior.
class FlowerChildEnvelopeFollower {
  public:
    FlowerChildEnvelopeFollower() {
        sampleRateChanged();
        reset();
    }

    void sampleRateChanged() {
        attackTimeChanged();
        holdTimeChanged();
        decayTimeChanged();
    }

    void attackTimeChanged() {
        attackSamples_ = secondsToSamples(attackTime_);
        attackStep_    = 1.0 / attackSamples_;
    }

    void holdTimeChanged() {
        holdSamples_ = secondsToSamples(holdTime_);
    }

    void decayTimeChanged() {
        decaySamples_ = secondsToSamples(decayTime_);
        decayStep_    = 1.0 / decaySamples_;
    }

    void reset(double value = 0.0) {
        currentSlewedValue_ = sanitize(std::abs(value));
        holdCounter_        = 0.0;
    }

    double run(double input) {
        const double target = sanitize(std::abs(input));

        if (target >= currentSlewedValue_) {
            currentSlewedValue_ = slewToward(currentSlewedValue_, target, attackStep_);
            holdCounter_        = holdSamples_;
        } else if (holdCounter_ > 0.0) {
            holdCounter_ -= 1.0;
        } else {
            currentSlewedValue_ = slewToward(currentSlewedValue_, target, decayStep_);
        }

        out_ = currentSlewedValue_;
        return out_;
    }

    bool isIdle() const {
        return std::abs(currentSlewedValue_) <= 1.0e-6;
    }

    static double secondsToSamples(double seconds) {
        if (!std::isfinite(seconds) || seconds <= 0.0) {
            return 1.0;
        }

        return std::max(1.0, SampleRate::timeToSamples(seconds));
    }

    static double sanitize(double value) {
        if (!std::isfinite(value)) {
            return 0.0;
        }

        return std::clamp(value, -1.0, 1.0);
    }

    static double slewToward(double value, double target, double step) {
        if (target > value) {
            return std::min(target, value + step);
        }

        return std::max(target, value - step);
    }

    double out_{};
    double currentSlewedValue_{};

    // Parameters.
    Wire attackTime_{ 0.001 };  // seconds
    Wire holdTime_{ 0.001 };    // seconds
    Wire decayTime_{ 0.001 };   // seconds

    // Coefficients.
    double attackSamples_{};
    double holdSamples_{};
    double decaySamples_{};
    double attackStep_{};
    double decayStep_{};
    double holdCounter_{};
};

} // namespace soemdsp::dynamics
