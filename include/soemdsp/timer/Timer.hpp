#pragma once

#include "soemdsp/SampleRate.hpp"
#include "soemdsp/Wire.hpp"

#include <functional>

namespace soemdsp::timer {
struct Timer {
    // io
    Wire<double> phase_;    // Set a delay with the help of the reset function. Reset will call trigger function right
                            // away if phase is >= 1.0.
    Wire<double> timerInc_; // Is set by triggerTimeChanged().
    void reset(double delaySeconds = 0) {
        phase_ = 1.0 - SampleRate::timeToIncrement(delaySeconds);
    }
    void run() {
        if (phase_ >= 1.0) {
            triggerFunction_();
            wrap(phase_);
        }

        phase_.w += timerInc_;
    }
    void runIfIsRunning() {
        if (isTimerRunning_)
            run();
    }

    // parameters
    void sampleRateChanged() {
        timerInc_ = std::max(SampleRate::freq_, 1.0);
    }
    Wire<double> triggerTime_{ 1.0 }; // sampleRateChanged()
    Wire<int> isTimerRunning_;        // 0 or 1
    std::function<void()> triggerFunction_ = []() {};
};
} // namespace soemdsp::timer
