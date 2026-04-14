#pragma once

#include "soemdsp/SampleRate.hpp"
#include "soemdsp/Wire.hpp"
#include "soemdsp/semath.hpp"
#include "RSMET/rosic/rosic_CookbookFilter.h"

#include <array>

namespace soemdsp::filter {
struct MultiStageFilter {
    SE_DEFAULT_CONSTRUCTOR(MultiStageFilter);

    using Mode = rosic::CookbookFilter::modes;

    // io
    double in_{};
    double out_{};

    // parameters
    void sampleRateChanged() {
        filter_.setSampleRate(SampleRate::freq_);
    }
    void modeChanged() { // mode_
        filter_.setMode(mode_);
    }
    void frequencyChanged() { // frequency_
        filter_.setFreq(frequency_);
        filter_.calcCoeffs();
    }
    void gainChanged() { // gain_
        filter_.setGain(gain_);
        filter_.calcCoeffs();
    }
    void stagesChanged() { // stages_
        if (prevStages_ == stages_)
            return;

        if (stages_ == 0) {
            filter_.setMode(Mode::BYPASS);
        } else if (prevStages_ == 0) {
            filter_.setMode(mode_);
        }
        else {
            filter_.setNumStages(stages_);
        }

        prevStages_ = stages_;
    }
    void qChanged() { // q_
        filter_.setQ(q_);
        filter_.calcCoeffs();
    }
    Wire<double> frequency_{ 1000.0 };
    Wire<int> stages_{ 2 };                    // 0 to 5, 0 is bypass
    Wire<double> q_{ 1.0 };                    // 0 to 5, 0 is bypass
    Wire<double> gain_{ 0.0 };                 // decibels
    Wire<int> mode_{ Enum::i(Mode::LOWPASS) }; // 0 to 9

    void reset() {
        filter_.resetBuffers();
    }

    double run(double input) {
        in_  = input;
        out_ = filter_.getSample(in_);
        return out_;
    }

    // ensures filter runs at the same frequency and all internal variables are updated with master filter except for
    // input signal and final output
    void slave(MultiStageFilter& master) {
        frequency_.pointTo(master.frequency_);
        stages_.pointTo(master.stages_);
        q_.pointTo(master.q_);
        gain_.pointTo(master.gain_);
        mode_.pointTo(master.mode_);
    }

    // objects
    rosic::CookbookFilter filter_;

    protected:
    int prevStages_{ stages_ };
};

struct StereoMultiStageFilter {
    SE_CUSTOM_CONSTRUCTOR(StereoMultiStageFilter)() {
        filterL_.frequency_.pointTo(frequency_);
        filterL_.stages_.pointTo(stages_);
        filterL_.q_.pointTo(q_);
        filterL_.gain_.pointTo(gain_);
        filterL_.mode_.pointTo(mode_);

        filterR_.slave(filterL_);
    }

    using Mode = rosic::CookbookFilter::modes;

    // io
    void reset() {
        filterL_.reset();
        filterR_.reset();
    }
    void run(double inL, double inR) {
        outL_ = filterL_.run(inL_ = inL);
        outR_ = filterR_.run(inR_ = inR);
    }
    void run(double inL, double inR, double *outL, double *outR) {
        run(inL, inR);
        *outL = outL_;
        *outR = outR_;
    }
    double inL_{}, inR_{};
    double outL_{}, outR_{};

    // parameters
    void sampleRateChanged() {
        filterL_.sampleRateChanged();
        filterR_.sampleRateChanged();
        //filterR_.filter_.copyCoeffs(filterL_.filter_);
    }
    void modeChanged() { // mode_
        filterL_.modeChanged();
        filterR_.modeChanged();
        //filterR_.filter_.copyCoeffs(filterL_.filter_);
    }
    void frequencyChanged() { // frequency_
        filterL_.frequencyChanged();
        filterR_.frequencyChanged();
        //filterR_.filter_.copyCoeffs(filterL_.filter_);
    }
    void gainChanged() { // gain_
        filterL_.gainChanged();
        filterR_.gainChanged();
        //filterR_.filter_.copyCoeffs(filterL_.filter_);
    }
    void stagesChanged() { // stages_
        filterL_.stagesChanged();
        filterR_.stagesChanged();
    }
    void qChanged() { // q_
        filterL_.qChanged();
        filterR_.qChanged();
        //filterR_.filter_.copyCoeffs(filterL_.filter_);
    }
    Wire<double> frequency_{1000.0};
    Wire<int> stages_{2};  // 0 to 5, 0 is bypass
    Wire<double> q_{1.0};    // 0.0 to 10+, higher values being more resonant
    Wire<double> gain_{0.0}; // decibels
    Wire<int> mode_{ Enum::i(Mode::LOWPASS) }; // 0 to 9

    MultiStageFilter filterL_, filterR_;
};
} // namespace soemdsp::filter

