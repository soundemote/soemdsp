#include "../math.cxx"
#include "../Randoms/NoiseGenerator.cxx"

void calculateOvershoot(double& v)
{
	v -= floor(v);
}

class LinearFeedbackEnvelope
{
	bool isIdle()
    {
        return envState == ENVSTATE::OFF;
    }

	void setSampleRate(double v)
    {
        secondsPerSample = 1.0/v;

        updateAttackIncrement();
        updateDecayIncrement();
        updateReleaseIncrement();

        feedbackAmtMultiplier = 44100.0/v;
        updateGlobalFeedback();        
    }

	void reset()
    {
        currentValue = 0;
        envState = ENVSTATE::OFF;
        secondsPassed = 0;
    }

	void triggerAttack(double velocity = 1, bool updateVelScale = true)
    {
        /* skip delay stage is delay time is negligible
        and skip attack stage if attack time is
        negliglbe. */
        if (updateVelScale)
            velocityScaling = 1 - (1-velocity) * velocityInfluence;

        if (envState == ENVSTATE::OFF && delayTime >= secondsPerSample)
        {            
            envState = ENVSTATE::DELAY;            
            return;
        }

        if (currentValue >= velocityScaling)
        {
            updateDecayIncrement();
            envState = ENVSTATE::DECAY;
            return;
        }        

        updateAttackIncrement();
        envState = ENVSTATE::ATTACK;
    }

	void triggerRelease()
    {
        envState = ENVSTATE::RELEASE;
        updateReleaseIncrement();
    }

	void setVelocityInfluence(double v) 
	{ 
		velocityInfluence = v;
	}

	double getSample()
    {
        if (doLoop)
        {
            r = noise.getSampleBipolar()*.00001;
            updatefbAttack();
            updatefbDecay();
        }
        else
            r = 0;

        switch (envState)
        {
        case ENVSTATE::OFF:
            break;

        case ENVSTATE::DELAY:
            
            secondsPassed += secondsPerSample;
            if (secondsPassed >= delayTime)
            {
                secondsPassed = 0;
                envState = ENVSTATE::ATTACK;		
            }
            break;

        case ENVSTATE::ATTACK:
            currentValue += calculateNextValue(attackIncrement, atkShape, fbAttack);
            if (currentValue >= velocityScaling)
            {
                double timeSpentDecaying = velocityScaling - 1/currentValue;
                currentValue = velocityScaling;
                updateDecayIncrement();
                currentValue -= timeSpentDecaying * calculateNextValue(decayIncrement, decShape, fbDecay);
                envState = ENVSTATE::DECAY;
            }
            break;

        case ENVSTATE::DECAY:
            if (currentValue < (sustainAmp*velocityScaling))
            {
                makeRandomDecision(currentValue);
                map(currentValue, 0.0, 1.0, sustainAmp, 1.0);
            }
            currentValue -= calculateNextValue(decayIncrement, decShape, fbDecay);
            // Instead of this if statement here, transition to 100% feedback 
            // where speed of transition is linear based on based on a "sustain factor"
            // such as feedback_value -= sustainFactor;
            // then when feedback_value >= 0%, go to sustain and hold current value.
            if (doLoop)
            {
                if (currentValue <= sustainAmp)
                {				
                double timeSpentAttacking = (((sustainAmp*velocityScaling) - abs(decayIncrement)) / currentValue) - velocityScaling;
                    calculateOvershoot(timeSpentAttacking);
                    //if (currentValue < 0 || currentValue > 1)
                        //makeRandomDecision(&currentValue);
                    currentValue = (sustainAmp*velocityScaling);
                    updateAttackIncrement();
                    currentValue += timeSpentAttacking * calculateNextValue(attackIncrement, atkShape, fbAttack);
                    envState = ENVSTATE::ATTACK;
                    if (currentValue > 1)
                        makeRandomDecision(currentValue);
                }				
            }
            else if (currentValue < (sustainAmp*velocityScaling))
            {
                envState = ENVSTATE::SUSTAIN;
                currentValue = (sustainAmp*velocityScaling);
            }
            break;

        case ENVSTATE::SUSTAIN:
                currentValue = (sustainAmp*velocityScaling);
            break;

        case ENVSTATE::RELEASE:
            // Release now controls the speed transition to intitial feedback value
            // feedback_value += releaseFactr;
            // if feedback_value == intial_feedback_value, stop incrementing
            // or you can try just going to %100% feedback value
            currentValue -= calculateNextValue(releaseIncrement, relShape, fbRelease);
            if (currentValue <= 0)
                reset();
            break;
        }

        if (currentValue < 0 || currentValue > 1)
            makeRandomDecision(currentValue);
        return currentValue;
    }

	void setDelayTime(double v) { delayTime = v; }
	void setAttackTime(double v) { attackTime = v; updateAttackIncrement(); }
	void setDecayTime(double v) { decayTime = v; updateDecayIncrement(); }
	void setReleaseTime(double v) { releaseTime = v; updateReleaseIncrement(); }
	void setGlobalTime(double v) 
	{ 
		globalTime = v; 
		updateAttackIncrement();
		updateDecayIncrement();
		updateReleaseIncrement();
	}

