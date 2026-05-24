#pragma once
#include <soemdsp/Wire.hpp>
#include <soemdsp/filter/Smoother.hpp>
#include <soemdsp/semath.hpp>
#include <soemdsp/random/NoiseGenerator.hpp>
// Outputs linearly smoothed bipolar fixed values based on noise, linear smoother, and a phase/increment timer
namespace soemdsp::timer {
struct SampleAndHoldSmoothed {
    SampleAndHoldSmoothed() {}

    random::LCG64 noise_;
    filter::LinearSmoother smoother_;

    //io
    double noiseOut_{};
    double discreteOut_{};
    double continuousOut_{};

    //parameters
    Wire phase_;
    Wire increment_{ 1.0 };

    //setting the seed resets the noise generator
    void reset() {
        noise_.reset();
        phase_ = 1.0;
    }

    void trigger() {
        phase_            = math::wrap(phase_);
        lastSampledValue_ = discreteOut_;
        discreteOut_      = noiseOut_;
        smoother_.setTarget(discreteOut_);
    }

    void triggerNoPhaseWrap() {
        lastSampledValue_ = discreteOut_;
        discreteOut_      = noiseOut_;
        smoother_.setTarget(discreteOut_);
    }

    double run(double input) {
        noiseOut_ = input;
        if (phase_ >= 1.0) {
            trigger();
        }

        phase_ += increment_;

        return continuousOut_ = smoother_.out_;
    }

    double run() {
        noiseOut_ = noise_.runBipolar();
        if (phase_ >= 1.0) {
            trigger();
        }

        phase_ += increment_;

        smoother_.run();

        return continuousOut_ = smoother_.out_;
    }

    double runNoInc() {
        noiseOut_ = noise_.runBipolar();

        smoother_.run();

        return continuousOut_ = smoother_.out_;
    }

  protected:
    double input_{};
    double lastSampledValue_{};
};
} //namespace timer
