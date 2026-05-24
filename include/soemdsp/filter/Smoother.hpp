#pragma once

#include "../../../libs/RSMET/SmoothingFilter.hpp"
#include <cassert>

#include <soemdsp/SampleRate.hpp>
#include <soemdsp/Wire.hpp>
#include <soemdsp/sehelper.hpp>

namespace soemdsp::filter {
struct SmootherBase {
    SmootherBase() = default;

    // io
    virtual void reset() {
        out_ = in_;
    }
    virtual void setTarget(double) = 0;
    virtual void setState(double v) {
        out_ = in_ = v;
    }

    virtual void setTimeInSamples(double v) {
        timeInSamples_ = v;
        incrementChanged();
    }
    bool needsSmoothing() {
        return !math::isNear(out_, in_, 1.e-7);
    } // NOLINT

    // parameters
    // call when in_ changes and you've not changed in_ via setTarget
    virtual void targetChanged() {}
    // call whenever timeInSamples, sampleRate, and/or in_ depending on the smoother type
    virtual void incrementChanged() = 0;
    Wire timeInSamples_{ 512.0 };
    Wire in_;
    double out_{};

    std::function<void(void)> callback_ = []() {};

    // master takes care of increment, target value updates, and assigns the callback to the slave
    void slave(SmootherBase& master) {
        timeInSamples_.pointTo(master.timeInSamples_);
        in_.pointTo(master.in_);
        callback_ = master.callback_;
    }

    virtual double run() = 0;
    bool hasOvershot(double increment, double currentValue, double targetValue) {
        return (increment > 0.0) ? (currentValue > targetValue) : (currentValue < targetValue);
    }
};

struct LinearSmoother : public SmootherBase {
    LinearSmoother() = default;

    // io
    void setTarget(double v) {
        in_ = v;
        incrementChanged();
    }

    // parameters
    void targetChanged() override {
        incrementChanged();
    }
    // call whenever timeInSamples or sampleRate changes. This is called by setTarget already.
    void incrementChanged() override {
        increment_ = (in_ - out_) / std::max<double>(1.0, timeInSamples_);
    }

    // coefficients
    Wire increment_; // incrementChanged() // based on timeInSamples, used to connect multiple smoothers
    double run() override {
        out_ += increment_;

        if (hasOvershot(increment_, out_, in_)) {
            setState(in_);
        }

        callback_();

        return out_;
    }
};

class ExponentialSmoother : public SmootherBase {
  public:
    ExponentialSmoother() {
        filter_.setOrder(2);
        filter_.setShapeParameter(1);
    }
#pragma region UPDATE
    void setTarget(double v) {
        in_ = v;
    }

    // call whenever timeInSamples or sampleRate changes
    void incrementChanged() override {
        filter_.setTimeInSamples(timeInSamples_);
    }

#pragma endregion

    void setState(double v) override {
        filter_.setStates(v);
        in_  = v;
        out_ = in_;
    }

    void reset() override {
        setState(in_);
    }

    double run() override {
        out_ = filter_.getSample(in_);

        if (abs(out_ - in_) <= 1.e-6) {
            setState(in_);
        }

        callback_();

        return out_;
    }

    rsSmoothingFilter filter_;
};

class LinExpSmoother : public SmootherBase {
  public:
    LinExpSmoother() {
        filter_.setOrder(2);
        filter_.setShapeParameter(1);
    }
#pragma region UPDATE

    void linOrExpChanged() {
        incrementChanged();
    }

    bool constantTime{ true };

    // call whenever timeInSamples or sampleRate changes
    void incrementChanged() override {
        switch (convert::double_to_int(linOrExp_)) {
        case 0:
            if (!constantTime) {
                increment_ = (in_ - out_) / std::max<double>(1.0, timeInSamples_);
            } else {
                increment_ = ((out_ < in_) ? 1.0 : -1.0) * (1.0 / SampleRate::samplesToTime(static_cast<int>(std::max<double>(1.0, timeInSamples_)))) / SampleRate::freq_;
            }
            break;
        case 1:
            filter_.setTimeConstantAndSampleRate(SampleRate::samplesToTime(static_cast<int>(timeInSamples_)), SampleRate::freq_);
            break;
        default:
            debug::FAIL();
        }
    }

    Wire increment_; // based on timeInSamples, used to connect multiple smoothers
    Wire linOrExp_;     // linOrExpChanged()
#pragma endregion
    void setTarget(double v) {
        in_ = v;
        incrementChanged();
    }

    void setState(double v) override {
        filter_.setStates(v);
        out_ = in_ = v;
    }

    void reset() override {
        setState(in_);
    }

    double run() override {
        switch (convert::double_to_int(linOrExp_)) {
        case 0:
            out_ += increment_;
            if (hasOvershot(increment_, out_, in_)) {
                setState(in_);
            }
            break;
        case 1:
            out_ = filter_.getSample(in_);
            if (abs(out_ - in_) <= 1.e-6) {
                setState(in_);
            }
            break;
        default:
            debug::FAIL();
        }

        callback_();
        return out_;
    }

    rsSmoothingFilter filter_;
};

} // namespace soemdsp::filter
