#pragma once

#include <functional>

#include <soemdsp/SampleRate.hpp>
#include <soemdsp/Wire.hpp>

namespace soemdsp::timer {
struct Timer {
    // io
    Wire phase_;    // Set a delay with the help of the reset function. Reset will call trigger function right
                            // away if phase is >= 1.0.
    Wire timerInc_; // Is set by triggerTimeChanged().
    void reset(double delaySeconds = 0) {
        phase_ = 1.0 - SampleRate::timeToIncrement(delaySeconds);
    }
    void run() {
        if (phase_ >= 1.0) {
            func();
            math::wrap(phase_);
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
    Wire triggerTime_{ 1.0 }; // sampleRateChanged()
    Wire isTimerRunning_;        // 0 or 1
    void (*func)() = op::empty;
};
} // namespace soemdsp::timer
