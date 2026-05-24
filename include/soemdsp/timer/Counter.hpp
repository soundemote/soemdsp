#pragma once
#include <functional>
#include <soemdsp/Wire.hpp>
#include <soemdsp/sehelper.hpp>
namespace soemdsp::timer {
struct Counter {
    Counter(double countMax = 1.0)
      : countMax_(countMax) {}

    void reset() {
        count_ = 0.0;
    }

    void increment(double incrementAmt = 1.0) {
        debug::CHECK(incrementAmt >= 0.0);

        //We compare double to double, no casting required
        if (count_ >= countMax_) {
            callback_();
            reset();
        }
        count_ += incrementAmt;
    }

    void decrement(double decrementAmt = 1.0) {
        debug::CHECK(decrementAmt >= 0.0);

        if (count_ >= countMax_) {
            callback_();
            reset();
        }
        count_ -= decrementAmt;
    }

    //Use Wire (which wraps double) for parameters
    double count_{};
    Wire countMax_{ 1.0 };
    void (*callback_)() = op::empty;
};
} //namespace soemdsp::timer
