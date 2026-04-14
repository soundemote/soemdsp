#include "LinearEnvelope.cxx"
#include "LinearDigitalAttackDecay.cxx"
#include "LinearFeedbackEnvelope.cxx"
#include "../math.cxx"

class DualLinearPluckEnvelope
{
    //LinearDigitalAttackDecay mainEnv;
    LinearFeedbackEnvelope mainEnv;
    LinearDigitalAttackDecay modEnv;

    double decayModStrength = 0;
    double sustainModStrength = 0;

    double decayTime = 0;
    double sustainAmplitude = 0;
    double modSignal = 0;
    double mainSignal = 0;

    double curveAmount = 0;

    void reset()
    {
        mainEnv.reset();
        modEnv.reset();
    }

    void setSampleRate(double v)
    {
        mainEnv.setSampleRate(v);
        modEnv.setSampleRate(v);
    }

    void triggerAttack()
    {
        mainEnv.triggerAttack();
        modEnv.triggerAttack();
    }

    void triggerRelease()
    {
        mainEnv.triggerRelease();
        modEnv.triggerRelease();
    }

    void setAttackTime(double v)
    {
        mainEnv.setAttackTime(v);
    }

    void setDecayTime(double v)
    {
        decayTime = v;
    }

    void setSustainAmplitude(double v)
    {
        //sustainAmplitude = v;
    }

    void setReleaseTime(double v)
    {
        //mainEnv.setReleaseTime(v);
    }

    void setModAttackTime(double v)
    {
        modEnv.setAttackTime(v);
    }

    void setModDecayTime(double v)
    {
        modEnv.setDecayTime(v);
    }

    void setModDelayTime(double v)
    {
        modEnv.setDelayTime(v);
    }

    void setDecayModulationStrength(double v)
    {
        decayModStrength = v;
    }

    void setSustainModulationStrength(double v)
    {
        sustainModStrength = v;
    }

    double getSample()
    {
        
        modSignal = rationalCurve(-modEnv.getSample()+1,curveAmount);

        mainEnv.setDecayTime(decayTime/* + modSignal * decayModStrength*/);

        //mainEnv.setSustainAmplitude(sustainAmplitude + -modSignal * sustainModStrength);

        //return modSignal;
        return mainSignal = mainEnv.getSample();
    }
}