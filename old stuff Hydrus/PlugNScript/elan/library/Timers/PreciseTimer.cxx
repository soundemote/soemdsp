class PreciseTimer
{
	// reset will call the timer function right away if phase is 1.0.
	// Set phase to 0.0 if you want a timer to trigger after the time.
	void reset(double phase = 1.0)
	{
		timer = phase;
		sampleDelayCounter = 0;
	}

	void setSampleRate(double v)
	{
		sampleRate = v;
		updateTimerInc();
		updateDelaySamples();
	}

	void setFrequency(double v)
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

	void increment()
	{
		if (sampleDelayCounter < delaySamples)
		{
			++sampleDelayCounter;
			return;
		}

        doTrigger = false;
        
		if (timer >= 1)
		{
			doTrigger = true;
			wrapTimer();
		}

		timer += timerInc;
	}

    bool hasTriggered()
    {
        return doTrigger;
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

	protected double timer = 1.0;
	protected double timerInc = 0.0;
	protected double frequency = 1.0;
	protected double sampleRate = 44100;
	protected double delaySeconds = 0;
	protected int delaySamples = 0;
	protected int sampleDelayCounter = 0;
    protected bool doTrigger = false;

	bool isTimerRunning = true;

	void updateTimerInc()
	{
		timerInc = min(frequency / sampleRate, 1.0);
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