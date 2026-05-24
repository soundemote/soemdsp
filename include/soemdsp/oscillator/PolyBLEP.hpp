#pragma once

#include <magic_enum/magic_enum.hpp>

#include <soemdsp/Wire.hpp>
#include <soemdsp/meta.hpp>
#include <soemdsp/sehelper.hpp>
#include <soemdsp/semath.hpp>

namespace soemdsp::oscillator {



struct PolyBLEP {
    PolyBLEP() = default;

    //enum class Controls {

    //};

//constexpr meta::ModuleMeta mySynthModule{
//
//    .name = "Classic Oscillator",
//    .desc = "A simple selectable-waveform synth oscillator.",
//    .wires = {{
//        { 
//            .name = "Power", 
//            .desc = "Main engine state", 
//            .unit = "state", 
//            .type = meta::WireType::onoff, 
//            .usefuldef = 1.0, .usefulmin = 0.0, .usefulmax = 1.0 
//        },
//        { 
//            .name = "Bypass", 
//            .desc = "Route audio around DSP processing", 
//            .unit = "state", 
//            .type = meta::WireType::bypass, 
//            .usefuldef = 0.0, .usefulmin = 0.0, .usefulmax = 1.0 
//        },
//        { 
//            .name = "Oversampling", 
//            .desc = "Linear multi-sampling steps", 
//            .unit = "x", 
//            .type = meta::WireType::unipolar_integer, 
//            .usefuldef = 2.0, .usefulmin = 1.0, .usefulmax = 8.0 
//        },
//        { 
//            .name = "Fine Tune", 
//            .desc = "Pitch micro adjustments", 
//            .unit = "semi", 
//            .type = meta::WireType::bipolar_decimal_normalized, 
//            .usefuldef = 0.0, .usefulmin = -1.0, .usefulmax = 1.0 
//        }
//    }}
//};

    enum class Shape {
        Sin,
        Cos,
        Square,
        Pulse,
        PulseCenter,
        Saw,
        Ramp,
        SquareSaw,
        Tri,
        Trisaw,
        Trisquare,
        Tripulse,
        RectifiedSinHalf,
        RectifiedSin,
        Trapezoid
    };

    // io
    double out_{};

    // parameters
    Wire waveform_;
    Wire morph_{ 0.5 };
    Wire phase_;
    Wire phaseOffset_;

    // coefficients
    Wire increment_;

    //constexpr meta::WireMeta wireMeta[] = {
    //    {    "waveform",            "Waveform",   "", 0.0, 0.0,    14.0 }, // 0-3 waveform selection
    //    {       "morph",               "Morph",   "", 0.5, 0.0,    1.0 },
    //    {       "phase",               "Phase",   "", 0.0, 0.0,    1.0 },
    //    { "phaseOffset",        "Phase Offset",   "", 0.0, 0.0,    1.0 },
    //    {   "increment", "Frequency Increment",  "", 0.0, 0.0, 1000.0 }  // exposed coefficient
    //};



    static std::string getWaveshapeString(int shape) {
        auto optional = magic_enum::enum_cast<Shape>(shape);

        if (!optional) {
            return "";
        }

        return std::string(magic_enum::enum_name(optional.value()));
    }

    double run() {
        const double p = math::wrap(phase_);
        const double i = increment_;
        const double m = morph_;

        const Shape enumShape = convert::sizet_to_enum<Shape>(waveform_);

        out_ = get(enumShape, i, p, m);

        phase_ = math::wrap(phase_ + i);

        return out_;
    }

    // clamps increment to min and max, recommended values are default: 0.0 to 0.5
    double runClamped(double minInc = 0.0, double maxInc = 0.5) {
        const double p = math::wrap(phase_);
        const double i = std::clamp(*increment_.r, minInc, maxInc);
        const double m = morph_;

        const auto shapeNameOptional = magic_enum::enum_cast<Shape>(waveform_).value_or(Shape::Sin);

        out_ = get(shapeNameOptional, i, p, m);

        phase_ = math::wrap(phase_ + i);

        return out_;
    }

    void reset() {
        phase_ = 0;
    }

    void slaveIncrement(PolyBLEP& master) {
        increment_.pointTo(master.increment_);
    }
    void slaveMorph(PolyBLEP& master) {
        morph_.pointTo(master.morph_);
    }

