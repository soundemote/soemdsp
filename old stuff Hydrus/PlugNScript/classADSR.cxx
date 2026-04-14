class ADSR_Analog
{
    ADSR_Analog() {}

    double Attack = 0.0;
    double Decay = 0.0;
    double Sustain = 1.0;
    double Release = 0.0;

    int State = 0;
    double Target = 0.0;
    double currrentPosition = 0.0;

    double getAmplitude()
    {
        switch (State)
        {
            case 0:
                currrentPosition = currrentPosition * (1.0 - Release) + Target * Release;
                break;
            case 1:
                currrentPosition = currrentPosition * (1.0 - Attack) + Target * Attack;
                if (currrentPosition + 0.0001 >= 1.0) State = 2;
                break;
            case 2:
                currrentPosition = currrentPosition * (1.0 - Decay) + Sustain * Decay;
                break;
        }
        return currrentPosition;
    }

    void startAttack()
    {
        Target = 1.0;
        State = 1;        
    }

    void startRelease()
    {
        Target = 0.0;
        State = 0;
    }

    protected double timeCurve(double x)
    {
        return pow(1.0-x,20);
    }

    void setAttack(double x)
    {
        Attack = timeCurve(x);
    }

    void setDecay(double x)
    {
        Decay = timeCurve(x);
    }

    void setSustain(double x)
    {
        Sustain = x;
    }

    void setRelease(double x)
    {
        Release = timeCurve(x);
    }

    void setResonance(double x) {}
}


class ADSR_Analog_Spike
{
    ADSR_Analog_Spike() {}

    
    double Attack = 0.0;
    double Decay = 0.0;
    double Sustain = 1.0;
    double Release = 0.0;

    int State = 0;
    double Target = 0.0;
    double currrentPosition = 0.0;

    double getAmplitude()
    {
        const double PI = 3.14159265358979;
        switch (State)
        {
            case 0:
                currrentPosition = currrentPosition * (1.0 - Release) + Target * Release;
                break;
            case 1:
                currrentPosition = currrentPosition * (1.0 - Attack) + Target * Attack;
                if (currrentPosition + 0.0001 >= 1.0) State = 2;
                break;
            case 2:
                currrentPosition = currrentPosition * (1.0 - Decay) + Sustain * Decay;
                break;
        }
        return tanh((currrentPosition*2.0-1.0)*PI*2-PI)*.5+.5;
    }

    void startAttack()
    {
        Target = 1.0;
        State = 1;        
    }

    void startRelease()
    {
        Target = 0.0;
        State = 0;
    }

    protected double timeCurve(double x)
    {
        return pow(1.0-x,20);
    }

    void setAttack(double x)
    {
        Attack = timeCurve(x);
    }

    void setDecay(double x)
    {
        Decay = timeCurve(x);
    }

    void setSustain(double x)
    {
        Sustain = x;
    }

    void setRelease(double x)
    {
        Release = timeCurve(x);
    }

    void setResonance(double x) {}
}


class ADSR_Analog_Resonant
{
    ADSR_Analog_Resonant() {}

    double Attack = 0.0;
    double Decay = 0.0;
    double Sustain = 1.0;
    double Release = 0.0;
    double Resonance = 0.5;

    int State = 0;
    double Target = 0.0;
    double currentPosition = 0.0;
    double currentMagnitude = 0.0;

    double getAmplitude()
    {
        switch (State)
        {
            case 0:
                filterChange(Release, 0.0);
                break;
            case 1:
                filterChange(Attack, 1.0);
                if (currentPosition + 0.0001 >= 1.0) State = 2;
                break;
            case 2:
                filterChange(Decay, Sustain);
                break;
        }
        return currentPosition;
    }

    void filterChange(double time, double target)
    {
        currentMagnitude += ((target - currentPosition) * time);
        currentPosition = (currentMagnitude + currentPosition) * Resonance;
    }

    void startAttack()
    {
        State = 1;        
    }

    void startRelease()
    {
        State = 0;
    }

    protected double timeCurve(double x)
    {
        return pow(1.0-x*0.8,20);
    }

    void setAttack(double x)
    {
        Attack = timeCurve(x);
    }

    void setDecay(double x)
    {
        Decay = timeCurve(x);
    }

    void setSustain(double x)
    {
        Sustain = x;
    }

    void setRelease(double x)
    {
        Release = timeCurve(x);
    }

    void setResonance(double x)
    {
        Resonance = x * 0.95;
    }
}


class ADSR_Formula_One
{
    ADSR_Formula_One() {}

