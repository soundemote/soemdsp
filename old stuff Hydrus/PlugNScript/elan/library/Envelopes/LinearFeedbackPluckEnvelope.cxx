#include "../math.cxx"
#include "../Randoms/NoiseGenerator.cxx"
#include "../Timers/PreciseOneShot.cxx"

class LinearFeedbackEnvelope
{
	bool isIdle()
    {
        return envState == ENVSTATE::OFF;
    }

	void setSampleRate(double v)
    {
        secondsPerSample = 1.0/v;
        oneShotTimer.setSampleRate(v);
        updateDecayIncrement();
        updateReleaseIncrement();        
    }

	void reset()
    {
        currentValue = 0;
        envState = ENVSTATE::OFF;
        secondsPassed = 0;
        oneShotTimer.reset();
    }

	void triggerAttack(double velocity = 1)
    {
        currentValue = velocity;

        envState = ENVSTATE::DECAY;
        oneShotTimer.reset();
        updateDecayIncrement();
    }

	void triggerRelease()
    {
        envState = ENVSTATE::RELEASE;
        updateReleaseIncrement();
    }

	double getSample()
    {
        if (oneShotTimer.hasTriggered())
        {
            finalDecayMod = endingDecay;
            updateDecayFeedback();
        }
        else if (oneShotTimer.isTimerRunning())
        {
            finalDecayMod = decay+map(rationalCurve(oneShotTimer.getPosition(),decayModCurve), decayModStart, decayModEnd);
            updateDecayFeedback();
        }

        switch (envState)
        {
        case ENVSTATE::OFF:
            break;

        case ENVSTATE::DECAY:
            currentValue -= decayIncrement + currentValue*currentValue*fbDecay;
            
            if (currentValue < 0)
                reset();            
            break;

        case ENVSTATE::RELEASE:                        
            currentValue -= releaseIncrement + currentValue*currentValue*fbRelease;
            if (currentValue <= 0)
                reset();
            break;
        }

        oneShotTimer.increment();

        return currentValue;
    }

	void updateDecayIncrement()
    {
        decayIncrement = (currentValue-1) * secondsPerSample / 50;
    }

	void updateReleaseIncrement()
    {
        releaseIncrement = currentValue * secondsPerSample / 50;
    }

	void setDecay(double v) 
	{ 
		decay = v;
	}

    void setDecayModStart(double v)
    {
        decayModStart = v;
    }
    void setDecayModEnd(double v)
    {
        decayModEnd = v;
    }
    void setDecayModFrequency(double v)
    {
        oneShotTimer.setFrequency(v);
    }
    void setDecayModCurve(double v)
    {
        decayModCurve = v;
    }
    void setFinalDecay(double v)
    {
        endingDecay = v;
    }

    void updateDecayFeedback()
    {
        fbDecay = min(.999999, exp(-(finalDecayMod)*10));
    }

	void setRelease(double v) 
	{ 
		fbRelease = min(.999999, exp(-v*10));
	}

	int getEnvState() { return envState; }

    PreciseOneShot oneShotTimer;

	double secondsPerSample = 0;

	double decayIncrement = 0.0;
	double releaseIncrement = 0.0;

	double fbAttack = 0;
	double fbDecay = 0;
    double decay = 0;

    double finalDecayMod = 0;    
    double decayModStart = 0;
    double decayModEnd = 0;
    double endingDecay = 0;
    double decayModCurve = 0;

	double fbRelease = 0;

	double currentValue = 0;
	double secondsPassed = 0;

	int envState = ENVSTATE::OFF;
};