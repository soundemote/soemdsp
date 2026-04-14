#include "ChaoticaCore.h"

void ChaoticaCoreOversampled::setSampleRate(double sr)
{
	hostSampleRate = sr;

	ChaoticaCore::setSampleRate(oversampling * hostSampleRate);
	adsr.setSampleRate(hostSampleRate);
	filter.setSampleRate(hostSampleRate);
}

/* OVERSAMPLED */
void ChaoticaCore::setSampleRate(double v) 
{
	osc.setSampleRate(v);
	saw.setSampleRate(v);
}

bool ChaoticaCore::isSilent()
{
	return MonoSynth::isSilent() && adsr.isIdle();
}

void ChaoticaCore::triggerFrequencyChange()
{
	MonoSynth::triggerFrequencyChange();
	//osc.setFrequency(currentFrequency);
}

void ChaoticaCore::triggerAttack()
{
	MonoSynth::triggerAttack();
	adsr.triggerAttack();
}

void ChaoticaCore::triggerRelease()
{
	MonoSynth::triggerRelease();
	adsr.triggerRelease();
}

void ChaoticaCore::Reset()
{	
	MonoSynth::Reset();
	//osc.Reset();
	adsr.Reset();
}

void ChaoticaCoreOversampled::processSampleFrame(double * outL, double * outR)
{
	MonoSynth::incrementPitchGlide();

	if (isSilent())
	{
		*outL = *outR = 0.0;
		return;
	}
	sawVal = saw.getSample();
	for (int i = 0; i < oversampling; i++)
	{
		ChaoticaCore::processSampleFrame(outL, outR);

		*outL = antiAliasFilterL.getSampleDirect1(*outL);
		*outR = antiAliasFilterR.getSampleDirect1(*outR);
	}

	//*outL = filter.getSample(*outL) * adsr.getSample();

	*outL *= outputAmp;
	*outR *= outputAmp;
}

/* OVERSAMPLED*/
void ChaoticaCore::processSampleFrame(double *outL, double *outR) 
{
	osc.getSample(outL, outR);
}