    static double get(Shape waveform, double incrementAbs, double phase, double morph) {
        switch (waveform) {
        case Shape::Sin:
            return sin(phase * constant::kTAU);
        case Shape::Cos:
            return cos(phase * constant::kTAU);
        case Shape::Square:
            return square(incrementAbs, phase);
        case Shape::Pulse:
            return pulse(incrementAbs, phase, morph);
        case Shape::PulseCenter:
            return pulseCenter(incrementAbs, phase, morph);
        case Shape::Saw:
            return saw(incrementAbs, phase);
        case Shape::SquareSaw:
            return saw_square(incrementAbs, phase, morph);
        case Shape::Ramp:
            return ramp(incrementAbs, phase);
        case Shape::Tri:
            return tri(incrementAbs, phase);
        case Shape::Trisaw:
            return trisaw(incrementAbs, phase, morph);
        case Shape::Trisquare:
            return triSquare(incrementAbs, phase, morph);
        case Shape::Tripulse:
            return triPulse(incrementAbs, phase, morph);
        case Shape::RectifiedSinHalf:
            return rectSinHalf(incrementAbs, phase);
        case Shape::RectifiedSin:
            return rectSinFull(incrementAbs, phase);
        case Shape::Trapezoid:
            return trapezoid(incrementAbs, phase);
        default: debug::FAIL();
        }
        return {};
    }

  private:
    static double blep(double t, double dt) {
        if (t < dt) {
            return -pow(t / dt - 1, 2);
        } else if (t > 1 - dt) {
            return pow((t - 1) / dt + 1, 2);
        } else {
            return 0;
        }
    }

    static double blamp(double t, double dt) {
        if (t < dt) {
            t = t / dt - 1;
            return -constant::k1z3 * pow(t, 3);
        } else if (t > 1 - dt) {
            t = (t - 1) / dt + 1;
            return constant::k1z3 * pow(t, 3);
        } else {
            return 0;
        }
    }

    static double tri(double incrementAbs, double t) {
        double t1 = t + 0.25;
        t1        = math::wrap(t1);
        double t2 = t + 0.75;
        t2        = math::wrap(t2);

        double y = t * 4;

        if (y >= 3) {
            y -= 4;
        } else if (y > 1) {
            y = 2 - y;
        }

        return y + 4 * incrementAbs * (blamp(t1, incrementAbs) - blamp(t2, incrementAbs));
    }

    static double ramp(double incrementAbs, double t) {
        double t1 = t + 0.5;
        t1        = math::wrap(t1);

        double y = t1 * 2 - 1;

        y -= blep(t1, incrementAbs);

        return y;
    }

    static double saw(double incrementAbs, double t) {
        double t1 = t;
        t1        = math::wrap(t1);

        double y = 1 - 2 * t1;

        y += blep(t1, incrementAbs);

        return y;
    }

    static double square(double incrementAbs, double t) {
        double t1 = t + 0.5;
        t1        = math::wrap(t1);

        double y = t < 0.5 ? 1 : -1;

        y += blep(t, incrementAbs) - blep(t1, incrementAbs);

        return y;
    }

    static double saw_square(double incrementAbs, double t, double morph) {
        t = math::wrap(t);

        double y = 1.0 - 2.0 * t; // base saw

        if (t < 0.5) {
            y += morph; // first half offset
        } else {
            y -= morph; // second half offset
        }

        // BLEP corrections (scaled exactly right for the variable middle jump)
        y += blep(t, incrementAbs);

        double t_mid = math::wrap(t - 0.5);
        y += (-morph) * blep(t_mid, incrementAbs); // variable middle jump correction

        return y;
    }

    static double rectSinHalf(double incrementAbs, double t) {
        double t1 = t + 0.5;
        t1        = math::wrap(t1);

        double y = (t < 0.5 ? 2 * sin(constant::kTAU * t) - constant::k2zPI : -constant::k2zPI);

        y += constant::kTAU * incrementAbs * (blamp(t, incrementAbs) + blamp(t1, incrementAbs));

        return y;
    }

    static double rectSinFull(double incrementAbs, double t) {
        double t1 = t + 0.25;
        t1        = math::wrap(t1);

        double y = 2 * sin(constant::kPI * t1) - constant::k4zPI;

        y += constant::kTAU * incrementAbs * blamp(t1, incrementAbs);

        return y;
    }

    static double trisaw(double incrementAbs, double t, double morph) {
        double pw = std::clamp(morph, 0.0001, 0.9999);

        double t1 = t + 0.5 * pw;
        t1        = math::wrap(t1);
        double t2 = t + 1 - 0.5 * pw;
        t2        = math::wrap(t2);

        double y = t * 2;

        if (y >= 2 - pw) {
            y = (y - 2) / pw;
        } else if (y >= pw) {
            y = 1 - (y - pw) / (1 - pw);
        } else {
            y /= pw;
        }

        y += incrementAbs / (pw - pw * pw) * (blamp(t1, incrementAbs) - blamp(t2, incrementAbs));

        return y;
    }

