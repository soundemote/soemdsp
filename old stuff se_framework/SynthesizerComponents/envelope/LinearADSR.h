#pragma once

/*
Next steps:
- Get rid of sustain and create an underlying envelope that controls
feedback amount where feedback gradually becomes 100% when it's time
to sustain. This will remove the hard "if" statement for starting
sustain stage and it will go from decay to sustain smoothly.

- would be awesome to do the same with delay so delay has a gradual onset to attack

- these natural curves could be better for guitar pluck envelopes

-	Get rid fo release stage by simplt modulating sustain.
*/

class baseADSR
{
public:
  baseADSR() = default;
  virtual ~baseADSR() = default;
  
	virtual void reset() {}

	virtual void trigger() {}
	virtual void triggerDelay() {}
	virtual void triggerAttack(double /*velocity = 1*/, bool /*updateVelScale = true*/) {}
	virtual void triggerDecay() {}
	virtual void triggerSustain() {}
	virtual void triggerRelease() {}
};

class FeedbackADSR : public baseADSR
{
public:
	enum EnvState { OFF, DELAY, ATTACK, DECAY, SUSTAIN, RELEASE };

	FeedbackADSR() = default;
	virtual ~FeedbackADSR() = default;

	bool isIdle();

	void setSampleRate(double v);

	void reset() override;

	virtual void trigger() override;
	virtual void triggerDelay() override;
	virtual void triggerAttack(double velocity = 1, bool updateVelScale = true) override;
	virtual void triggerDecay() override;
	virtual void triggerSustain() override;
	virtual void triggerRelease() override;

	void setVelocityInfluence(double v) 
	{ 
		velocityInfluence = v; 
	}

	double getSample();

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

	void updateAttackIncrement();
	void updateDecayIncrement();
	void updateReleaseIncrement();	

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
	void setGlobalFeedback(double v);

	enum Shape { EXP, POW, LOG, SIGMOID, LIN };
	void setAttackShape(Shape v) { atkShape = v; }
	void setDecayShape(Shape v) { decShape = v; }
	void setReleaseShape(Shape v) { relShape = v; }

	void setDoLoop(bool v)
	{
		doLoop = v;
		if (doLoop && (envState == SUSTAIN || envState == RELEASE || envState == OFF))
			triggerAttack();
	}
	void setDoOneShot(bool v) { doOneShot = v; }

	void setNoiseSeed(int v)
	{
		noise.setSeed(11128 + v);
		noise.reset();
	}

	EnvState getEnvState() { return envState; }

protected:
	/* Core Objects*/	
	elan::NoiseGenerator noise;
	double r = 0;

	bool doLoop = false;
	bool doOneShot = false;
	void makeRandomDecision(double * v);

	/* Core Functions */
	void updateGlobalFeedback();
	void updatefbAttack();
	void updatefbDecay();
	void updatefbRelease();
	double calculateNextVaue(double time, Shape shape, double fbAmt);

	/* Internal Values*/
	Shape
		atkShape = LOG,
		decShape = EXP,
		relShape = EXP;

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

	EnvState envState = EnvState::OFF;
};

class LinearADSR : public baseADSR
{
public:
	enum EnvState { OFF, DELAY, ATTACK, DECAY, SUSTAIN, RELEASE };

  LinearADSR() = default;
  virtual ~LinearADSR() = default;
  
	bool isIdle();
	EnvState getEnvState() { return envState; }

	void setSampleRate(double v);

	void reset() override;

	virtual void trigger() override;
	virtual void triggerDelay() override;
	virtual void triggerAttack(double velocity = 1, bool updateVelScale = true) override;
	virtual void triggerDecay() override;
	virtual void triggerSustain() override;
	virtual void triggerRelease() override;

	virtual double getSample();

	// 1.e-6 to inf exponential
	void setDelayTime(double v);

	// 1.e-6 to inf exponential
	void setAttackTime(double v);

	// 1.e-6 to inf exponential
	void setDecayTime(double v);

	// 0 to 1
	void setSustainAmplitude(double v);

	// 1.e-6 to inf exponential
	void setReleaseTime(double v);

	void setLooping(bool v)
	{ 
		isLooping = v; 
		if (isLooping && envState != ATTACK && envState != DECAY)
		{
			triggerAttack();
		}
	}

protected:

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

	EnvState envState = EnvState::OFF;
};
