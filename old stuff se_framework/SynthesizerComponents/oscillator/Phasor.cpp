#include "Phasor.h"

/*** SET ***/

void Phasor::setSampleRate(double v)
{
	secondsPerSample = 1 / v;
	calculateIncrementAmount();
}

void Phasor::setFrequency(double v)
{
	frequency = v;
	calculateIncrementAmount();
}

void Phasor::setPhaseOffset(double v)
{
	phaseOffset = v;
}

/*** GET ***/

double Phasor::getUnipolarValue()
{
	return wrapPhase(phase + phaseOffset);
}

double Phasor::getBipolarValue()
{
	return getUnipolarValue() * 2 - 1;
}

double Phasor::getIncrementAmount()
{
	return incAmt;
}

double Phasor::getPhaseOffset()
{
	return phaseOffset;
}

double Phasor::getSecondsPerSample()
{
	return secondsPerSample;
}

/*** ACTION ***/

void Phasor::reset()
{
	phase = 0;
}

void Phasor::partialReset(double v)
{
	phase = phase * (1 - v);
}

void Phasor::setPhase(double v)
{
	phase = v;
}

void Phasor::increment()
{
	phase = wrapPhase(phase + incAmt);
}

void Phasor::increment(double v)
{
	phase = wrapPhase(phase + v);
}

/*** INTERNAL ***/

void Phasor::calculateIncrementAmount()
{
	incAmt = secondsPerSample * frequency;
	incAmtAbs = std::abs(incAmt);
}

/*** PROCESS ***/

double Phasor::getSample()
{
	double out = getUnipolarValue();
	increment();
	return out;
}

double Phasor::getSampleBipolar()
{
	double out = getBipolarValue();
	increment();
	return out;
}
