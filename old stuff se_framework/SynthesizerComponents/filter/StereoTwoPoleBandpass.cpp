#include "StereoTwoPoleBandpass.h"

void StereoTwoPoleBandpass::setSampleRate(double v)
{
	lpFilterL.setSampleRate(v);
	lpFilterR.setSampleRate(v);
	hpFilterL.setSampleRate(v);
	hpFilterR.setSampleRate(v);
	sampleRate = v;
}

void StereoTwoPoleBandpass::getSample(double * left, double * right)
{
	if (!highpassIsBypassed)
	{
		*left = hpFilterL.getSample(*left);
		*right = hpFilterR.getSample(*right);
	}

	if (!lowpassIsBypassed)
	{
		*left = lpFilterL.getSample(*left);
		*right = lpFilterR.getSample(*right);
	}		
}
void StereoTwoPoleBandpass::setHighpassCutoff(double v)
{
	highpassCut = v;
	highpassIsBypassed = v < .01 + 1.e-6;
	updateHPFrequency();
}

void StereoTwoPoleBandpass::setLowpassCutoff(double v)
{
	lowpassCut = v;
	lowpassIsBypassed = v > sampleRate * 0.5 - 1.e-6;
	updateLPFrequency();
}

void StereoTwoPoleBandpass::updateHPFrequency()
{
	double cutoff = clamp(highpassCut, 0.0, sampleRate * 0.5);
	hpFilterL.setCutoff(cutoff);
	hpFilterR.setCutoff(cutoff);
}

void StereoTwoPoleBandpass::updateLPFrequency()
{
	double cutoff = clamp(lowpassCut, 0.0, sampleRate * 0.5);
	lpFilterL.setCutoff(cutoff);
	lpFilterR.setCutoff(cutoff);
}

/*************************************/

void MonoTwoPoleBandpass::setSampleRate(double v)
{
	lpFilter.setSampleRate(v);
	hpFilter.setSampleRate(v);
}

double MonoTwoPoleBandpass::getSample(double in)
{
	if (!isHighpassBypassed)
		in = hpFilter.getSample(in);

	if (!isLowpassBypassed)
		in = lpFilter.getSample(in);

	return in;
}

void MonoTwoPoleBandpass::setHighpassCutoff(double v)
{
	hpFilter.setCutoff(clamp(v, 0.0, sampleRate * 0.5));
}

void MonoTwoPoleBandpass::setLowpassCutoff(double v)
{
	lpFilter.setCutoff(clamp(v, 0.0, sampleRate * 0.5));
}
