#pragma once

#include "soemdsp/filter/OnePoleFilter.hpp"
#include "soemdsp/timer/Counter.hpp"

namespace soemdsp::dynamics {
struct EarProtector {
    SE_CUSTOM_CONSTRUCTOR(EarProtector)(double incrementTime = .0005, double decrementTime = .15, double thresholdDecibels = 6.0)
      : incrementTime_(incrementTime)
      , decrementTime_(decrementTime)
      , threshold_(dbToAmp(thresholdDecibels)) {
        sampleRateChanged();
    }

    void sampleRateChanged() {
        hpFilter.frequency_ = 1000;
        hpFilter.sampleRateChanged();
        increment_ = SampleRate::timeToIncrement(incrementTime_);
        decrement_ = SampleRate::timeToIncrement(decrementTime_);
    }

    // io
    // outputs 0 if signal is too loud, 1 if ok
    double run(double in) {
        in_           = in;
        double absIn_ = abs(hpFilter.run(in));

        if (absIn_ >= threshold_) {
            counter_ += increment_;
        }

        if (counter_ >= 1.0) {
            out_ = 0.0;
        } else {
            out_ = 1.0;
        }

        counter_ = std::clamp(counter_, 0.0, 2.0);
        counter_ -= decrement_;

        return out_;
    }
    double in_{};
    double out_{};

    // parameters
    double incrementTime_{ .0005 };     // time it takes to mute while above threshold
    double decrementTime_{ .15 };       // time it takes to unmute
    double threshold_{ dbToAmp(-6.0) }; // threshold in amplitude for when to start timing

    // coefficients
    double counter_{};
    double increment_{};
    double decrement_{};

    // objects
    filter::OnePoleHP hpFilter;
};
} // namespace soemdsp::dynamics
