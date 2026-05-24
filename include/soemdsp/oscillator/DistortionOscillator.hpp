#pragma once

#include <cmath>
#include <soemdsp/SampleRate.hpp>
#include <magic_enum/magic_enum.hpp>
#include <soemdsp/Wire.hpp>
#include <soemdsp/semath.hpp>

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
        sineAmp_ = SampleRate::quarterfreq_ / (log10(frequency_) * frequency_) * constant::kPIz2 * 0.8;
    }
    void morphChanged() {
        morphFactor_ = pow(morph_, 4.0) * 0.999 + 0.001;
    }

    /* Parameters */
    Wire frequency_;
    Wire morph_;
    Wire increment_;
    Wire phase_;
    Wire waveform_;
    Wire antialiasingFactor_;
    Wire antialiasingAmplitude_;

    /* Internal */
    Wire sineAmp_;
    Wire morphFactor_{ 1.0 };

    /* Objects */
    random::LCG64 random_;

#pragma endregion

    //void setNoiseSeed(int seed) {
    //    random_.setSeed(seed);
    //}

    static std::string getModeString(int shape) {
        auto shapeNameOptional = magic_enum::enum_cast<DistortionOscillator::Waveshape>(shape);

        if (!shapeNameOptional) {
            return "";
        }

        return std::string(magic_enum::enum_name(shapeNameOptional.value()));
    }

    static double parabolSine(double x) {
        double xin = x;

        if (x > constant::k1z2) {
            xin = x - constant::k1z2;
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
        return x / (1.0 + xx / (3.0 + xx / (5.0 + xx / (7.0 + xx / (9.0 + xx * constant::k1z11)))));

        //this equation might also work: x*(27.0 + xx) / (27.0 + 9.0 * xx)
    }

    double calculateState() {
        phase_.w += increment_;
        phase_.w           = math::wrap(phase_);
        return finalPhase_ = math::wrap(phase_);
    }

    double calculateStateWithAntialiasing() {
        phase_ += increment_;
        phase_             = math::wrap(phase_);
        return finalPhase_ = math::wrap(phase_ + antialiasingAmplitude_ /* *random_.runGaussian(antialiasingFactor_)*/);
    }

    void reset() {
        phase_.w = 0;
    }

    double runNoCalculateState() {
        switch (static_cast<Waveshape>(*waveform_.r)) {
        case Waveshape::AnalogSawSine: {
            double toSine = (finalPhase_ * 2.0 - 1.0) * constant::kPI;
            return tanh(sin(toSine) * sineAmp_ * morphFactor_) * cos(toSine);
        }
        case Waveshape::AnalogSawParabol: {
            return tanh(parabolSine(finalPhase_) * sineAmp_ * morphFactor_) * parabolSine(fmod((finalPhase_ + constant::k1z4), 1));
        }
        case Waveshape::PerfectSaw: {
            return acos(tanh(sin(finalPhase_ * constant::kPI * 2) * sineAmp_ * morphFactor_) * sin(fmod((finalPhase_ + constant::k1z4), 1.0) * constant::kPI * 2)) / (constant::kPI * constant::k1z2) - 1;
        }
        case Waveshape::AnalogSquare: {
            return tanh(parabolSine(finalPhase_) * sineAmp_ * morphFactor_) * (tanh(parabolSine(finalPhase_) * sineAmp_ * constant::k1z2 * morphFactor_) * parabolSine(fmod((finalPhase_ + constant::k1z4), 1)) * constant::k1z2 + constant::k1z2);
        }
        case Waveshape::Square: {
            return tanh(parabolSine(finalPhase_) * sineAmp_ * morphFactor_);
        }
        case Waveshape::Tri: {
            double adjustedMorphFactor_ = math::map0to1<double>(morphFactor_, 0.15, 1.0);
            double scaling              = tanh((1.0 - (convert::freq_to_pitch(frequency_) / 127.0)) * 9.0);
            return acos(sin(finalPhase_ * constant::kPI * 2) * adjustedMorphFactor_ * scaling) / constant::kPI * 2.0 - 1.0;
        }
        case Waveshape::BowTri: {
            double bow = parabolSine(finalPhase_);
            return convert::to_bipolar(tanh(bow * sineAmp_ * morphFactor_) * bow);
        }
        case Waveshape::DistortedBowTri: {
            double bow = parabolSine(finalPhase_);
            double sq  = tanh(bow * sineAmp_ * morphFactor_);
            return convert::to_bipolar(tanh(sq * bow * 2));
        }
        case Waveshape::WalterWave: {
            double bow = parabolSine(finalPhase_);
            double sq  = tanh(bow * sineAmp_ * morphFactor_);
            return sq * constant::k1z2 + constant::k1z2 - tanh(sq * bow * 2);
        }
        case Waveshape::ParabolSine: {
            return parabolSine(finalPhase_);
        }
        default:
            debug::FAIL();
        }
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
} //namespace soemdsp::oscillator
