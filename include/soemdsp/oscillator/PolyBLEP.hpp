#pragma once

#include "soemdsp/Wire.hpp"
#include "soemdsp/sehelper.hpp"
#include "soemdsp/semath.hpp"

#include <magic_enum.hpp>

using std::clamp;

namespace soemdsp::oscillator {

class PolyBLEP {
  public:
    enum class Shape {
        Sin,
        Cos,
        Square,
        Pulse,
        PulseCenter,
        Saw,
        Ramp,
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
    Wire<int> waveform_;
    Wire<double> morph_{ 0.5 };
    Wire<double> phase_;
    Wire<double> phaseOffset_;

    // coefficients
    WireMod<double> increment_;

    static std::string getWaveshapeString(int shape) {
        auto optional = magic_enum::enum_cast<Shape>(shape);

        if (!optional) {
            return "";
        }

        return std::string(magic_enum::enum_name(optional.value()));
    }

    double run() {
        const double p = wrap(phase_);
        const double i = increment_;
        const double m = morph_;

        const Shape enumShape = Enum::get(waveform_, Shape::Sin);

        out_ = get(enumShape, i, p, m);

        phase_ = wrap(phase_ + i);

        return out_;
    }

    // clamps increment to min and max, recommended values are default: 0.0 to 0.5
    double runClamped(double minInc = 0.0, double maxInc = 0.5) {
        const double p = wrap(phase_);
        const double i = clamp(double(increment_), minInc, maxInc);
        const double m = morph_;

        const auto shapeNameOptional = magic_enum::enum_cast<Shape>(waveform_).value_or(Shape::Sin);

        out_ = get(shapeNameOptional, i, p, m);

        phase_ = wrap(phase_ + i);

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
            return sin(phase * kTAU);
        case Shape::Cos:
            return cos(phase * kTAU);
        case Shape::Square:
            return square(incrementAbs, phase);
        case Shape::Pulse:
            return pulse(incrementAbs, phase, morph);
        case Shape::PulseCenter:
            return pulseCenter(incrementAbs, phase, morph);
        case Shape::Saw:
            return saw(incrementAbs, phase);
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
            SE_SWITCH_STATEMENT_FAILURE
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
            return -k1z3 * pow(t, 3);
        } else if (t > 1 - dt) {
            t = (t - 1) / dt + 1;
            return k1z3 * pow(t, 3);
        } else {
            return 0;
        }
    }

    static double tri(double incrementAbs, double t) {
        double t1 = t + 0.25;
        t1        = wrap(t1);
        double t2 = t + 0.75;
        t2        = wrap(t2);

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
        t1        = wrap(t1);

        double y = t1 * 2 - 1;

        y -= blep(t1, incrementAbs);

        return y;
    }

    static double saw(double incrementAbs, double t) {
        double t1 = t;
        t1        = wrap(t1);

        double y = 1 - 2 * t1;

        y += blep(t1, incrementAbs);

        return y;
    }

    static double square(double incrementAbs, double t) {
        double t1 = t + 0.5;
        t1        = wrap(t1);

        double y = t < 0.5 ? 1 : -1;

        y += blep(t, incrementAbs) - blep(t1, incrementAbs);

        return y;
    }

    static double rectSinHalf(double incrementAbs, double t) {
        double t1 = t + 0.5;
        t1        = wrap(t1);

        double y = (t < 0.5 ? 2 * sin(kTAU * t) - k2zPI : -k2zPI);

        y += kTAU * incrementAbs * (blamp(t, incrementAbs) + blamp(t1, incrementAbs));

        return y;
    }

    static double rectSinFull(double incrementAbs, double t) {
        double t1 = t + 0.25;
        t1        = wrap(t1);

        double y = 2 * sin(kPI * t1) - k4zPI;

        y += kTAU * incrementAbs * blamp(t1, incrementAbs);

        return y;
    }

    static double trisaw(double incrementAbs, double t, double morph) {
        double pw = clamp(morph, 0.0001, 0.9999);

        double t1 = t + 0.5 * pw;
        t1        = wrap(t1);
        double t2 = t + 1 - 0.5 * pw;
        t2        = wrap(t2);

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
        t1        = wrap(t1);

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
            t2        = wrap(t2);
            double t3 = t1 + 1 - morph;
            t3        = wrap(t3);
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
        t1        = wrap(t1);
        double t2 = t1 + 0.5;
        t2        = wrap(t2);

        // Triangle #1
        y += 4 * incrementAbs * (blamp(t1, incrementAbs) - blamp(t2, incrementAbs));

        t1 = t + 0.375;
        t1 = wrap(t1);
        t2 = t1 + 0.5;
        t2 = wrap(t2);

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
        t1        = wrap(t1);
        double t2 = t1 + 0.5;
        t2        = wrap(t2);

        // Triangle #1
        y += scale * 2 * incrementAbs * (blamp(t1, incrementAbs) - blamp(t2, incrementAbs));

        t1 = t + 0.25 + 0.25 * pw;
        t1 = wrap(t1);
        t2 = t1 + 0.5;
        t2 = wrap(t2);

        // Triangle #2
        return y + scale * 2 * incrementAbs * (blamp(t1, incrementAbs) - blamp(t2, incrementAbs));
    }

    static double pulseCenter(double incrementAbs, double t, double morph) {
        double t1 = t + 0.875 + 0.25 * (morph - 0.5);
        t1        = wrap(t1);

        double t2 = t + 0.375 + 0.25 * (morph - 0.5);
        t2        = wrap(t2);

        // Square #1
        double y = t1 < 0.5 ? 1 : -1;

        y += blep(t1, incrementAbs) - blep(t2, incrementAbs);

        t1 += 0.5 * (1 - morph);
        t1 = wrap(t1);

        t2 += 0.5 * (1 - morph);
        t2 = wrap(t2);

        // Square #2
        y += t1 < 0.5 ? 1 : -1;

        y += blep(t1, incrementAbs) - blep(t2, incrementAbs);

        return 0.5 * y;
    }

    static double pulse(double incrementAbs, double t, double morph) {
        double t1 = t + 1 - morph;
        t1        = wrap(t1);

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
