#pragma once

class PreciseTimer
{
public:
  
  PreciseTimer() = default;
  virtual ~PreciseTimer() = default;

	// reset will call the timer function right away if phase is 1.0.
	// Set phase to 0.0 if you want a timer to trigger after the time.
	virtual void reset(double phase = 1.0)
	{
		timer = phase;
		sampleDelayCounter = 0;
	}

	virtual void setSampleRate(double v)
	{
		sampleRateInv = 1/v;
		sampleRate = v;
		updateTimerInc();
		updateDelaySamples();
	}

	void setSeconds(double v)
	{
		setFrequency(1/v);
	}

	virtual void setFrequency(double v)
	{
		frequency = v;
		updateTimerInc();
	}

	void incrememtIfRunning()
	{
		if (!isTimerRunning)
			return;
		
		increment();
	}

	virtual void increment()
	{
		if (sampleDelayCounter < delaySamples)
		{
			++sampleDelayCounter;
			return;
		}

		if (timer >= 1)
		{
			triggerFunction();
			wrapTimer();
		}

		timer += timerInc;
	}


	void startTimer() { isTimerRunning = true; }
	void stopTimer() { isTimerRunning = false; }
	bool getIsTimerRunning() { return isTimerRunning; }

	// returns a value between 0 and 1 to indicate time passed
	double getPosition() { return timer; }

	void setDelaySeconds(double v)
	{
		delaySeconds = v;
		updateDelaySamples();
	}

	std::function<void()> triggerFunction = []() { return; };

protected:
	double timer = 1.0;
	double timerInc = 0.0;
	double frequency = 1.0;
	double sampleRate = 44100;
	double sampleRateInv = 1/44100.0;
	double delaySeconds = 0;
	int delaySamples = 0;
	int sampleDelayCounter = 0;

	bool isTimerRunning = true;

	void updateTimerInc()
	{
		timerInc = std::min(sampleRateInv * frequency, 1.0);
	}

	void wrapTimer()
	{
		timer = timer - 1; // calculate overshoot
	}

	void updateDelaySamples()
	{
		delaySamples = int(delaySeconds * sampleRate);
	}

};

class DelayedTrigger
{
public:
	DelayedTrigger() = default;
	virtual ~DelayedTrigger() = default;

	void start() { isTimerRunning = true; }
	void stop() { isTimerRunning = false; }

	void reset()
 	{
		sampleDelayCounter = 0;
	}

	void restart()
	{
		reset();
		start();
	}

	void incrememt()
	{
		if (!isTimerRunning)
			return;

		if (sampleDelayCounter < delaySamples)
		{
			++sampleDelayCounter;
			return;
		}

		stop();
		triggerFunction();
	}

	virtual void setSampleRate(double v)
	{
		sampleRate = v;
		updateDelaySamples();
	}

	void setTriggerFunction(std::function<void()> func) { triggerFunction = func; }

	void setSeconds(double v)
	{
		delaySeconds = v;
		updateDelaySamples();
	}

	void setFrequency(double v)
	{
		delaySeconds = 1/v;
		updateDelaySamples();
	}

	bool getIsTimerRunning() { return isTimerRunning; }

protected:

	int sampleDelayCounter = 0;
	double delaySeconds = 1;
	bool isTimerRunning = false;
	int delaySamples = 44100;
	double sampleRate = 44100;

	std::function<void()> triggerFunction = []() { return; };

	void updateDelaySamples()
	{
		delaySamples = static_cast<int>(round(delaySeconds * sampleRate));
	}
};

class ClockDivider
{
public:
	ClockDivider() = default;
	~ClockDivider() = default;

	void reset()
	{
		numEvents = 0;
	}

	void trigger()
	{
		numEvents = numEvents % division;
		if (numEvents == 0)
			triggerFunction();

		++numEvents;
	}

	void setDivision(int v)
	{
		division = v;
	}

	int getDivision() { return division; }

	std::function<void()> triggerFunction = nullptr;

protected:

	int division = 1;
	int numEvents = 0;
};

