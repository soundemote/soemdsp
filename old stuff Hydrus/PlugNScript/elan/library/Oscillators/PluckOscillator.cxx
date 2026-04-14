#include "../Oscillators/MusicalSaw.cxx"
#include "../Filters/Lowpass.cxx"
#include "../Filters/LinearSmoother.cxx"

class PluckOscillator
{
    MusicalSaw osc;
    LinearSmoother smoother;
    Lowpass lpf;

    double currentAmplitude = 1;
    double amplitudeMultiplier = 1;

    double currentValue = 0;
    double delayCurve = 0;
    double minDecay = 0, maxDecay = 1;

    void reset()
    {
        lpf.reset();
        osc.reset();
        smoother.reset();
        currentAmplitude = 1;
    }

    void setSampleRate(double v)
    {
        osc.setSampleRate(v);
        smoother.setSampleRate(v);
        lpf.setSampleRate(v);
    }

    void triggerAttack()
    {
        osc.reset();
        smoother.reset();
        lpf.reset();

        currentAmplitude = 1;
    }

    void setFrequency(double v)
    {
        osc.setFrequency(v);
    }

    void setAmplitudeMultiplier(double v)
    {
        amplitudeMultiplier = v;
    }

    double getSample()
    {
        if (timer.hasTriggered())
            currentAmplitude *= amplitudeMultiplier;//map(rampUp.getPosition(), minDecay, maxDecay);   


        osc.setMorph(currentAmplitude);
        lpf.setFrequency(pitchToFrequency(map0to1(currentAmplitude, -50, 130)));
        currentValue = osc.getSample() * currentAmplitude;

        return currentValue;
    }
}