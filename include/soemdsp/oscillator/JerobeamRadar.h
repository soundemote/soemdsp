#pragma once

#include "soemdsp/utility/MaxMSPHelper.h"
#include "soemdsp/modulator/Phasor.h"

#include <algorithm>

namespace soemdsp::oscillator {

class JerobeamRadar : public utility::gen {
  public:
    JerobeamRadar()          = default;
    virtual ~JerobeamRadar() = default;

    void getSampleFrame(double& outL, double& outR);

    void reset() override {
        phasor.reset();
        rotator.reset();
    }

    void setFrequency(double v) {
        phasor.setFrequency(v);
    }
    void setPhaseOffset(double v) {
        phasor.setPhaseOffset(v);
    }
    void setSampleRate(double v) {
        phasor.setSampleRate(v);
        rotator.setSampleRate(v);
    }

    void setDensity(double v) {
        density = v;
        update_dens();
    }
    void setSharp(double v) {
        tri1 = v * .5 + .5;
    }
    void setFade(double v) {
        pow1 = v;
    }
    void setRotation(double v) {
        rotator.setPhaseOffset(v);
    }

    void setDirection(double v) {
        tri2 = v;
    }
    void setShade(double v) {
        pow2 = std::clamp(v, -80.0, 80.0);
    }
    void setLap(double v) {
        lap = std::max(1.e-6, v + 1);
    }

    void setRingCut(bool v) {
        ringcut = v;
        update_dens();
    }
    void setPow1Up(bool v) {
        pow1_up = v;
    }
    void setPow1Down(bool v) {
        pow1_down = v;
    }
    void setPow2Bend(bool v) {
        pow2_bend = v;
    }
    void setPhaseInv(bool v) {
        phase_inv = v;
    }
    void setTunnelInv(bool v) {
        tunnel_inv = v;
    }
    void setSpiralReturn(bool v) {
        spiral_return = v;
    }

    void setLength(double v) {
        length = v;
    }
    void setRatio(double v) {
        ratio  = v;
        ration = ratio + .1;
    }
    void setFrontRing(double v) {
        frontring = v;
    }
    void setZoom(double v) {
        size           = v;
        double x       = 1.0 - v;
        y_fix_for_zoom = x + (x - pow(x, 6.0));
    }

    void setZDepth(double v) {
        z_depth = v;
    }
    void setInner(double v) {
        z_end = v;
    }

    void setX(double v) {
        x = -v;
        update_x_y();
    }
    void setY(double v) {
        y = v;
        update_x_y();
    }

    double getUnipolarTrianglePhasorValue() {
        return triphase;
    }

    /* Core Objects */
    Phasor phasor;
    Phasor rotator;

  protected:
    /* Core Functions */
    void render(double in_phas, double* out_x, double* out_y, double* out_z);
    void update_dens() {
        dens = (ringcut ? floor(density) : density) + 1.e-6;
        dens = std::min(dens, 1.e+6);
    }
    void update_x_y();

    /* Parameters */
    double frequency{};

    double density{ 1.0 }; // Density 0 to 100
    double tri1{ 0.5 };  // Sharp 0 to 1
    double pow1{};       // Fade .1 to 10
    double rot{ 0.5 };   // Rotation -1 to +1

    double tri2{}; // Direction 0 to 1
    double pow2{}; // Shade .1 to 10
    double lap{};  // Lap -1 to +1

    bool ringcut{};       // Ring Cut
    bool pow1_up{};       // Up
    bool pow1_down{};     // Down
    bool pow2_bend{};     // Bend
    bool phase_inv{};     // -Phase or PhaseInv1
    bool tunnel_inv{};    // unlabeled or PhaseInv2
    bool spiral_return{}; // Return

    double length{ 1.0 }; // Length .0001 to 1
    double ratio{};       // Ratio 0 to 1
    double frontring{};   // Front Ring 0 to 1
    double size{};        // Zoom 0 to 1
    double y_fix_for_zoom{ 1.0 };

    double x{};       // X -1 to +1
    double y{};       // Y -1 to +1
    double z_depth{}; // ZDepth 0 to 1
    double z_end{};   // Inner 0 to 1

    /* Internal States */
    double dens{ 1.0 };
    double ration{ 0.1 };
    double triphase{};

    /* `update x y` variables */
    double x_{}, y_{}, ph{}, r{};
};
} // namespace soemdsp::oscillator
