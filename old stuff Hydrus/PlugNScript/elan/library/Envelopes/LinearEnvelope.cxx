#include "../math.cxx"

class LinearEnvelope
{
	bool isIdle() { return envState == ENVSTATE::OFF; }
	int getEnvState() { return envState; }

	void setSampleRate(double v)
	{
		secondsPerSample = 1 / v;
		setDelayTime(delayTime);
		setAttackTime(attackTime);
		setSustainAmplitude(sustainAmp);
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
		* and skip attack stage if attack time is
		* negliglbe. */

		if (delayTime < secondsPerSample)
		{
			if (attackTime <= secondsPerSample)
			{
				envState = ENVSTATE::DECAY;
				currentValue = 1.0;						
			}
			else
			{
				envState = ENVSTATE::ATTACK;
			}
		}
		else
		{
			envState = ENVSTATE::DELAY;
		}
	}

	void triggerRelease()
	{
		envState = ENVSTATE::RELEASE;
		setReleaseTime(releaseTime);
	}

	double getSample()
	{
		switch (envState)
		{
		case ENVSTATE::OFF:
			break;

		case ENVSTATE::DELAY:
			secondsPassed += secondsPerSample;
			if (secondsPassed >= delayTime)
				envState = ENVSTATE::ATTACK;
			break;

		case ENVSTATE::ATTACK:
			currentValue += attackIncrement;
            
			if (currentValue >= 1)
			{
				currentValue = 1;
				envState = ENVSTATE::DECAY;
			}
			break;

		case ENVSTATE::DECAY:
			currentValue -= decayDecrement;
			if (currentValue <= sustainAmp)
			{
				currentValue = sustainAmp;
				envState = ENVSTATE::SUSTAIN;
			}
			break;

		case ENVSTATE::SUSTAIN:
			if (isLooping)
				envState = ENVSTATE::ATTACK;
			currentValue = sustainAmp;
			break;

		case ENVSTATE::RELEASE:
			currentValue -= releaseDecrement;
			if (currentValue <= 0)
				reset();
			break;
		}

		return currentValue;
	}

	// 1.e-6 to inf exponential
	void setDelayTime(double v)
	{
		delayTime = v;
	}

	// 1.e-6 to inf exponential
	void setAttackTime(double v)
	{
		attackTime = v;
		attackIncrement = min(secondsPerSample / attackTime, 1.0);
	}

	// 1.e-6 to inf exponential
	void setDecayTime(double v)
	{
		decayTime = v;
		decayDecrement = (1 - sustainAmp) * secondsPerSample / decayTime;
	}

	// 0 to 1
	void setSustainAmplitude(double v)
	{
		sustainAmp = v;
		setDecayTime(decayTime);
		setReleaseTime(releaseTime);
	}

	// 1.e-6 to inf exponential
	void setReleaseTime(double v)
	{
		releaseTime = v;
		releaseDecrement = currentValue * secondsPerSample / releaseTime;
	}

	void setLooping(bool v)
	{ 
		isLooping = v; 
		if (isLooping && (envState != ENVSTATE::ATTACK || envState != ENVSTATE::DECAY))
			triggerAttack();
	}

	double /* Parameters */
		sustainAmp = 1,
		delayTime = 0,
		attackTime = 0,
		decayTime = 1,
		releaseTime = 1;

	double /* Internal Values*/
		secondsPerSample = 0,
		attackIncrement = 0,
		decayDecrement = 0,
		releaseDecrement = 0;

	double /* Counters */
		currentValue = 0,
		secondsPassed = 0;

	bool isLooping = false;

	int envState = ENVSTATE::OFF;
};