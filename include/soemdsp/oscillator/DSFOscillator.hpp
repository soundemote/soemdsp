#pragma once

#include "soemdsp/SampleRate.hpp"
#include "soemdsp/Wire.hpp"
#include "soemdsp/sehelper.hpp"
#include "soemdsp/semath.hpp"

#include <array>
#include <cassert>
#include <cstddef>

/*
 * PROBLEMS:
 * - attack causes amplitude spike
 * - morph_ not consistent in volume
 * - over frequency is not consistent in volume
 * - harmonics get cut as frequency goes higher and clicks in and out
 * - between saw and square volume is inconsistent
 * - square gets dull as frequency drops
 */

namespace soemdsp::oscillator {

struct DSFOscillatorBase {
    SE_DEFAULT_CONSTRUCTOR(DSFOscillatorBase);

    void reset() {
        phase_.w = 0.;
        leak_    = 1.;
    }

    // io
    double out_{};

    // If slaving phase, only master needs to call calculateState
    double calculateState() {
        phase_ += increment_ * 0.9999; // added 0.9999 to help prevent errors
        phase_    = wrap(phase_);
        dsfState_ = wrap(phase_) * kTAU;
        return dsfState_;
    }

    // This gets the actual oscillator output
    double run() {
        leak_            = leak_ * 0.99 + 0.000005;
        preAmpAdjustOut_ = preAmpAdjustOut_ * (1.0 - leak_);
        preAmpAdjustOut_ += DSF() * increment_;
        return out_ = preAmpAdjustOut_ * ampAdjust_;
    }

    /* Parameters */
    virtual void frequencyChanged() {} // Run when samplerate, frequency, or increment changes
    virtual void morphChanged() {}     // Run when morph changes
    Wire<double> increment_;           // frequencyChanged()
    Wire<double> phase_;
    Wire<double> phaseOffset_;
    Wire<double> frequency_;    // frequencyChanged()
    Wire<double> morph_{ 1.0 }; // morphChanged()
    Wire<double> dsfState_;     // a function of phase

    void slave(DSFOscillatorBase& master) {
        morph_.pointTo(master.morph_);
        increment_.pointTo(master.increment_);
        frequency_.pointTo(master.frequency_);
        phase_.pointTo(master.phase_);
        dsfState_.pointTo(master.dsfState_);
    }

    // only master needs to call calculateState(), connects phase_ and dsfState_
    void slavePhase(DSFOscillatorBase& master) {
        phase_.pointTo(master.phase_);
        dsfState_.pointTo(master.dsfState_);
    }

  protected:
    virtual double DSF() {
        return 0;
    }

    Wire<double> ampAdjust_{ 1. };
    Wire<int> numPartials_;
    double leak_{ 1. };
    Wire<double> k_;
    Wire<double> k2_;
    Wire<double> k42_;
    Wire<double> preAmpAdjustOut_;
};

struct DSFOscillatorSineSaw : public DSFOscillatorBase {
    SE_DEFAULT_CONSTRUCTOR(DSFOscillatorSineSaw);
#pragma region UPDATE
    // Run when samplerate, frequency, or increment changes
    void frequencyChanged() override {
        numPartials_ = frequency_ == 0.0 ? 1 : static_cast<int>(SampleRate::halffreq_ / frequency_);
    }

    // Run when morph changes
    void morphChanged() override {
        SE_ERROR(morph_ >= 0.0 && morph_ <= 1.0, "Morph value when combined with modulation is not between 0 and 1");
        k_   = (1.0 - std::pow(morph_, 0.14)) * 4.0;
        k2_  = k_ * k_;
        k42_ = std::pow(4.0, k2_);

        // amplitude equalization for oscillator over morph_
        ampAdjust_ = map0to1<double>(morph_, 3.15, 2.7);
    }
#pragma endregion

    // only master runs morphChanged
    void slaveMorph(DSFOscillatorSineSaw& master) {
        morph_.pointTo(master.morph_);
        k_.pointTo(master.k_);
        k2_.pointTo(master.k2_);
        k42_.pointTo(master.k42_);
        ampAdjust_.pointTo(master.ampAdjust_);
    }

    // only master calculates increment and frequency.
    void slaveIncrement(DSFOscillatorSineSaw& master) {
        increment_.pointTo(master.increment_);
        frequency_.pointTo(master.frequency_);
    }

  private:
    double DSF() override {
        double x_    = dsfState_;
        double xn    = dsfState_ * toDouble(numPartials_);
        double cosx  = std::cos(dsfState_);
        double cosxn = std::cos(xn);

        return (morph_ < 0 ? -1.0 : 1.0) * ((k42_ * cosxn - pow(8.0, k2_) * (cosxn * cosx - sin(xn) * sin(x_))) * pow(2.0, -k2_ * (numPartials_ + 1.0)) + cosx * k42_ - pow(2.0, k2_)) /
               (1.0 - pow(2.0, 1.0 + k2_) * cosx + k42_);
    }
};

struct DSFOscillatorSineSquare : public DSFOscillatorBase {
    SE_DEFAULT_CONSTRUCTOR(DSFOscillatorSineSquare);
    // Run when samplerate, frequency, or increment changes
    void frequencyChanged() override {
        numPartials_ = static_cast<int>(frequency_ == 0.0 ? 2 : SampleRate::halffreq_ / frequency_) / 2;
    }

    void morphChanged() override {
        k_ = 1 - (1.0 / (std::pow((morph_ / 2.0 + 0.25), 14.0) * 10000. + 1.0)) + .000000000001;

        // amplitude equalization for oscillator over morph_
        ampAdjust_ = map0to1<double>(morph_, 0.34, 0.81);
    }

    // only master runs morphChanged
    void slaveMorph(DSFOscillatorSineSquare& master) {
        morph_.pointTo(master.morph_);
        k_.pointTo(master.k_);
        k2_.pointTo(master.k2_);
        k42_.pointTo(master.k42_);
        ampAdjust_.pointTo(master.ampAdjust_);
    }

    // only master calculates increment and frequency.
    void slaveIncrement(DSFOscillatorSineSquare& master) {
        increment_.pointTo(master.increment_);
        frequency_.pointTo(master.frequency_);
    }

  private:
    double DSF() override {
        double x_          = dsfState_;
        double pow_k_n_p_1 = std::pow(k_, numPartials_ + 1.);
        return 8.0 * ((pow_k_n_p_1 * k_ * std::cos(x_ * (2. * numPartials_ - 1.)) - pow_k_n_p_1 * std::cos(x_ * (2. * numPartials_ + 1.)) - k_ * std::cos(x_) * (k_ - 1.)) / k_ /
                      (1. + k_ * k_ - 2. * k_ * std::cos(2. * x_)));
    }
};

} // namespace soemdsp::oscillator
