#pragma once

#include "soemdsp/SampleRate.hpp"
#include "soemdsp/curve_functions.hpp"
#include "soemdsp/filter/OnePoleFilter.hpp"
#include "soemdsp/sehelper.hpp"
#include "soemdsp/utility/NoiseGenerator.hpp"

#include <magic_enum.hpp>

#include <cmath>
#include <random>
#include <unordered_map>

namespace soemdsp::modulator {

class FlexibleRandomWalk {
  public:
    enum class Method {
        white_noise,
        filtered_white_noise,
        random_steps,
        fixed_steps,
    };

    SE_CUSTOM_CONSTRUCTOR(FlexibleRandomWalk)() {
        lpf_.frequency_.pointTo(frequency_);
    }

    // io
    void sampleRateChanged() {
        frequencyChanged();
    }

    void frequencyChanged() { // frequency_
        lpf_.sampleRateChanged();
        increment_ = SampleRate::frequencyToIncrement(frequency_ * SampleRate::period_);
        updateIncrement();
    }

    void jitterChanged() { // jitter_
        updateIncrement();
        whiteNoiseMixChanged();
    }

    void whiteNoiseMixChanged() { // already called when calling jitterChanged
        double freqAndJitterAvg = (jitterInc_ + increment_) * 0.5;

        whiteNoiseMix_ = 0.0;
        if (freqAndJitterAvg >= startWhiteNoiseMix_) {
            whiteNoiseMix_ = curve::Rational{-0.7}.get(mapNtoN(freqAndJitterAvg, startWhiteNoiseMix_, 1.0, 0.0, 1.0));
        }

        randomMix_ = 1.0 - whiteNoiseMix_;
    }
    double out_{};

    // Parameters: for control outside the class
    Wire<double> frequency_; // 0 to sampleRate
    Wire<double> jitter_;    // value in frequency, must be more than 0 and less than half samplerate
    // Wire<double> gaussianFactor_{ 0.6 }; // 0 to 1 for noise_generator
    Wire<int> gaussianFactor_{ 1 };                    // 1+ for NaiveNoiseGenerator
    Wire<int> method_{ Enum::i(Method::fixed_steps) }; // 0 to 3

    static std::string getMethodString(int method) {
        std::optional opt = magic_enum::enum_cast<Method>(method);
        if (!opt) {
            return "";
        }
        return std::string(magic_enum::enum_name(opt.value()));
    }

    void reset() {
        out_ = 0;
        noiseGenerator_.reset();
        lpf_.reset();
    }

    // depth of value changes, can be from 0 to 1, negative values are ok

    // bipolar values
    double run() {
        switch (magic_enum::enum_cast<Method>(method_).value_or(Method::filtered_white_noise)) {
        case Method::white_noise: {
            return out_ = noiseGenerator_.run(-1.0, +1.0);
        }
        case Method::filtered_white_noise: {
            return out_ = lpf_.run(noiseGenerator_.run(-1.0, +1.0));
        }
        case Method::fixed_steps: {
            double n = noiseGenerator_.run(-1.0, +1.0);
            double r = n > 0.0 ? double(stepSize_) : -double(stepSize_);

            out_ = std::clamp(out_ + r, -1.0, +1.0);

            return lpf_.run(out_ * randomMix_ + n * whiteNoiseMix_);
        }
        case Method::random_steps: {
            double n = noiseGenerator_.run(-1.0, +1.0);
            double r = n * stepSize_;
            out_     = std::clamp(out_ + r, -1.0, +1.0);
            return lpf_.run(out_ * randomMix_ + n * whiteNoiseMix_);
        }
            SE_SWITCH_STATEMENT_FAILURE
        }
        return {};
    }

    utility::NoiseGenerator noiseGenerator_;
    filter::OnePoleLP lpf_;

    void slave(FlexibleRandomWalk& master) {
        gaussianFactor_.pointTo(master.gaussianFactor_);

        // slave does not need to call sampleRateChanged or frequencyChanged
        lpf_.slave(master.lpf_);
        increment_.pointTo(master.increment_);
        frequency_.pointTo(master.frequency_);

        // slave does not need to call updateIncrement
        stepSize_.pointTo(master.stepSize_);
        jitterInc_.pointTo(jitterInc_);

        // slave does not need to call whiteNoiseMixChanged or jitterChanged
        jitter_.pointTo(master.jitter_);
        randomMix_.pointTo(master.randomMix_);
        whiteNoiseMix_.pointTo(master.whiteNoiseMix_);

        // slave gets method from master
        method_.pointTo(master.method_);
    }

  private:
    Wire<double> increment_; // 0 to 1
    Wire<double> stepSize_;  // 0 to 1
    Wire<double> whiteNoiseMix_;
    Wire<double> randomMix_;
    Wire<double> jitterInc_;
    const double startWhiteNoiseMix_{ 0.9 }; // where along the frequency or increment value to start adding white noise

    void updateIncrement() {
        jitterInc_ = SampleRate::frequencyToIncrement(jitter_);
        stepSize_ = increment_;
        stepSize_ += map0to1(curve::Rational{0.99}.get(jitterInc_), -stepSize_, 1.0 - stepSize_);
    }
};
} // namespace soemdsp::modulator
