#pragma once

#include "soemdsp/timer/Counter.hpp"
#include "soemdsp/filter/OnePoleFilter.hpp"

namespace soemdsp::dynamics {
struct SilenceDetector {
    SE_CUSTOM_CONSTRUCTOR(SilenceDetector)() {       
        sampleRateChanged();
    }

    void sampleRateChanged() {
        increment_ = SampleRate::timeToIncrement(1.0);
    }

    // io
    // returns true if silence is detected
    bool run(double in) {
        in_    = in;
        double absIn = abs(in);

        counter_ += increment_;

        if (absIn >= silenceThresholdAmplitude) {
            isSilent_ = false;
            counter_ = 0.0;
        } else if (counter_ > 1.0) {
            isSilent_ = true;
        }
        return isSilent_;
    }
    double in_{};
    bool isSilent_{ true };

    // parameters
    double silenceThresholdAmplitude{ 1.e-6 };

    // coefficients
    double counter_{};
    double increment_{};
};
} // namespace soemdsp::dynamics
