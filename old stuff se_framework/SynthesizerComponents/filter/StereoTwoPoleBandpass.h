#pragma once

class StereoTwoPoleBandpass
{
public:
	StereoTwoPoleBandpass()
	{
		lpFilterL.setMode(RAPT::rsOnePoleFilter<double, double>::modes::LOWPASS_BLT);
		lpFilterR.setMode(RAPT::rsOnePoleFilter<double, double>::modes::LOWPASS_BLT);
		hpFilterL.setMode(RAPT::rsOnePoleFilter<double, double>::modes::HIGHPASS_BLT);
		hpFilterR.setMode(RAPT::rsOnePoleFilter<double, double>::modes::HIGHPASS_BLT);
	}

	RAPT::rsOnePoleFilter<double, double> lpFilterL;
	RAPT::rsOnePoleFilter<double, double> lpFilterR;
	RAPT::rsOnePoleFilter<double, double> hpFilterL;	
	RAPT::rsOnePoleFilter<double, double> hpFilterR;

	void setSampleRate(double v);

	void getSample(double * left, double * right);

	void setHighpassCutoff(double v);

	void setLowpassCutoff(double v);

protected:

	void updateHPFrequency();
	void updateLPFrequency();

	bool
		highpassIsBypassed = false,
		lowpassIsBypassed = false;

	double
		highpassCut = 0.01,
		lowpassCut = 22050;

	double sampleRate = 44100;
};

class MonoTwoPoleBandpass
{
public:
	MonoTwoPoleBandpass()
	{
		lpFilter.setMode(RAPT::rsOnePoleFilter<double, double>::modes::LOWPASS_BLT);
		hpFilter.setMode(RAPT::rsOnePoleFilter<double, double>::modes::HIGHPASS_BLT);
	}

	RAPT::rsOnePoleFilter<double,double> lpFilter, hpFilter;

	void setSampleRate(double v);

	double getSample(double in);

	void reset() { lpFilter.reset(); hpFilter.reset(); }

	void setHighpassCutoff(double v);

	void setLowpassCutoff(double v);

	void setIsHighpassBypass(bool v) { isHighpassBypassed = v; }
	void setIsLowpassBypass(bool v) { isLowpassBypassed = v; }

protected:

	bool isHighpassBypassed = false;
	bool isLowpassBypassed = false;
	double sampleRate = 44100;
};
