#pragma once
#include <soemdsp/Wire.hpp>
#include <soemdsp/filter/Smoother.hpp>
#include <soemdsp/semath.hpp>
#include <soemdsp/random/NoiseGenerator.hpp>
//
//LCG (Linear Congruential Generator) is based on arithmetic: It relies on
//modular multiplication and addition. It is essentially "math-based"
//randomness.
//
//LFSR (Linear Feedback Shift Register) is based on bit-shifting and XOR logic:
//It treats the state as a sequence of bits and shifts them according to a
//feedback polynomial. It is essentially "logic-based" randomness.
//
namespace soemdsp::timer {
// Outputs bipolar fixed values based on noise and a phase/increment timer
struct SampleAndHold {
    // parameters
    Wire phase_;
    Wire increment_{ 1.0 };

    // io
    // setting the seed resets the noise generator
    void reset() {
        noise_.reset();
        phase_ = 1.0;
    }

    void trigger() {
        phase_            = math::wrap(phase_);
        lastSampledValue_ = discreteOut_;
        discreteOut_      = noiseOut_;
    }

    void triggerNoPhaseWrap() {
        lastSampledValue_ = discreteOut_;
        discreteOut_      = noiseOut_;
    }

    double run(double input) {
        noiseOut_ = input;
        if (phase_ >= 1.0) {
            trigger();
        }

        phase_ += increment_;

        return discreteOut_;
    }

    double run() {
        noiseOut_ = noise_.runBipolar();
        if (phase_ >= 1.0) {
            trigger();
        }

        phase_ += increment_;

        return discreteOut_;
    }

    double runNoInc() {
        noiseOut_ = noise_.runBipolar();

        return discreteOut_;
    }
    double discreteOut_{};
    double noiseOut_{};
    double lastSampledValue_{};

    // setting the seed resets the noise generator
    void setSeed(int seed) {
        noise_.seed_ = seed;
        // noise_.seedChanged();
    }
    
    // dsp objects
    random::LCG64 noise_;
};
} // namespace soemdsp::timer