    static double triPulse(double incrementAbs, double t, double morph) {
        double t1 = t + 0.75 + 0.5 * morph;
        t1        = math::wrap(t1);

        double y = [&]() {
            if (t1 >= morph) {
                return -morph;
            } else {
                double a = 4 * t1;
                return (a >= 2 * morph ? 4 - a / morph - morph : a / morph - morph);
            }
        }();

        if (t1 >= morph) {
            y = -morph;
        } else {
            y = 4 * t1;
            y = (y >= 2 * morph ? 4 - y / morph - morph : y / morph - morph);
        }

        if (morph > 0) {
            double t2 = t1 + 1 - 0.5 * morph;
            t2        = math::wrap(t2);
            double t3 = t1 + 1 - morph;
            t3        = math::wrap(t3);
            y += 2 * incrementAbs / morph * (blamp(t1, incrementAbs) - 2 * blamp(t2, incrementAbs) + blamp(t3, incrementAbs));
        }

        return y;
    }

    static double trapezoid(double incrementAbs, double t) {
        double y = 4 * t;

        if (y >= 3) {
            y -= 4;
        } else if (y > 1) {
            y = 2 - y;
        }

        y = std::max(-1.0, std::min(1.0, 2.0 * y));

        double t1 = t + 0.125;
        t1        = math::wrap(t1);
        double t2 = t1 + 0.5;
        t2        = math::wrap(t2);

        // Triangle #1
        y += 4 * incrementAbs * (blamp(t1, incrementAbs) - blamp(t2, incrementAbs));

        t1 = t + 0.375;
        t1 = math::wrap(t1);
        t2 = t1 + 0.5;
        t2 = math::wrap(t2);

        // Triangle #2
        y += 4 * incrementAbs * (blamp(t1, incrementAbs) - blamp(t2, incrementAbs));

        return y;
    }

    static double triSquare(double incrementAbs, double t, double morph) {
        double pw    = std::min(0.9999, morph);
        double scale = 1 / (1 - pw);

        double y = 4 * t;
        if (y >= 3) {
            y -= 4;
        } else if (y > 1) {
            y = 2 - y;
        }
        y = std::max(-1.0, std::min(1.0, scale * y));

        double t1 = t + 0.25 - 0.25 * pw;
        t1        = math::wrap(t1);
        double t2 = t1 + 0.5;
        t2        = math::wrap(t2);

        // Triangle #1
        y += scale * 2 * incrementAbs * (blamp(t1, incrementAbs) - blamp(t2, incrementAbs));

        t1 = t + 0.25 + 0.25 * pw;
        t1 = math::wrap(t1);
        t2 = t1 + 0.5;
        t2 = math::wrap(t2);

        // Triangle #2
        return y + scale * 2 * incrementAbs * (blamp(t1, incrementAbs) - blamp(t2, incrementAbs));
    }

    static double pulseCenter(double incrementAbs, double t, double morph) {
        double t1 = t + 0.875 + 0.25 * (morph - 0.5);
        t1        = math::wrap(t1);

        double t2 = t + 0.375 + 0.25 * (morph - 0.5);
        t2        = math::wrap(t2);

        // Square #1
        double y = t1 < 0.5 ? 1 : -1;

        y += blep(t1, incrementAbs) - blep(t2, incrementAbs);

        t1 += 0.5 * (1 - morph);
        t1 = math::wrap(t1);

        t2 += 0.5 * (1 - morph);
        t2 = math::wrap(t2);

        // Square #2
        y += t1 < 0.5 ? 1 : -1;

        y += blep(t1, incrementAbs) - blep(t2, incrementAbs);

        return 0.5 * y;
    }

    static double pulse(double incrementAbs, double t, double morph) {
        double t1 = t + 1 - morph;
        t1        = math::wrap(t1);

        double y = -2 * morph;

        if (t < morph) {
            y += 2;
        }

        y += blep(t, incrementAbs) - blep(t1, incrementAbs);

        return y;
    }

    // double runWithAntialiasing() {
    //     double p = wrap(phase);
    //     double i = increment;
    //     double m = morph;

    //     auto shapeNameOptional = magic_enum::enum_cast<Shape>(waveform).value_or(Shape::Sin);

    //     output = get(shapeNameOptional, i, p, m);

    //     phase.vw_ = phase + i;

    //     return finalPhase = wrap(
    //              phase +
    //              antialiasingAmplitude * random.getRandomGaussian(antialiasingFactor));
    // }
};
} // namespace soemdsp::oscillator
