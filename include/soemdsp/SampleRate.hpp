#pragma once

#include "Wire.hpp"
#include "random/Random.hpp"
#include "sehelper.hpp"
#include "semath.hpp"

#include <vector>

namespace soemdsp {
struct SampleRate {
    static void Update(double samplerate, double blocksize = 512) {
        freq_        = samplerate;
        block_       = blocksize;
        freqInt_     = toInt(samplerate);
        tau_z_freq_  = kTAU / freq_;
        halffreq_    = freq_ * k1z2;
        quarterfreq_ = freq_ * k1z4;
        period_      = 1.0 / freq_;
    }

    SampleRate(double frequency = 44100.0) {
        Update(frequency);
    }

    inline static double freq_{ 44100.0 };
    inline static int freqInt_{ 44100 };
    inline static double block_{ 512 };
    inline static double halffreq_{ freq_ * k1z2 };
    inline static double quarterfreq_{ freq_ * k1z4 };
    inline static double period_{ 1.0 / freq_ };
    inline static double tau_z_freq_{ kTAU / freq_ };

    static double samplesToTime(double samples) {
        return samples * SampleRate::period_;
    }

    static double frequencyToSamples(double freq) {
        return 1.0 / freq * SampleRate::freq_;
    }

    static double frequencyToIncrement(double freq) {
        return freq * SampleRate::period_;
    }

    static double incrementToFrequency(double inc) {
        return inc * SampleRate::freq_;
    }

    static double incrementToSamples(double inc) {
        if (inc <= SampleRate::period_) {
            return 1;
        }
        return 1.0 / inc;
    }

    static double samplesToIncrement(double samples) {
        return samples * SampleRate::period_;
    }

    // time * sampleRate, amount of time for incrementer to go from 0.0 to sample rate samples
    static double timeToSamples(double time) {
        return time * SampleRate::freq_;
    }

    // amount of time for incrementer to go from 0.0 to 1.0
    static double timeToIncrement(double time) {
        return 1.0 / (time * SampleRate::freq_);
    }

    static int velocitySmoothingFormula(double attackTime, double releaseTime, double previousVelocity, double nextVelocity) {
        if (nextVelocity < previousVelocity)
            return toInt(timeToSamples(std::min(attackTime + releaseTime * 0.25, .01)));
        else
            return toInt(timeToSamples(std::min(attackTime, .004)));
    }
};

struct GlobalRandom {
    SE_CUSTOM_CONSTRUCTOR(GlobalRandom)() {
        random_.setSeedAndReset(rand());
    }

    static void resetWithSeedAndNotify(int seed) {
        random_.setSeedAndReset(seed);
        seedChangedCallback_();
    }

    static void reset() {
        random_.reset();
    }

    static double nextBi() {
        return random_.runBipolar();
    }

    static double nextUni() {
        return random_.runUnipolar();
    }

    // populate this function to update all objects depending on random values
    inline static std::function<void()> seedChangedCallback_ = []() {};

    inline static random::LCG32 random_;
};
} // namespace soemdsp
