#pragma once

#include "soemdsp/utility/MaxMspHelper.h"

namespace soemdsp::oscillator {

class JerobeamWirdoSpiral : public utility::gen {
  public:
    void getSampleFrame(double* outL, double* outR);

    void setFrequency(double v) {
        freq = v;
    }
    void setSharp(double v) {
        tri = v;
    }
    void setCross(double v) {
        cross_spread = v;
    }
    void setDensity(double v) {
        dens = v * kTAU;
    }
    void setLength(double v) {
        length = v;
    }
    void setRotate(double v) {
        z_rot = v;
    }
    void setSplash(double v) {
        splash_depth = v;
    }
    void setSplashDensity(double v) {
        splash_dens = v;
    }
    void setCut(double v) {
        cut = floor(v + .5);
    }
    void setScrap(double v) {
        scrap = std::clamp(v, .0001, 1.0);
    }
    void setRingCut(double v) {
        splash_cut = v;
    }
    void setSplashSpeed(double v) {
        splash_speed = v;
    }
    void setSyncCut(double v) {
        sync_cut = v;
    }

  protected:
    /* Parameters */
    double freq{ 440.0 };
    double tri{};
    double cross_spread{};
    double dens{ 5 };
    double length{ 1 };
    double z_rot{};
    double splash_depth{};
    double splash_dens{};
    double cut{ 1000 };
    double scrap{ 1 };
    double splash_cut{ 10 };
    double splash_speed{};
    double sync_cut{};

    /* Internal States */
    double sampleRate;
    double phase_inc{};
    double phas{};
};
} // namespace soemdsp::oscillator
