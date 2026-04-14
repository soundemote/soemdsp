#pragma once

#include "soemdsp/SampleRate.hpp"
#include "soemdsp/Wire.hpp"
#include "soemdsp/sehelper.hpp"
#include "soemdsp/additive/layer.hpp"
#include <magic_enum.hpp>

#include <algorithm>
#include <string>
#include <Vector>
#include <array>

namespace soemdsp::additive {

struct Waveform : public LayerBase {
    SE_CUSTOM_CONSTRUCTOR(Waveform)();

    enum class Shape {
        // Basic
        Sine,
        Triangle,
        Sawtooth,
        Square,

        // Classic
        TriSaw,
        SawSquare,
        DoubleSaw,
        MultiSaw,
        PulseCenter,
        PulseLeft,
        PulseRight,   
        SquareDoubleSaw,
        RoundedSquareDoubleSaw,     

        // Exotic
        RectifiedSine,
        RectifiedSineTri,

        // Custom
        MultiPulse1,
        MultiPulse2,
        Organ,
    };

    enum class Par {
        shape,
        modA,
        modB
    };

    static std::string_view waveshapeStr(int waveform) {
        return Enum::str<Shape>(waveform);
    }

    std::string_view modAStr() {
        double v = mod_[Enum::sizeT(Par::modA)];
        switch (Enum::get<Shape>(shape_)) {
        case Shape::Sine:
            return fmt::format("harmonic: {}", toInt(v) * 99 + 1);
        case Shape::SawSquare:
            return fmt::format("mix: {:.2f}", v);
        case Shape::DoubleSaw:
            return fmt::format("pwm: {:.2f}", v);
        case Shape::PulseCenter:
            return fmt::format("pwm: {:.2f}", uniToBi(v));
        case Shape::TriSaw:
            return fmt::format("peak: {:.2f}", uniToBi(v));
        case Shape::Organ:
            return fmt::format("harm: {}", static_cast<int>(map0to1(v, 2.0, 13.0)));
        default:
            return fmt::format("mod: {:.2f}", v);
        }
    }
    std::string modBStr() {
        return fmt::format("mod: {:.2f}", mod_[Enum::sizeT(Par::modB)]);
    }

    // io
    void apply() override {
        for (size_t n = 0; n < hm_->currentSize_; ++n) {
            hm_->amplitude_[n] *= amplitude_[n];
            hm_->phase_[n] += phase_[n];
        }
    }
    void resize() override {
        size_t sizeDelta  = hm_->currentSize_ - phase_.size();
        size_t stardIndex = phase_.size() - 1;
        size_t endIndex   = stardIndex + sizeDelta;

        if (sizeDelta > 0) {
            shapeChanged(stardIndex, endIndex);
        }
    }

  private:
    void shapeChanged(int startIndex, int endIndex);
    int shape_{};
    size_t n_{};       // current harmonic index
    double h_{};       // current harmonic index
    double a_{};       // last calculated amplitude
    double p_{};       // last calculated phase;
    int counter_{ 1 }; // used in organ algorithm
    std::array<double, MAX_HARMONICS> phase_;
    std::array<double, MAX_HARMONICS> amplitude_;

    void sine(double select = 0.0);
    void triangle();
    void sawtooth();
    void square(double phaseRotate = 0.0);
    void sawsquare(double mix = 0.0);
    void rectifiedSine();
    void rectifiedSineTri(double mod = 0.0);
    void roundedSquareDoubleSaw(double mod = 0.0);
    void squareDoubleSaw(double mod = 0.0);
    void doublesaw(double pwm = 0.0);    
    void pulseCenter(double pwm = 0.5);
    void pulseLeft(double pwm = 0.5);
    void pulseRight(double pwm = 0.5);
    void multipulse1(double pwm = 0.5);
    void multipulse2(double pwm = 0.5);
    void multisaw(double pwm = 0.5);
    void trisaw(double pwm = 0.5);    
    void organ(int octaves = 2);
};
} // namespace soemdsp::additive
