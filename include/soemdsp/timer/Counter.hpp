#pragma once

#include <functional>

#include "soemdsp/sehelper.hpp"
#include "soemdsp/Wire.hpp"

namespace soemdsp::timer {
struct Counter {
    Counter(int countMax = 1) : countMax_(countMax){}
    //io
    int count_ = 0; // number of times incremented
    void reset() {
        count_ = 0;
    }
    void increment(int incrementAmt = 1) {
        SE_ASSERT(incrementAmt >= 0);
        if (count_ >= countMax_) {
            callback_();
            reset();
         }

        count_ += incrementAmt;
    }

    void decrement(int decrementAmt = 1) {
        SE_ASSERT(decrementAmt >= 0);
        if (count_ >= countMax_) {
            callback_();
            reset();
        }

        count_ -= decrementAmt;
    }

    // parameters
    Wire<int> countMax_{ 1 }; // must be at least 1
    std::function<void()> callback_ = []() {};

};
} // namespace soemdsp::timer
