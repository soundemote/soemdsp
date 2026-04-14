#pragma once

#include "soemdsp/utility/MaxMSPHelper.h"

namespace soemdsp::oscillator {
class JerobeamKeplerBouwkamp : public utility::gen {
  public:
    JerobeamKeplerBouwkamp()  = default;
    ~JerobeamKeplerBouwkamp() = default;

    void setFrequency(double v) {
        freq = v;
        recalculatePhaseInc();
    }
    void getSampleFrame(double* outL, double* outR);
    void setSampleRate(double v) {
        sampleRate = v;
        recalculatePhaseInc();
    }
    void reset() {
        osc_phase = 0.0;
    }

    void setStart(double v) {
        first_polygon = v;
    }
    void setLength(double v) {
        n = v;
    }
    void setCircles(double v) {
        circleblend = v;
    }
    void setZoom(double v) {
        zoom = v;
    }
    void setRotation(double v) {
        rotation = v;
    }
    void setTri(double v) {
        tri = v;
    }

  protected:
    void recalculatePhaseInc() {
        phase_inc = kTAU * freq / sampleRate;
    }

    /* Parameters */
    double freq{ 440.0 };

    int first_polygon{ 3 }; // Start 3 to 20
    int n{ 1 };             // Length 1 to 20
    double circleblend{};   // Circles 0 to 1
    double zoom{};          // Zoom 0 to 1
    double rotation{};      // Rotation -1 to +1
    double tri{};           // Tri 0 to 1;

    /* Internal States */
    double sampleRate{ 44100.0 };
    double phase_inc{};
    double osc_phase{};
};
} // namespace soemdsp::oscillator
