/*
Slowly drifts between random values based on a given frequency and amplitude
*/

#include "../math.cxx"
#include "NoiseGenerator.cxx"

class RandomWalk
{
	RandomWalk() {}

	double frequency = 0;
	double range = 1;
	double value = 0;
	double bias = 0;
	double sampleRate = 0;
	double phase = 0;

	void reset()
	{
		phase = 0;
		noiseGenerator.reset();
	}

	void setSampleRate(double v)
	{
		sampleRate = v;
		samplePeriod = TAU / sampleRate;
		updateIncrement();
	}

	void setPhase(double v)
	{
		phase = v * TAU;
	}

	// speed of value changes in frequency from 0 to sampleRate
	void setColor(double v)
	{
		frequency = min(v, sampleRate);
		updateIncrement();
	}

	// depth of value hanges from 0 to 1, negative values are ok
	void setRange(double v)
	{
		range = v;
	}

	void setBias(double v)
	{
		bias = v;
	}

	void setSeed(int v)
	{
		noiseGenerator.setSeed(v);
	}

	double getSample()
	{
		phase += increment;

    	bool isHalfPhase = (lastPhase <= PI) && (phase >= PI);
		bool isFullPhase = (lastPhase <= TAU) && (phase >= TAU);

		if (isHalfPhase)
			a = bias - range + range * noiseGenerator.getSampleUnipolar() * 2 ;
		else if (isFullPhase)
			b = bias - range + range * noiseGenerator.getSampleUnipolar() * 2;

	    if (isFullPhase)	    	
			phase -= TAU;

    	lastPhase = phase;

    	value = ((a-b) * cos(phase) + (a + b)) * 0.5;   	

		return value;
	}

	protected void updateIncrement()
	{
		increment = samplePeriod * frequency;
	}

	protected NoiseGenerator noiseGenerator;

	protected double lastPhase = 0;
	protected double increment = 0;

	protected double samplePeriod;

	protected double a;
	protected double b;
};