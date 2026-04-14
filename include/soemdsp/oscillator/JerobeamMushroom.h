#pragma once

#include "soemdsp/modulator/Phasor.h"
#include "RSMET/RAPT/GeometricTransformations.h"
#include "soemdsp/utility/MaxMspHelper.h"

namespace soemdsp::oscillator {

class JerobeamMushroom : public utility::gen {
  public:
    JerobeamMushroom()          = default;
    virtual ~JerobeamMushroom() = default;

    void getSampleFrame(double* outL, double* outR);

    void setFrequency(double v) {
        freq = v;
        updateFrequency();
    }
    void setPhaseOffset(double v) {
        phasor.setPhaseOffset(v * 0.5);
    }
    void setSampleRate(double v) {
        phasor.setSampleRate(v);
        capstemRotator.setSampleRate(v);
        clusterRotator.setSampleRate(v);
    }
    void reset() {
        phasor.reset();
        capstemRotator.reset();
        clusterRotator.reset();
    }

    void setNumMushrooms(int v) {
        nom = std::max(v, 1);
        updateFrequency();
    }

    void setGrow(double v) {
        grow = v;
    }
    void setDensity(double v) {
        dens = v;
    }
    void setCapRotation(double v) {
        capstemRotator.setPhaseOffset(v);
    }
    void setStemRotationSpd(double v) {
        capstemRotator.setFrequency(v);
    }
    void setHead(double v) {
        head = v;
    }
    void setSpread(double v) {
        spread = v * 4.0;
    }
    void setWobble(double v) {
        wobble = v;
    }
    void setClusterRotation(double v) {
        clusterRotator.setPhaseOffset(v);
    }
    void setClusterRotationSpd(double v) {
        clusterRotator.setFrequency(v);
    }
    void setSharp(double v) {
        sharp = v * .5 + .5;
    }
    void setWidth(double v) {
        width = v;
    }
    void setStem(double v) {
        stem = v;
    }
    void setApart(double v) {
        apart = v;
    }
    void setCapStemTransition(double v) {
        trans = v;
    }

    double getUnipolarRampPhasorValue() {
        return phas;
    }
    double getUnipolarTrianglePhasorValue() {
        return ph;
    }
    double getPhasorIncrementAmount() {
        return phasor.getIncrementAmount();
    }

    /* Core Objects */
    Phasor phasor;
    Phasor capstemRotator;
    Phasor clusterRotator;
    RAPT::rsRotationXYZ<double> rotator;

  protected:
    void updateFrequency() {
        if (nom < 0)
            phasor.setFrequency(freq / toDouble(nom) * 0.5);
        else
            phasor.setFrequency(freq * 0.5);
    }

    /* Parameters */
    double freq{ 440.0 };

    double grow{ 1.0 };       // Grow 0 to 1
    double dens{ 3.0 };       // Desity 0 to 100
    double head{ 2.0 / 3.0 }; // Head 0 to 1
    double trans{ 0.1 };      // Transisition 0 to 1

    double spread{ 1.0 / 2.0 };  // Spread 0 to 1
    double wobble{ 1.0 / 16.0 }; // Wobble 0 to 1
    double sharp{ 0.0 };         // Sharp -1 to 1

    int nom{ 1 }; // Number of Mushrooms -5 to 5

    double width{ 1.0 }; // Width 0 to 2
    double stem{};     // Stem 0 to 1
    double apart{};    // Apart 0 to 1

    /* Internal States */
    double sampleRate{ 44100.0 };
    double phase_inc{};
    double phas{}; // ramp phasor
    double ph{};   // triangle phasor
};
} // namespace soemdsp::oscillator
