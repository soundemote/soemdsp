#pragma once
#include <soemdsp/semath.hpp>
/* use as a master oscillator controller, outputs a 0 to 1 and -1 to +1
controller after calculating increment through samplerate, frequerncy, and
allows for arbitrary phase control
*/
namespace soemdsp::oscillator {
struct Phasor {
    Phasor() = default;

    // returns 0 to 1 value and increments phase
    virtual double getSample() {
        double out = getUnipolarValue();
        increment();
        return out;
    }
    // returns -1 to +1 value and increments phase
    virtual double getSampleBipolar() {
        double out = getBipolarValue();
        increment();
        return out;
    }

    void setSampleRate(double v) {
        secondsPerSample = 1 / v;
        calculateIncrementAmount();
    }
    // -inf to +inf
    void setFrequency(double v) {
        frequency = v;
        calculateIncrementAmount();
    }
    // -1 to +1, sets the time / value offset
    void setPhaseOffset(double v) {
        phaseOffset = v;
    }
    // -1 to +1, sets the current time / value
    void setPhase(double v) {
        phase = v;
    }

    // 0 to 1
    double getUnipolarValue() {
        return math::wrap(phase + phaseOffset);
    }
    // -1 to +1
    double getBipolarValue() {
        return getUnipolarValue() * 2 - 1;
    }
    // 0 to 1
    double getIncrementAmount() {
        return incAmt;
    }
    // -1 to +1
    double getPhaseOffset() {
        return phaseOffset;
    }
    // returns the stored value of 1.0/sampleRate
    double getSecondsPerSample() {
        return secondsPerSample;
    }

    // reset the to phase its phase offset
    void reset() {
        phase = 0;
    }
    // reset the variably back to its phase offset
    void partialReset(double v) {
        phase = phase * (1 - v);
    }
    // increment based on current frequency
    void increment() {
        phase = math::wrap(phase + incAmt);
    }
    // increment by arbitrary amount
    void increment(double v) {
        phase = math::wrap(phase + v);
    }

  protected:
    void calculateIncrementAmount() {
        incAmt    = secondsPerSample * frequency;
        incAmtAbs = abs(incAmt);
    }

    /* Parameters */
    double frequency{ 1.0 }, sampleRate{ 44100.0 }, phaseOffset{};

    /* Internal Values*/
    double secondsPerSample{}, phase{}, incAmt{}, incAmtAbs{};
};

namespace unipolar {
inline double triangle(double phase) {
    return (phase < 0.5) ? (4.0 * phase - 1.0) : (3.0 - 4.0 * phase);
}
inline double trisaw(double phase, double warp) {
    // Precondition: 0 <= t <= 1 and 0 < warp < 1.
    return (phase < warp) ? (phase / warp) : ((1.0 - phase) / (1.0 - warp));
}
} // namespace unipolar

namespace bipolar {
inline double triangle(double phase) {
    return (phase < 0.5) ? (4.0 * phase - 1.0) : (3.0 - 4.0 * phase);
}
inline double trisaw(double phase, double warp) {
    // Precondition: 0 <= t <= 1 and 0 < warp < 1.
    return (phase < warp) ? (2.0 * (phase / warp) - 1.0) : (2.0 * ((1.0 - phase) / (1.0 - warp)) - 1.0);
}
} // namespace bipolar
} // namespace soemdsp::oscillator
