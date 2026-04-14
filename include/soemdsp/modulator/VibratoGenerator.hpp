#pragma once

#include "soemdsp/SampleRate.hpp"
#include "soemdsp/Wire.hpp"
#include "soemdsp/oscillator/PolyBLEP.hpp"
#include "soemdsp/timer/SampleAndHold.hpp"

namespace soemdsp::modulator {
using std::clamp;
class VibratoGenerator {
  public:
    VibratoGenerator() {
        osc_.waveform_.w = Enum::i(oscillator::PolyBLEP::Shape::Trisaw);
        connect(osc_.phase_.r, osc_.phaseOffset_.r, osc_.increment_.r);
    }

    oscillator::PolyBLEP osc_;
    timer::SampleAndHoldSmoothed snhfreq_; // needs seed
    timer::SampleAndHoldSmoothed snhamp_;  // needs seed

    Wire<double> phase_;
    Wire<double> phaseOffset_;
    Wire<double> increment_;
    Wire<double> morph_;
    Wire<double> randomFreqMult_;
    Wire<double> randomAmpMult_;

    double oscOut_;
    double incrementOut_;
    double randFrqOut_;
    double randAmpOut_;

    double ampRandomOffset_;
    double freqRandomOffset_;

    // Generally you want amp and freq drift to have the same frequency of the vibrato osc, call this function instead
    // of freq/amp seperately
    void incrementChanged() {
        frqDriftIncrementChanged();
        ampDriftIncrementChanged();
    }

    void frqDriftIncrementChanged() {
        snhfreq_.smoother_.timeInSamples_ = SampleRate::incrementToSamples(increment_);
        snhfreq_.smoother_.incrementChanged();
    }

    void ampDriftIncrementChanged() {
        snhamp_.smoother_.timeInSamples_ = SampleRate::incrementToSamples(increment_);
        snhamp_.smoother_.incrementChanged();
    }

    // set the seed for both freq and amp randomizers
    void seedChanged() {
        // snhfreq_.seedChanged();
        // snhamp_.seedChanged();
    }

    void connect(double* phase, double* phaseOffset, double* increment) {
        osc_.phase_.pointTo(phase);
        osc_.phaseOffset_.pointTo(phaseOffset);
        osc_.increment_.pointTo(increment);
    }

    void reset() {
        phase_ = phaseOffset_;
    }

    double run() {
        randFrqOut_ = snhfreq_.runNoInc() * randomFreqMult_;
        randAmpOut_ = snhamp_.runNoInc() * randomAmpMult_;

        double inc = increment_;
        phase_.w += inc + inc * randFrqOut_;

        if (phase_ >= 1.0) {
            phase_ = wrap(phase_);
            snhfreq_.triggerNoPhaseWrap();
            snhamp_.triggerNoPhaseWrap();
        }

        auto waveform = magic_enum::enum_cast<oscillator::PolyBLEP::Shape>(*osc_.waveform_.r);
        oscOut_       = osc_.get(waveform.value(), abs(increment_), phase_, morph_);
        oscOut_       = (oscOut_ + oscOut_ * randAmpOut_);

        return oscOut_;
    }
};
} // namespace soemdsp::modulator
