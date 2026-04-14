class PluckEnvelope
{ 
    void reset()
    {
        x = 0;
        lengthInSecondsOffset = lengthModifier;
    }

    void setSampleRate(double v)
    {
        sampleRate = v;
        update();
    }

    void setSeconds(double v)
    {
        lengthInSeconds = v;
        update();
    }

    void setAttack(double v)
    {
        attack = v;
    }

    void setAttackRatio(double v)
    {
        attackRatio = v;
    }

    void setAttackMix(double v)
    {
        attackMix = v;
    }
    
    void setDecay(double v)
    {
        decay = v;
    }

    void setDecayRatio(double v)
    {
        decayRatio = v;
    }

    void setDecayMix(double v)
    {
        decayMix = v;
    }

    void setLengthInSeconds(double v)
    {
        lengthInSeconds = v;
        update();
    }

    void setLengthModifier(double v)
    {
        lengthModifier = v;
    }

    void setLoopTime(double v)
    {
        loopSeconds = v;
    }

    double getSample()
    {
        d1  = decay;
        d2  = decayRatio * decay;
        dec = decayMix * exp(-x/d1) + (1-decayMix) * exp(-x/d2);

        a1  = attack;
        a2  = attackRatio * attack;
        att = attackMix * exp(-x/a1) + (1-attackMix) * exp(-x/a2);

        env = dec - att;

        x += increment+lengthModifier;

        if (x > loopSeconds)
        {
            x = 0;
            lengthInSecondsOffset = lengthModifier;
        }

        lengthInSecondsOffset += lengthModifier;
        update();

        return env;
    }  

    void update()
    {        
        increment = (1.0/(lengthInSeconds + lengthInSecondsOffset)) / sampleRate;
    }

    // user variables
    double attack;
    double attackRatio;
    double attackMix;
    double decay;
    double decayRatio;
    double decayMix;
    double lengthInSeconds;
    double lengthInSecondsOffsetInc = 0;
    double lengthModifier = 0;
    double loopSeconds = 0;

    // internal states
    double d1, d2, dec, a1, a2, att;
    double x = 0;
    double env = 0;
    double sampleRate = 44100;
    double increment = .0001;
    double lengthInSecondsOffset;
};