#include "../math.cxx"

class LinearSmoother
{
	LinearSmoother() {}

	double currentValue = 0;
	double targetValue = 0;

	void reset()
	{
		currentValue = targetValue;
	}

	void setInternalValue(double v)
	{
		currentValue = targetValue = v;
	}

	void setSampleRate(double v)
	{
		sampleRate = v;

		updateNumSamplesToTarget();
		updateIncrement();		
	}

	void setFrequency(double v)
	{
		frequency = v;

		updateNumSamplesToTarget();
		updateIncrement();
	}

	double getSample(double v)
	{
		if (targetValue != v)
		{
			targetValue = v;
			updateIncrement();
		}

		if (!needsSmoothing())
			return currentValue;
		
		currentValue += increment;

		return currentValue;
	}

	bool needsSmoothing()
	{
		return abs(targetValue - currentValue) > 1.e-6; 
	}

	protected void updateNumSamplesToTarget()
	{
		numSamplesToTarget = int(sampleRate / frequency);
	}

	protected void updateIncrement()
	{
		increment = (targetValue - currentValue) / numSamplesToTarget;
	}

	double sampleRate = 44100;
	double increment = 0;
	double frequency = 50;
	int numSamplesToTarget = 0;
};