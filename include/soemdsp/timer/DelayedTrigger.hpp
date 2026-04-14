#pragma once

#include "soemdsp/Wire.hpp"

#include <functional>

namespace soemdsp::timer {
struct DelayedTrigger {
    // io
    bool isIdle() {
        return isTimerRunning_ || hasTriggered_;
    }
    void reset(double delaySeconds = 0) {
        phase_        = 1.0 - SampleRate::timeToIncrement(delaySeconds);
        hasTriggered_ = false;
    }
    void run() {
        if (!hasTriggered_) {
            if (phase_ >= 1.0) {
                triggerFunction_();
                hasTriggered_ = true;
            }
            phase_ += timerInc_;
        }
    }
    void runIfIsRunning() {
        if (isTimerRunning_ && !hasTriggered_)
            run();
    }
    Wire<double> phase_;         // Set a delay time by calling the reset function. Reset will trigger a callback right away if phase is >= 1.0.
    Wire<double> timerInc_;      // Set by sampleRateChanged().
    bool hasTriggered_{ false }; // Set by run()

    // parameters
    void sampleRateChanged() {
        timerInc_ = std::max(SampleRate::timeToIncrement(triggerTime_), 1.0);
    }
    Wire<double> triggerTime_{ 1.0 }; // sampleRateChanged()
    Wire<int> isTimerRunning_;        // 0 or 1
    std::function<void()> triggerFunction_ = []() {};
};
} // namespace soemdsp::timer
