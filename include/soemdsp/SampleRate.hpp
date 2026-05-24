#pragma once
#include <vector>
#include <soemdsp/Wire.hpp>
#include <soemdsp/random/Random.hpp>
#include <soemdsp/sehelper.hpp>
#include <soemdsp/semath.hpp>
namespace soemdsp {
struct SampleRate {
    static void Update(double samplerate, double blocksize = 512) {
        freq_        = samplerate;
        block_       = blocksize;
        freqInt_     = static_cast<int>(samplerate);
        tau_z_freq_  = constant::kTAU / freq_;
        halffreq_    = freq_ * constant::k1z2;
        quarterfreq_ = freq_ * constant::k1z4;
        period_      = 1.0 / freq_;
    }

    SampleRate(double frequency = 44100.0) {
        Update(frequency);
    }

    inline static double freq_{ 44100.0 };
    inline static int freqInt_{ 44100 };
    inline static double block_{ 512 };
    inline static double halffreq_{ freq_ * constant::k1z2 };
    inline static double quarterfreq_{ freq_ * constant::k1z4 };
    inline static double period_{ 1.0 / freq_ };
    inline static double tau_z_freq_{ constant::kTAU / freq_ };

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
        if (nextVelocity < previousVelocity) {
            return static_cast<int>(timeToSamples(std::min(attackTime + releaseTime * 0.25, .01)));
        } else {
            return static_cast<int>(timeToSamples(std::min(attackTime, .004)));
        }
    }
};
} // namespace soemdsp
