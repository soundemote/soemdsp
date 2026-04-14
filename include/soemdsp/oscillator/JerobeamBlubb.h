#pragma once

#include "soemdsp/modulator/Phasor.h"
#include "soemdsp/semath.hpp"

/*
- go up in overall pitch for a number of bubbles and randomly jump pitch back down
- random delay between
*/

namespace soemdsp::oscillator {

class JerobeamBlubb {
  public:
    void getSampleFrame(double* outL, double* outR);

    void setSampleRate(double v) {
        phasor.setSampleRate(v);
    }
    void setFrequency(double v) {
        phasor.setFrequency(v);
    }
    void reset() {
        phasor.reset();
    }

    void setRotX(double v) {
        rotX = v;
    }
    void setRotY(double v) {
        rotY = v;
    }
    void setRotZ(double v) {
        rotZ = v;
    }
    void setZDepth(double v) {
        zDepth = v;
    }

    enum Shape {
        CIRCLE,
        SQUARE
    };
    void setShape(Shape v) {
        shape = v;
    }

  protected:
    void rotate(double in_x, double in_y, double in_z, double rot_x, double rot_y, double rot_z, double* out_l, double* out_r);

    /* Parameters */
    double frequency{ 440.0 };
    double rotX{}, rotY{}, rotZ{}, zDepth{};

    /* Internal States */
    double phase{ 0.0 };
    double phaseOffset{ 0.0 };

    int shape = Shape::CIRCLE;

    static constexpr double phase45degrees{ degreesToPhase(45.0) };
    static constexpr double phase135degrees{ degreesToPhase(135.0) };

    /* Core Objects */
    Phasor phasor;
};
} // namespace soemdsp::oscillator
