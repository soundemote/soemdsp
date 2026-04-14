/*
Slowly drifts between random values based on a given frequency and amplitude
*/

#include "../math.cxx"
#include "NoiseGenerator.cxx"
#include "../Filters/LinearSmoother.cxx"
#include "../Filters/OnePoleFilter.cxx"

class FlexibleRandomWalk
{
	FlexibleRandomWalk() 
	{
		hpf.setMode(2);
		hpf.setFrequency(5);
	}
	
	double phase = 0;
	double increment = 0;
	double sampleFrequency = 0;

	double currentValue = 0;
	double currentSampledValue = 0;

	double sampleRate = 0;
	double samplePeriod = 0;

	OnePoleFilter hpf;

	void reset()
	{
		currentValue = 0;
		phase = 0;
		noiseGenerator.reset();
		linearSmoother.reset();
		hpf.reset();
	}

	void setSampleRate(double v)
	{
		sampleRate = v;
		samplePeriod = 1.0 / v;
		updateIncrement();
		linearSmoother.setSampleRate(sampleRate);
	}

	void setSeed(int v)
	{
		noiseGenerator.setSeed(v);
	}

	// speed of value changes frequency from 0 to sampleRate
	void setDetail(double v)
	{
		sampleFrequency =/* min(*/v/*, sampleRate)*/;
		linearSmoother.setFrequency(v);
		updateIncrement();
	}

	// depth of value changes, can be from 0 to 1, negative values are ok
	void setColor(double v)
	{
		noiseAmplitude = v;
		updateIncrement();
	}

	void setHighpassFrequency(double v)
	{
		hpf.setFrequency(v);
	}

	void setBypassHPF(bool v)
	{
		bypassHPF = v;

		if (bypassHPF)
			hpf.reset();
	}

	// bipolar values
	double getSample()
	{
		phase += increment;		

		if (abs(currentSampledValue) >= 1.5 || isClipping)
		{
			isClipping = true;

			if (currentSampledValue > 0)
				currentSampledValue -= .000000001;
			else
				currentSampledValue += .000000001;
		}

		if (abs(currentSampledValue) <= 0.9)
			isClipping = false;

		if (phase > 1)
		{	
			phase -= 1;
			lastSampledValue = currentSampledValue;
			currentSampledValue = currentValue;			
		}

		currentValue = clamp(currentValue + noiseGenerator.getSampleBipolar() * noiseAmplitude, -1.5, 1.5);

		//double sinusoidSmoother = map(cos(phase*PI + PI), -1, 1, lastSampledValue, currentSampledValue);

		// int bypassHPFInt = bypassHPF == false ? 0 : 1;
		// double test = 0;
		// switch(bypassHPFInt)
		// {
		// 	case 0:
		// 		return hpf.getSample(linearSmoother.getSample(sinusoidSmoother) * twoThirds);
		// 	case 1: 
		// 	default:
				//return linearSmoother.getSample(sinusoidSmoother) * twoThirds;
		return linearSmoother.getSample(currentValue) * twoThirds;
	//	}

		//return 0;
	}	

	protected void updateIncrement()
	{
		increment = samplePeriod * sampleFrequency;
	}

	NoiseGenerator noiseGenerator;
	protected LinearSmoother linearSmoother;

	protected bool isClipping;

	protected bool bypassHPF = false;
	protected double noiseAmplitude = 0;
	protected double lastSampledValue = 0;
	protected double twoThirds = 2.0/3.0;
};

	// double LinearInterpolate(double y1,double y2, double mu)
// {
//    return y1 * (1-mu) + y2 * mu;
// }

// double CosineInterpolate(double y1, double y2, double mu)
// {
//    double mu2 = 1 - cos(mu*PI) * 0.5;

//    return y1 * (1 - mu2) + y2 * mu2;
// }

// class CubicInterpolator
// {
// 	CubicInterpolator() {}

// 	array<double> memory = {0, .1, .2, .3};

// 	void setTargetValue(double v)
// 	{
// 		memory[2] = v;
// 	}

// 	double getSample(double a, double b, double c, double d, double phase)
// 	{
// 		// if (memory[0] != v2)
// 		// {	
// 		// 	// for (int i = memory.length; i-- > 1; )
// 		// 	// 	memory[i] = memory[i-1];

// 		// 	// memory[0] = v;
// 		// 	memory[3] = memory[2];
// 		// 	memory[2] = memory[1];
// 		// 	memory[1] = memory[0];
// 		// 	memory[0] = v2;
// 		// }

// 		//print("0:"+memory[0]+ "1:"+memory[1]+" 2:"+memory[2]+" 3:"+memory[3]+" v:"+v);

// 		return cubicInterpolate(memory[0], memory[1], memory[2], memory[3], phase);
// 	}

// 	protected double cubicInterpolate(double y0, double y1, double y2,double y3, double mu)
// 	{
// 	   double a0,a1,a2,a3,mu2;

// 	   mu2 = mu*mu;
// 	   a0 = y3 - y2 - y0 + y1;
// 	   a1 = y0 - y1 - a0;
// 	   a2 = y2 - y0;
// 	   a3 = y1;

// 	   return currentValue = a0*mu*mu2 + a1*mu2 + a2*mu + a3;
// 	}

// 	protected double catmullRomInterpolate(double y0, double y1, double y2, double y3, double mu)
// 	{
// 	   double a0,a1,a2,a3,mu2;

// 	   mu2 = mu*mu;
// 	   a0 = -0.5*y0 + 1.5*y1 - 1.5*y2 + 0.5*y3;
// 	   a1 = y0 - 2.5*y1 + 2*y2 - 0.5*y3;
// 	   a2 = -0.5*y0 + 0.5*y2;
// 	   a3 = y1;

// 	   return currentValue = a0*mu*mu2 + a1*mu2 + a2*mu + a3;
// 	}

// 	protected double targetValue;
// 	protected double currentValue;
// }

	/* CUBIC INTERPOLATION TEST
		y0 = y1;
		y1 = y2;
		y2 = y3;
		y3 = currentSampledValue;

		double a0,a1,a2,a3,mu2;

		mu = phase;	

		mu2 = mu*mu;
		a0 = -0.5*y0 + 1.5*y1 - 1.5*y2 + 0.5*y3;
		a1 = y0 - 2.5*y1 + 2*y2 - 0.5*y3;
		a2 = -0.5*y0 + 0.5*y2;
		a3 = y1;

		return a0*mu*mu2 + a1*mu2 + a2*mu + a3;
		*/

	/* cubic interpolation memory
	double y0 = 0; 
	double y1 = 0; 
	double y2 = 0;
	double y3 = 0;
	double mu = 0;
	*/