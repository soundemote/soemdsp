#include "XoxosOscillatorCore.h"

void XoxosOscillatorCoreOversampled::setSampleRate(double sr)
{
	hostSampleRate = sr;

	XoxosOscillatorCore::setSampleRate(oversampling * hostSampleRate);
}

/* OVERSAMPLED */
void XoxosOscillatorCore::setSampleRate(double v) 
{
	osc.setSampleRate(v);
}

bool XoxosOscillatorCore::isSilent()
{
	return false;
}

void XoxosOscillatorCore::triggerFrequencyChange()
{
	osc.setFrequency(currentFrequency);
}

void XoxosOscillatorCore::triggerAttack()
{

}

void XoxosOscillatorCore::triggerRelease()
{

}

void XoxosOscillatorCore::Reset()
{	
	osc.reset();
}

void XoxosOscillatorCoreOversampled::processSampleFrame(double * outL, double * outR)
{
	incrementPitchGlide();

	if (isSilent())
	{
		*outL = *outR = 0.0;
		return;
	}

	for (int i = 0; i < oversampling; i++)
	{
		XoxosOscillatorCore::processSampleFrame(outL, outR);

		*outL = antiAliasFilterL.getSampleDirect1(*outL);
	}

	*outL = *outR = clamp(*outL * outputAmp, -1.0, 1.0);
}

/* OVERSAMPLED*/
void XoxosOscillatorCore::processSampleFrame(double *outL, double *outR) 
{
	*outL = *outR = osc.getSample();
}

