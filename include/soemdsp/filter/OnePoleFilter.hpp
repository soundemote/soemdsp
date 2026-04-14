#pragma once

#include "soemdsp/SampleRate.hpp"
#include "soemdsp/Wire.hpp"
#include "soemdsp/semath.hpp"

#include <array>

namespace soemdsp::filter {
class OnePoleBase {
  public:
    OnePoleBase() {
        sampleRateChanged();
    }

    // io
    double in_;
    double out_;

    // parameters
    void sampleRateChanged() {
        w_ = std::min(SampleRate::tau_z_freq_, 0.000142475857) * frequency_;
    }
    Wire<double> frequency_; // sampleRateChanged()

    void reset() {
        buf_[0] = 0.;
        buf_[1] = 0.;
    }

    double run(double input) {
        in_     = input;
        buf_[1] = b0_ * input + b1_ * buf_[0] + a1_ * buf_[1];
        buf_[0] = input;
        out_    = buf_[1];
        return out_;
    }

    // ensures filter runs at the same frequency and all internal variables are updated with master filter except for
    // input signal and final output
    void slave(OnePoleBase& master) {
        b0_.pointTo(master.b0_);
        b1_.pointTo(master.b1_);
        a1_.pointTo(master.a1_);
        w_.pointTo(master.w_);
    }

    // coefficients:
    Wire<double> b0_{ 1.0 };
    Wire<double> b1_;
    Wire<double> a1_;
    Wire<double> w_; // 0 to TAU

    // objects
    std::array<double, 2> buf_{};
};

class OnePoleLP : public OnePoleBase { // MZT
  public:
    void sampleRateChanged() {
        OnePoleBase::sampleRateChanged();
        a1_ = exp(-w_);
        b0_ = 1.0 - a1_;
        b1_ = 0.0;
    }
};

class OnePoleHP : public OnePoleBase { // IIT
  public:
    void sampleRateChanged() {
        OnePoleBase::sampleRateChanged();
        a1_ = exp(-w_);
        b0_ = 0.5 * (1 + a1_);
        b1_ = -b0_;
    }
};
} // namespace soemdsp::filter
