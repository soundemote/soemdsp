class PreciseOneShot
{
	void reset(double phase = 0.0)
	{
		position = phase;
		sampleDelayCounter = 0;
		timerIsRunning = true;
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

	void increment()
	{
		if (!timerIsRunning)
		{
			doTrigger = false;
			return;
		}

        if (position >= 1)
        {
			doTrigger = true;
			stopTimer();
            return;
        }

		if (sampleDelayCounter < delaySamples)
		{
			++sampleDelayCounter;
			return;
		}

        doTrigger = false;

		position += timerInc;
	}

    bool hasTriggered()
    {
        return doTrigger && position > 0;
    }

	void startTimer() { timerIsRunning = true; }
	void stopTimer() { timerIsRunning = false; }
	bool isTimerRunning() { return timerIsRunning; }

	// returns a value between 0 and 1 to indicate time passed
	double getPosition() { return position; }

	void setDelaySeconds(double v)
	{
		delaySeconds = v;
		
		updateDelaySamples();
	}

	protected double position = 1.0;
	protected double timerInc = 0.0;
	protected double frequency = 1.0;
	protected double sampleRate = 44100;
	protected double delaySeconds = 0;
	protected int delaySamples = 0;
	protected int sampleDelayCounter = 0;
    protected bool doTrigger = false;

	bool timerIsRunning = true;

	void updateTimerInc()
	{
		timerInc = min(frequency / sampleRate, 1.0);
	}

	void wrapTimer()
	{
		position = position - 1; // calculate overshoot
	}

	void updateDelaySamples()
	{
		delaySamples = int(delaySeconds * sampleRate);
	}

};