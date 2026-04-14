#pragma once

#include "soemdsp/Wire.hpp"

#include <magic_enum.hpp>

#include <cmath>

namespace soemdsp::oscillator {
class DistortionOscillator {
  public:
    enum class Waveshape {
        AnalogSawSine,
        AnalogSawParabol,
        PerfectSaw,
        AnalogSquare,
        Square,
        Tri,
        BowTri,
        DistortedBowTri,
        WalterWave,
        ParabolSine
    };

#pragma region UPDATE
    void incrementChanged() {
        sineAmp_ = SampleRate::quarterfreq_ / (log10(frequency_) * frequency_) * kPIz2 * 0.8;
    }
    void morphChanged() {
        morphFactor_ = pow(morph_, 4.0) * 0.999 + 0.001;
    }

    /* Parameters */
    Wire<double> frequency_;
    Wire<double> morph_;
    Wire<double> increment_;
    Wire<double> phase_;
    Wire<int> waveform_;
    Wire<int> antialiasingFactor_;
    Wire<double> antialiasingAmplitude_;

    /* Internal */
    Wire<double> sineAmp_;
    Wire<double> morphFactor_{ 1.0 };

    /* Objects */
    random::LCG32 random_;

#pragma endregion

    // void setNoiseSeed(int seed) {
    //     random_.setSeed(seed);
    // }

    static std::string getModeString(int shape) {
        auto shapeNameOptional = magic_enum::enum_cast<DistortionOscillator::Waveshape>(shape);

        if (!shapeNameOptional) {
            return "";
        }

        return std::string(magic_enum::enum_name(shapeNameOptional.value()));
    }

    static double parabolSine(double x) {
        double xin = x;

        if (x > k1z2) {
            xin = x - k1z2;
        }
        xin = xin * 4.0 - 1.0;

        double a = xin * xin;
        if (x > 0.5) {
            return 0.0 - (1.0 - a) * (1.0 - a * 0.202);
        }

        return (1.0 - a) * (1.0 - a * 0.202);
    }

    static double tanHApprox(double x) {
        if (x > 5.0) {
            return 1.0;
        }
        if (x < -5.0) {
            return -1.0;
        }
        double xx = x * x;
        return x / (1.0 + xx / (3.0 + xx / (5.0 + xx / (7.0 + xx / (9.0 + xx * k1z11)))));

        // this equation might also work: x*(27.0 + xx) / (27.0 + 9.0 * xx)
    }

    double calculateState() {
        phase_.w += increment_;
        phase_.w           = wrap(phase_);
        return finalPhase_ = wrap(phase_);
    }

    double calculateStateWithAntialiasing() {
        phase_ += increment_;
        phase_             = wrap(phase_);
        return finalPhase_ = wrap(phase_ + antialiasingAmplitude_ /* *random_.runGaussian(antialiasingFactor_)*/);
    }

    void reset() {
        phase_.w = 0;
    }

    double runNoCalculateState() {
        switch (static_cast<Waveshape>(*waveform_.r)) {
        case Waveshape::AnalogSawSine: {
            double toSine = (finalPhase_ * 2.0 - 1.0) * kPI;
            return tanh(sin(toSine) * sineAmp_ * morphFactor_) * cos(toSine);
        }
        case Waveshape::AnalogSawParabol: {
            return tanh(parabolSine(finalPhase_) * sineAmp_ * morphFactor_) * parabolSine(fmod((finalPhase_ + k1z4), 1));
        }
        case Waveshape::PerfectSaw: {
            return acos(tanh(sin(finalPhase_ * kPI * 2) * sineAmp_ * morphFactor_) * sin(fmod((finalPhase_ + k1z4), 1.0) * kPI * 2)) / (kPI * k1z2) - 1;
        }
        case Waveshape::AnalogSquare: {
            return tanh(parabolSine(finalPhase_) * sineAmp_ * morphFactor_) * (tanh(parabolSine(finalPhase_) * sineAmp_ * k1z2 * morphFactor_) * parabolSine(fmod((finalPhase_ + k1z4), 1)) * k1z2 + k1z2);
        }
        case Waveshape::Square: {
            return tanh(parabolSine(finalPhase_) * sineAmp_ * morphFactor_);
        }
        case Waveshape::Tri: {
            double adjustedMorphFactor_ = map0to1<double>(morphFactor_, 0.15, 1.0);
            double scaling              = tanh((1.0 - (frequencyToPitch(frequency_) / 127.0)) * 9.0);
            return acos(sin(finalPhase_ * kPI * 2) * adjustedMorphFactor_ * scaling) / kPI * 2.0 - 1.0;
        }
        case Waveshape::BowTri: {
            double bow = parabolSine(finalPhase_);
            return uniToBi(tanh(bow * sineAmp_ * morphFactor_) * bow);
        }
        case Waveshape::DistortedBowTri: {
            double bow = parabolSine(finalPhase_);
            double sq  = tanh(bow * sineAmp_ * morphFactor_);
            return uniToBi(tanh(sq * bow * 2));
        }
        case Waveshape::WalterWave: {
            double bow = parabolSine(finalPhase_);
            double sq  = tanh(bow * sineAmp_ * morphFactor_);
            return sq * k1z2 + k1z2 - tanh(sq * bow * 2);
        }
        case Waveshape::ParabolSine: {
            return parabolSine(finalPhase_);
        }
            SE_SWITCH_STATEMENT_FAILURE
        }
        return {};
    }

    double run() {
        if (antialiasingAmplitude_ > 0) {
            calculateStateWithAntialiasing();
        } else {
            calculateState();
        }
        return runNoCalculateState();
    }

  private:
    double finalPhase_{};
};
} // namespace soemdsp::oscillator
