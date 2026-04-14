#pragma once

namespace soemdsp::oscillator {

class Phasor {
  public:
    Phasor()          = default;
    virtual ~Phasor() = default;

    // returns 0 to 1 value and increments phase
    virtual double getSample();
    // returns -1 to +1 value and increments phase
    virtual double getSampleBipolar();

    void setSampleRate(double v);
    // -inf to +inf
    void setFrequency(double v);
    // -1 to +1, sets the time / value offset
    void setPhaseOffset(double v);
    // -1 to +1, sets the current time / value
    void setPhase(double v);

    // 0 to 1
    double getUnipolarValue();
    // -1 to +1
    double getBipolarValue();
    // 0 to 1
    double getIncrementAmount();
    // -1 to +1
    double getPhaseOffset();
    // returns the stored value of 1.0/sampleRate
    double getSecondsPerSample();

    // reset the to phase its phase offset
    void reset();
    // reset the variably back to its phase offset
    void partialReset(double v);
    // increment based on current frequency
    void increment();
    // increment by arbitrary amount
    void increment(double v);

  protected:
    void calculateIncrementAmount();

    /* Parameters */
    double frequency{ 1.0 }, sampleRate{ 44100.0 }, phaseOffset{};

    /* Internal Values*/
    double secondsPerSample{}, phase{}, incAmt{}, incAmtAbs{};
};
} // namespace soemdsp::oscillator