	void setSustainAmplitude(double v)
	{
		sustainAmp = clamp(v, 0.000001, .999999); //100% sustain was causing feedback gain to infinity.
	}

	void updateAttackIncrement()
    {
        attackIncrement = (velocityScaling-currentValue) * secondsPerSample / (attackTime+globalTime);
    }
	void updateDecayIncrement()
    {
        decayIncrement = (currentValue-(sustainAmp*velocityScaling)) * secondsPerSample / (decayTime+globalTime);
    }
	void updateReleaseIncrement()
    {
        releaseIncrement = (currentValue-0) * secondsPerSample / (releaseTime+globalTime);
    }

	void setAttackFeedback(double v) 
	{ 
		attackFBAmt = 1-v;
		attackFBAmt = exp(-v*10);
		updatefbAttack(); 
	}
	void setDecayFeedback(double v) 
	{ 
		decayFBAmt = 1-v; 
		decayFBAmt = exp(-v*10);
		updatefbDecay(); 
	}
	void setReleaseFeedback(double v) 
	{ 
		releaseFBAmt = 1-v;
		releaseFBAmt = exp(-v*10);
		updatefbRelease();
	}
	void setGlobalFeedback(double v)
    {
        feedbackAmt = v;
        updateGlobalFeedback();
    }

	void setAttackShape(int v) { atkShape = v; }
	void setDecayShape(int v) { decShape = v; }
	void setReleaseShape(int v) { relShape = v; }

	void setDoLoop(bool v)
	{
		doLoop = v;
		if (doLoop && (envState == ENVSTATE::SUSTAIN || envState == ENVSTATE::RELEASE || envState == ENVSTATE::OFF))
			triggerAttack();
	}
	void setDoOneShot(bool v) { doOneShot = v; }

	void setNoiseSeed(int v)
	{
		noise.setSeed(11128 + v);
		noise.reset();
	}

	int getEnvState() { return envState; }

	/* Core Objects*/	
	NoiseGenerator noise;
	double r = 0;

	bool doLoop = false;
	bool doOneShot = false;
	void makeRandomDecision(double& v)
    {
        v = noise.getSampleUnipolar();
    }

	/* Core Functions */
	void updateGlobalFeedback()
    {
        finalFeedbackAmtMultiplier = feedbackAmt*feedbackAmtMultiplier;
        updatefbAttack();
        updatefbDecay();
        updatefbRelease();
    }
	void updatefbAttack()
    {
        fbAttack = attackFBAmt + attackFBAmt * finalFeedbackAmtMultiplier;
        fbAttack = min(.999999, fbAttack); 
    }

	void updatefbDecay()
    {
        fbDecay = decayFBAmt + decayFBAmt * finalFeedbackAmtMultiplier;
        fbDecay = min(.999999, fbDecay);
    }

	void updatefbRelease()
    {
        fbRelease = releaseFBAmt + releaseFBAmt * finalFeedbackAmtMultiplier;
        fbRelease = min(.999999, fbRelease);
    }
	double calculateNextValue(double inc, int shape, double fbAmount)
    {
        inc = max(1.e-8, inc);

        double ouput;
        switch (shape)
        {
        case ENVSHAPE::EXP:
            ouput = inc + currentValue*fbAmount + fbAmount*r*.01 + r*.01;
            break;
        case ENVSHAPE::LOG:
            ouput =  inc + (velocityScaling-currentValue)*fbAmount + fbAmount*r + r*.001;
            break;
        case ENVSHAPE::SIGMOID:
            ouput =  inc + (velocityScaling-currentValue)*currentValue*fbAmount + fbAmount*r*.0001;
            break;
        case ENVSHAPE::POW:
            ouput =  inc + currentValue*currentValue*fbAmount + fbAmount*r*.01 + r*.0001;
            break;
        case ENVSHAPE::LIN: default:
            ouput = inc;
            break;
        }

        return ouput;
    }

	/* Internal Values*/
	int atkShape = ENVSHAPE::LOG;
    int decShape = ENVSHAPE::EXP;
    int relShape = ENVSHAPE::EXP;

	double
		velocityScaling = 1,
		velocityInfluence = 1;

	double
		secondsPerSample = 0,
		feedbackAmtMultiplier = 1,
		finalFeedbackAmtMultiplier = 1;

	double
		delayTime = 0,
		attackTime = 1.e-6,
		decayTime = .5,
		sustainAmp = 1,
		releaseTime = .5,
		globalTime = 1.e-6,

		attackIncrement = 0.0,
		decayIncrement = 0.0,
		releaseIncrement = 0.0,

		attackFBAmt = 0.0,
		decayFBAmt = 0.0,
		releaseFBAmt = 0.0,		
		feedbackAmt = 0,

		fbAttack = 0,
		fbDecay = 0,
		fbRelease = 0;

	double
		currentValue = 0,
		secondsPassed = 0;

	int envState = ENVSTATE::OFF;
};