    double Attack = 0.0;
    double Decay = 0.0;
    double Sustain = 1.0;
    double Release = 0.0;

    double AttackTension = 1.0;
    double DecayTension = 1.0;

    int State = 0;
    double Target = 0.0;
    double currentPosition = 0.0;
    double releaseStartPosition = 0.0;

    double min(double x, double y)
    {
        if (x < y) return x;
        return y;
    }

    double max(double x, double y)
    {
        if (x > y) return x;
        return y;
    }    

    double AttackDecay()
    {
        double att = pow(currentPosition*(1/Attack),AttackTension);
        double dec = pow(Attack/currentPosition,DecayTension)*Sustain+(1-Sustain);
        return smoothMin(att,dec,0.2);
    }

    double ReleaseCurve()
    {   
        double y = (currentPosition / (sampleRate * Release));
        double x = y/(1+abs(y));
        return (1.0-(x*x*(3.0-2.0*x))) * releaseStartPosition;
    }

    double smoothMin(double x, double y, double smoothing)
    {
        double r = max(smoothing-abs(x-y),0.0) / smoothing;
        return min(x,y)-r*r*smoothing*(1.0/4.0);
    }

    double getAmplitude()
    {
        double ret = 0.0;
        switch (State)
        {
            case 0:
                ret = ReleaseCurve();
                break;
            case 1:
                ret = AttackDecay();
                break;
        }
        currentPosition++;
        return ret;
    }

    void startAttack()
    {
        currentPosition = 0.0;
        State = 1;
    }

    void startRelease()
    {
        releaseStartPosition = AttackDecay();
        currentPosition = 0.0;
        State = 0;
    }

    void setAttack(double x)
    {
        Attack = 4.0 * pow(x+0.1,4) * sampleRate;
    }

    void setDecay(double x)
    {
        DecayTension = (1-pow(x,.1)) * 16;
    }

    void setSustain(double x)
    {
        Sustain = 1.-x;
    }

    void setRelease(double x)
    {
        Release = x;
    }

    void setResonance(double x)
    { 
        AttackTension = (x+.01) * 4.0;
    }
}


class AD_Formula_One
{
    AD_Formula_One() {}

    double Attack = 0.0;
    double Decay = 0.0;
    
    double AttackTension = 1.0;
    double DecayTension = 1.0;

    int State = 0;
    double Target = 0.0;
    double currentPosition = 0.0;

    double min(double x, double y)
    {
        if (x < y) return x;
        return y;
    }

    double max(double x, double y)
    {
        if (x > y) return x;
        return y;
    }    

    double AttackDecay()
    {
        double att = pow(currentPosition*(1/Attack),AttackTension);
        double dec = pow(Attack/currentPosition,DecayTension);
        return smoothMin(att,dec,0.2);
    }

    double smoothMin(double x, double y, double smoothing)
    {
        double r = max(smoothing-abs(x-y),0.0) / smoothing;
        return min(x,y)-r*r*smoothing*(1.0/4.0);
    }

    double getAmplitude()
    {
        double ret = AttackDecay();
        currentPosition++;
        return ret;
    }

    void startAttack()
    {
        currentPosition = 0.0;
    }

    void startRelease() {}

    void setAttack(double x)
    {
        Attack = 4.0 * pow(x+0.1,4) * sampleRate;
    }

    void setDecay(double x)
    {
        DecayTension = (1-pow(x,.1)) * 16;
    }

    void setSustain(double x) {}

    void setRelease(double x) {}

    void setResonance(double x)
    { 
        AttackTension = (x+.01) * 4.0;
    }
}


class ADSR_Gravitational
{
    ADSR_Gravitational() {}

    double Attack = 0.0;
    double Decay = 0.0;
    double Sustain = 1.0;
    double Release = 0.0;

    double getAmplitude()
    {
        return 0.0;
    }

    // non state change method, sample positional with a kind of gravity...
    // this way you simply chang ethe positions and 'affect' the locaiton of the envelope based on time...
    // one could think of this as falling though time.. Kung Fury style?


    // x count
    // basic = (x >= 0) ? 1: 0;
    // attack phase = (x >= 0) * (x <= a)
    // delay phase = (x >= a) * sustain
    // release phase = 0.0


}


/*


Attack (0.0 to 1.0)

0 to 1 ,  attack time 

SR   44100


If State = Attack
    IncValue =  (SR*8) / AKnob
If State = Decay
    IncValue = (SR*8) *  / AKnob

EnvPosition = EnvPosition + IncValue
if (EnvPosition >= 1.0) then 
  if (DecayKnob != 0.0) State = Decay
  else State = Sustain
*/