template<class T> class EnvelopeFollower2
{
public:
	EnvelopeFollower2() : minMaxSmoother(10000)
	{
		typedef RAPT::rsPrototypeDesigner<T> PD;

		preFilter.setApproximationMethod(PD::BESSEL);
		preFilter.setOrder(6);

		minMaxSmoother.setSlewRateLimiting(1.0);
		minMaxSmoother.setMinMaxMix(0.5);

		postFilter.setApproximationMethod(PD::BESSEL);
		postFilter.setOrder(6);

		setSampleRate(sampleRate);	
	}

	void setSampleRate(T v)
	{
		sampleRate = v;

		preFilter.setSampleRate(sampleRate);
		preFilter.setFrequency(sampleRate / T(6));
		slewLimiter.setSampleRate(sampleRate);
		postFilter.setSampleRate(sampleRate);

		minTime = T(2) / sampleRate;

		setAttackTime(attackTime);
		setReleaseTime(releaseTime);
		setMinMaxSmoothingFrequency(minMaxSmoothingFreq);
		setPostFilterFrequency(postFilterFreq);
	}

	void setAttackTime(T v)
	{
		attackTime = std::max<T>(v, minTime);
		slewLimiter.setAttackTime(attackTime * T(1000));
	}

	void setReleaseTime(T v)
	{
		releaseTime = std::max<T>(v, minTime);
		slewLimiter.setReleaseTime(releaseTime * T(1000));
	}

	void setHoldTime(T v)
	{
		holdTime = v;
		slewLimiter.setHoldTime(holdTime * T(1000));
	}

	void setMinMaxSmoothingFrequency(T v)
	{
		minMaxSmoothingFreq = std::min(v, T(1) / minTime);
		minMaxSmoother.setLength(int(sampleRate / minMaxSmoothingFreq));
	}

	void setPostFilterFrequency(T v)
	{
		postFilterFreq = std::min(v, T(1) / minTime);		
		postFilter.setFrequency(postFilterFreq);
	}

	T getSample(T v)
	{
		v = rsAbs(preFilter.getSample(v));
		v = slewLimiter.getSample(v);
		v = minMaxSmoother.getSample(v);
		v = postFilter.getSample(v);

		return v;
	}

	void reset()
	{
		preFilter.reset();		
		minMaxSmoother.reset();
		slewLimiter.reset();
		postFilter.reset();
	}

protected:

	RAPT::rsEngineersFilter<T, T> preFilter;
	RAPT::rsSlewRateLimiterWithHold<T, T> slewLimiter;
	RAPT::rsMinMaxSmoother<T> minMaxSmoother;
	RAPT::rsEngineersFilter<T, T> postFilter;

	T inputFreq = T(100);
	T sampleRate = T(44100);
	T minTime = T(2)/sampleRate;

	T attackTime = T(.1);
	T releaseTime = T(.1);
	T holdTime = T(0);
	T minMaxSmoothingFreq = T(100);
	T postFilterFreq = T(100);	
};

class EnvelopeFollower
{
public:
	EnvelopeFollower() = default;
	~EnvelopeFollower() = default;

	void setSampleRate(double v)
	{
		sampleRate = static_cast<int>(v);
		ef.setSampleRate(v);
	}

	double getSample(double v)
	{
		return currentSlewedValue = ef.getSample(v);
	}

	bool isIdle()
	{
		return currentSlewedValue <= 1.e-6;
	}

	void setAttackTime(double v)
	{
		attackTime = v;
		ef.setAttackTime(v);
	}

	void setHoldTime(double v)
	{
		holdTime = v;
		ef.setHoldTime(v);
	}

	void setDecayTime(double v)
	{
		decayTime = v;
		ef.setReleaseTime(decayTime);
	}


	double currentSlewedValue;
	int sampleRate = 44100;
	double attackTime = 1;
	double holdTime = 1;
	double decayTime = 1;

	RAPT::rsSlewRateLimiterWithHold<double, double> ef;
};

class SampleAndHold
{
public:

	SampleAndHold()
	{
		timer.triggerFunction = [this]() { sampledValue = inputValue; };
	}

	~SampleAndHold() = default;

	void setSampleRate(double v)
	{
		timer.setSampleRate(v);
		smoother.setSampleRate(v);
	}

	void setFrequency(double v)
	{
		setSmoothingFrequency(v);
		setSamplingFrequency(v);
	}

	void setFrequency(double smoothingFrequency, double samplingFrequency)
	{
		setSmoothingFrequency(smoothingFrequency);
		setSamplingFrequency(samplingFrequency);
	}

	void reset()
	{
		timer.reset();
		smoother.setInternalValue(sampledValue);
	}

	void triggerNewSample()
	{
		timer.reset();
	}

	void incrementNoSmoothing(double v)
	{
		timer.increment();
		inputValue = v;
	}

	void increment(double v)
	{
		timer.increment();
		inputValue = v;

		smoothedValue = smoother.getSample(v);
	}

	double getInputValue() { return inputValue; }
	double getSampledValue() { return sampledValue; }
	double getSmoothedValue() { return smoothedValue; }

	void setSmoothingFrequency(double v)
	{
		smoother.setSmoothingTime(1.0 / v);
	}

	void setSamplingFrequency(double v)
	{
		timer.setFrequency(v);
	}

protected:
	double inputValue = 0;
	double smoothedValue = 0;
	double sampledValue = 0;

	LinearSmoother smoother;
	PreciseTimer timer;
};
