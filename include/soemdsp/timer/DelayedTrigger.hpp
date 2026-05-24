#pragma once

#include <functional>

#include <soemdsp/SampleRate.hpp>
#include <soemdsp/Wire.hpp>

namespace soemdsp::timer {
struct DelayedTrigger {
    // io
    bool isIdle() {
        return convert::double_to_bool(isTimerRunning_) || hasTriggered_;
    }
    void reset(double delaySeconds = 0) {
        phase_        = 1.0 - SampleRate::timeToIncrement(delaySeconds);
        hasTriggered_ = false;
    }
    void run() {
        if (!hasTriggered_) {
            if (phase_ >= 1.0) {
                func();
                hasTriggered_ = true;
            }
            phase_ += timerInc_;
        }
    }
    void runIfIsRunning() {
        if (isTimerRunning_ && !hasTriggered_)
            run();
    }
    Wire phase_;         // Set a delay time by calling the reset function. Reset will trigger a callback right away if phase is >= 1.0.
    Wire timerInc_;      // Set by sampleRateChanged().
    bool hasTriggered_{}; // Set by run()

    // parameters
    void sampleRateChanged() {
        timerInc_ = std::max(SampleRate::timeToIncrement(triggerTime_), 1.0);
    }
    Wire triggerTime_{ 1.0 }; // sampleRateChanged()
    Wire isTimerRunning_;        // 0 or 1
    void (*func)() = op::empty;
};
} // namespace soemdsp::timer
