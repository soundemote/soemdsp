#include "BasicOscillatorCore.h"

const vector<String> BasicOscillatorCore::ResetModeMenuItems{ "Always", "Legato", "When silent", "One-shot", "Never" };

void BasicOscillatorCoreOversampled::setSampleRate(double sr)
{
	hostSampleRate = sr;

	BasicOscillatorCore::setSampleRate(oversampling * hostSampleRate);
	adsr.setSampleRate(hostSampleRate);
	filter.setSampleRate(hostSampleRate);
}

/* OVERSAMPLED */
void BasicOscillatorCore::setSampleRate(double v) 
{
	osc.setSampleRate(v);
}

bool BasicOscillatorCore::isSilent()
{
	return adsr.isIdle();
}

void BasicOscillatorCore::triggerAttack()
{
	switch (resetMode)
	{
	case ResetMode::on_note:
	case ResetMode::when_not_legato:
		reset();
		break;
	case ResetMode::when_silent:
		if (adsr.isIdle())
			reset();
		break;
	case ResetMode::one_shot:
		reset();
		oneShotIsFinished = false;
		updateFreq();
		break;
	case ResetMode::never:
		break;
	}

	adsr.triggerAttack();
}

void BasicOscillatorCore::triggerNoteOnLegato()
{
	switch (resetMode)
	{
	case ResetMode::one_shot:
	case ResetMode::on_note:
		reset();
		adsr.triggerAttack();
		break;
	case ResetMode::when_not_legato:
  default: case ResetMode::never:
		break;
	}

	if (resetMode == ResetMode::one_shot)
		adsr.triggerAttack();
}

void BasicOscillatorCore::triggerNoteOffLegato()
{
	switch (resetMode)
	{
	case ResetMode::one_shot:
	case ResetMode::on_note:
		reset();
		adsr.triggerAttack();
		break;
	case ResetMode::when_not_legato:
  default: case ResetMode::never:
		break;
	}
}

void BasicOscillatorCore::triggerRelease()
{
	if (resetMode != ResetMode::one_shot)
		adsr.triggerRelease();
}

void BasicOscillatorCore::triggerOneShotRelease()
{
	adsr.triggerRelease();
}

void BasicOscillatorCore::reset()
{	
	osc.reset();
	adsr.reset();
	phaseTimer = 0;
	oneShotIsFinished = false;
}

void BasicOscillatorCoreOversampled::processSampleFrame(double * outL, double * outR)
{
	if (isSilent())
	{
		*outL = *outR = currentValue;
		return;
	}

	//if (oversampling > 1)
	//{
	//	for (int i = 0; i < oversampling; i++)
	//	{
	//		BasicOscillatorCore::processSampleFrame(outL, outR);
	//		*outL = antiAliasFilterL.getSampleDirect1(*outL);
	//	}
	//}
	//else
	//{
		BasicOscillatorCore::processSampleFrame(outL, outR);
	//}

	*outL = filter.getSample(*outL);	

	if (isUnipolar)
	{
		*outL = bipolarToUnipolar(*outL);
		*outL *= adsr.getSample();
		if (isInverted)
			*outL += -1;
	}
	else
	{
		if (isInverted)
			*outL *= -1;
		*outL *= adsr.getSample();
	}

	*outL *= gain;

	currentValue = *outR = *outL;
}

/* OVERSAMPLED*/
void BasicOscillatorCore::processSampleFrame(double *outL, double *outR) 
{
	handleOneShotMode();

	currentValue = osc.getSample();

	*outL = *outR = currentValue;
}

