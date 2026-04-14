/*
Slowly drifts between random values based on a given frequency and amplitude
*/

#include "../math.cxx"
#include "NoiseGenerator.cxx"
#include "../Dynamics/SoftClipper.cxx"
#include "../Filters/Highpass.cxx"

class DriftGenerator
{
	DriftGenerator() 
	{
		hpf.setFrequency(.1);
	}

	double sampleRate = 0;
	double phase = 0;
	double frequency = 0;
	double value = 0;
	double amplitude = 1;

	void setSampleRate(double v)
	{
		sampleRate = v;
		hpf.setSampleRate(v);
		updateIncrement();
	}

	void reset(double v)
	{
		phase = 0;
		noiseGenerator.reset();
		hpf.reset();
	}

	void setColor(double v)
	{
		frequency = v;
		updateIncrement();
	}

	void setAmplitude(double v)
	{
		amplitude = v;
	}

	void setSeed(int v)
	{
		noiseGenerator.setSeed(v);
	}

	double getSample()
	{
		phase += increment * noiseGenerator.getSampleBipolar() * amplitude;

		if (phase >= 1) 
			phase -= 1;
		else if (phase < 0)
			phase += 1;

		value = sin(phase*TAU);

		return hpf.getSample(value);
	}

	protected void updateIncrement()
	{
		increment = frequency / sampleRate;
	}

	protected NoiseGenerator noiseGenerator;
	Highpass hpf;

	protected double increment = 0;
}