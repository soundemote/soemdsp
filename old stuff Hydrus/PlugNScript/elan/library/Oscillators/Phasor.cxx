class Phasor
{
    void reset()
    {
        phase = phaseOffset;
    }

	void setSampleRate(double v)
    {
        sampleRate = v;
        update();
    }
    
	void setFrequency(double v)
    {
        frequency = v;
        update();
    }

    void setPhase(double v)
    {
        phase = v;
    }

    void setPhaseOffset(double v)
    {
    	phaseOffset = v;
    }

    void inc()
    {
        phase += increment;
		phase = phase - floor(phase);
    }

	double getSample()
	{
		inc();
		return wrapPhaseExtreme(phase + phaseOffset);
	}

    double getValue()
    {
        return wrapPhaseExtreme(phase + phaseOffset);
    }

    protected void update()
    {
        increment = frequency / sampleRate;
    }

	double sampleRate = 44100;
	double phase = 0;
	double frequency = 440;
    double phaseOffset = 0;
    double increment = 0;
};