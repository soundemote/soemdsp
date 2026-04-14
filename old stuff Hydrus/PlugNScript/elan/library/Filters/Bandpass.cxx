/*
One pole bandpass filter created with simple lowpass and highpass filters with controls for frequency and width.
*/
#include "../math.cxx";
#include "Lowpass.cxx"
#include "Highpass.cxx"

class Bandpass
{
	Bandpass() {}

	Lowpass lowpass;
	Highpass highpass;
	double frequency = 0;
	double halfWidth = 0;
	double output = 0;

	void reset()
	{
		lowpass.reset();
		highpass.reset();
	}

	void setSampleRate(double v)
	{
		lowpass.setSampleRate(v);
		highpass.setSampleRate(v);
	}

	// total frequency range
	void setWidth(double v)
	{
		halfWidth = v * 0.5;
	}

	void setFrequency(double v)
	{
		frequency = v;
		update();
	}

	double getSample(double v)
	{	
		return output = highpass.getSample(lowpass.getSample(v));
	}

	void update()
	{
		lowpass.setFrequency(max(frequency + halfWidth,0));
		highpass.setFrequency(max(frequency - halfWidth,0));
	}
};