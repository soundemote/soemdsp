#pragma once

#include "soemdsp/modulator/Phasor.h"
#include "soemdsp/utility/MaxMspHelper.h"

namespace soemdsp::oscillator {

class JerobeamTorus : public utility::gen {
  public:
    void setFrequency(double v) {
        phasor.setFrequency(v);
    }
    void getSampleFrame(double* outL, double* outR);
    void setSampleRate(double v);
    void reset();

    // 0 to 50 integer
    void setDensity(double v) {
        density = v;
        updateDensity();
    }
    // true or false
    void setQuantizeDensity(bool v) {
        quantizeDensity = v;
        updateDensity();
    }
    // 0 to 2
    void setSDensity(double v) {
        subdensity = v;
        updateDensity();
    }
    // true or false
    void setQuantizeSubDensity(bool v) {
        quantizeSubDensity = v;
        updateSubDensity();
    }
    // 0 to 1
    void setSharp(double v) {
        sharp = v;
    }
    // 0.1 to 1
    void setSize(double v) {
        div         = 1.0 / v;
        vol_correct = 1 / (1 + v + v * div);
    }

    // 0 to 1
    void setLength(double v) {
        len = v;
    }
    // -1 to +1
    void setBalance(double v) {
        bal = v;
    }
    // Wander 0 to 1
    void setWander(double v) {
        wanderspeed = v;
        updateDensity();
    }

    // -1 to 1
    void setDarkAngle(double v) {
        phasorDarkAngle.setPhaseOffset(v);
    }
    // 0 to 10 integer
    void setDarkIntensity(int v) {
        dank = v * 2 + 1;
    } // odd integers only

    // -1 to +1
    void setRotX(double v) {
        phasorX.setPhaseOffset(v);
    }
    // -1 to +1
    void setRotY(double v) {
        phasorY.setPhaseOffset(v);
    }
    // -1 to 1
    void setRotZ(double v) {
        phasorZ.setPhaseOffset(v);
    }

    // -1 to +1
    void setZAngleX(double v) {
        zaspx = v;
    };
    // -1 to +1
    void setZAngleY(double v) {
        zaspy = v;
    };
    // 0 to 2
    void setZDepth(double v) {
        zdepth     = v;
        zdepth_z_2 = zdepth / 2.0;
    }

    double getUnipolarTrianglePhasorValue() {
        return triphase;
    }
    double getUnipolarTrianglePhasorValue2() {
        return phas;
    }

    Phasor phasor;
    Phasor phasorWander;
    Phasor phasorX, phasorY, phasorZ;
    Phasor phasorDarkAngle;

  protected:
    /* Core Functions */
    void rotate(double in_x, double in_y, double in_z, double rot_x, double rot_y, double rot_z, double* out_x, double* out_y, double* out_z);
    void render(double in_x, double in_y, double in_z, double zaspx, double zaspy, double zdepth, double* out_l, double* out_r);

    /* Parameters */
    double freq{ 440.0 };
    double density{ 1.0 };
    double subdensity{};
    double sharp{ 0.5 };
    double div{ 0.5 };

    double len{};
    double bal{};

    double wanderspeed{};

    double dangle{};
    double dank{};

    double rot_x{};
    double rot_y{};
    double rot_z{};

    double zaspx{};
    double zaspy{};
    double zdepth{};

    bool quantizeDensity{ true };
    bool quantizeSubDensity{ true };

    /* Updates */
    void updateDensity() {
        dense = quantizeDensity ? floor(density) : density;
        phasorWander.setFrequency(wanderspeed / dense);
        pow2_dense = pow(dense, 2);
        updateSubDensity();
    }

    void updateSubDensity() {
        if (quantizeSubDensity)
            sdens = floor(pow2_dense * subdensity) * kTAU;
        else
            sdens = pow2_dense * subdensity * kTAU;
    }

    /* Internal States */
    double sampleRate{ 44100.0 };
    double triphase{};
    double phas{};
    double dense{};
    double sdens{};

    /* Coefficients */
    double vol_correct{};
    double pow2_dense{};
    double zdepth_z_2{};
};
} // namespace soemdsp::oscillator
