/* Linearly smoothed S&H that outputs bipolar values. */

#include "NoiseGenerator.cxx"
#include "../Timers/PreciseTimer.cxx"
#include "../Filters/LinearSmoother.cxx"

class SampleAndHold
{
	void setSampleRate(double v)
	{
		sampleRate = v;

		timer.setSampleRate(v);
		smoother.setSampleRate(v);
	}
	
	void reset()
	{
		noise.reset();
		timer.reset();
		smoother.setInternalValue(sampledValue);
	}

	void trigger()
	{
		timer.reset();		
	}

	void increment()
	{
		noiseValue = noise.getSampleBipolar();

        timer.increment();

        if (timer.hasTriggered())
		{
			lastSampledValue = sampledValue;
            currentSampledValue = noiseValue;
		}
		
		sampledValue = map(cos(timer.getPosition()*PI + PI), 0, 1, lastSampledValue, currentSampledValue);		

		smoothedValue = smoother.getSample(sampledValue);
	}

	double getNoiseValue()    { return noiseValue;    }
	double getSampledValue()  { return sampledValue;  }
	double getSmoothedValue() { return smoothedValue; }

	void setSeed(int v)
	{
		noise.setSeed(v);
	}

	void setFrequency(double v)
	{
		setSmoothingFrequency(v);
		setSamplingFrequency(v);
	}

	void setSmoothingFrequency(double v)
	{
		smoother.setFrequency(v);
	}

	void setSamplingFrequency(double v)
	{	
		timer.setFrequency(v);
	}

	protected double noiseValue = 0;
	protected double smoothedValue = 0;
	protected double sampledValue = 0;
	protected double lastSampledValue = 0;
	protected double currentSampledValue = 0;
	protected double sampleRate = 44100;

	protected NoiseGenerator noise;
	protected LinearSmoother smoother;
	protected PreciseTimer timer;
};