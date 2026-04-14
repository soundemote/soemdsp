#include "JerobeamTorusCore.h"

const vector<String> JerobeamTorusCore::FeedbackSourceMenuItems{ "RampPhasor", "TriPhasor", "TriPhasor2", "Left", "Right", "L+R", "L-R" };
const vector<String> JerobeamTorusCore::ClippingModeMenuItems{ "Hard", "Soft" };
const vector<String> JerobeamTorusCore::KeytrackingMenuItems{ "Keytracking: On", "Keytracking: Off" };
const vector<String> JerobeamTorusCore::ResetModeMenuItems{ "Always", "Legato", "When silent", "One-shot", "Never" };
const vector<String> JerobeamTorusCore::EnvResetModeMenuItems{ "Always", "On-note", "Legato", "Never" };

JerobeamTorusCore::JerobeamTorusCore(MidiMaster* /*MasterFrequencyToUse*/)
{
	harmonicSmoother.setSmootherType(ParamSmoother::type::LINEAR);
	octaveSmoother.setSmootherType(ParamSmoother::type::LINEAR);
}

void JerobeamTorusCore::setSampleRate(double v)
{
	jbTorus.setSampleRate(v);
	bpFeedbackFilter.setSampleRate(v);
	ampEnv.setSampleRate(v);
	bandpassModule->jura::AudioModule::setSampleRate(v);
}

void JerobeamTorusCore::triggerRelease()
{
	// one shot mode will handle release specially
	if (resetMode != ResetMode::one_shot)
		ampEnv.triggerRelease();
}

bool JerobeamTorusCore::isSilent()
{
	return ampEnv.isIdle() &&
		!harmonicSmoother.needsSmoothing() &&
		!octaveSmoother.needsSmoothing();
}

void JerobeamTorusCore::triggerAttack()
{
	switch (resetMode)
	{
	case ResetMode::on_note:
	case ResetMode::when_not_legato:
		reset();
		break;
	case ResetMode::when_silent:
		if (ampEnv.isIdle())
			reset();
		break;
	case ResetMode::one_shot:
		reset();
		oneShotIsFinished = false;
		break;
	case ResetMode::never:
		break;
	}

	switch (ampEnvResetMode)
	{
	case EnvResetMode::env_always:
	case EnvResetMode::env_on_note:
	case EnvResetMode::env_when_not_legato:
		ampEnv.reset();
		ampEnv.triggerAttack(getMidiState().velocity, true);
		break;
	case EnvResetMode::env_never:
		ampEnv.triggerAttack(getMidiState().velocity, true);
		break;
	}
}

void JerobeamTorusCore::triggerNoteOnLegato()
{
	switch (resetMode)
	{
	case ResetMode::one_shot:
		reset();
		oneShotIsFinished = false;
		break;
	case ResetMode::on_note:
		reset();
		break;
	case ResetMode::when_not_legato:
	case ResetMode::when_silent:
	case ResetMode::never:
		break;
	}

	switch (ampEnvResetMode)
	{
	case EnvResetMode::env_always:
	case EnvResetMode::env_on_note:
		ampEnv.reset();
		ampEnv.triggerAttack(getMidiState().velocity, false);
		break;
	case EnvResetMode::env_when_not_legato:
	case EnvResetMode::env_never:
		break;
	}
}

void JerobeamTorusCore::triggerNoteOffLegato()
{
	switch (resetMode)
	{
	case ResetMode::one_shot:
		reset();
		oneShotIsFinished = false;
		break;
	case ResetMode::on_note:
		reset();
		break;
	case ResetMode::when_not_legato:
	case ResetMode::when_silent:
	case ResetMode::never:
		break;
	}

	switch (ampEnvResetMode)
	{
	case EnvResetMode::env_always:
		ampEnv.reset();
		ampEnv.triggerAttack(getMidiState().velocity, false);
		break;
	case EnvResetMode::env_on_note:
	case EnvResetMode::env_when_not_legato:
	case EnvResetMode::env_never:
		break;
	}
}

void JerobeamTorusCore::reset()
{
	jbTorus.reset();
	phaseTimer = 0;
}

void JerobeamTorusCore::calculateBeatFrequency()
{
	if (midiMasterPtr == nullptr)
		return;

	currentBeatFreq = midiMasterPtr->getBPMFactor() * MidiMaster::TempoMultipliers
		[
			clamp((double)tempoSelection, 0.0, double(MidiMaster::TempoMultipliers.size()-1))
		] * powerOfTwo((int)tempoMultiplier);
	calculatePitchToFrequency();
}

void JerobeamTorusCore::calculatePitchToFrequency()
{
	if (midiMasterPtr == nullptr)
		return;

	if (keytrackingMode == KeytrackingMode::On)
		currentFrequency = clamp(pitchToFreq((getMidiState().noteSmoothed + semitoneOffset) + octaveSmoother.getCurrentValue()) * harmonicSmoother.getCurrentValue(), -1.e+100, +1.e+100);
	else
		currentFrequency = 0;

	updateFrequency();
}

void JerobeamTorusCore::updateFrequency()
{
	if (midiMasterPtr == nullptr)
		return;

	finalFinalFINALfrequency = (currentFrequency + currentBeatFreq + frequencyOffset) * midiMasterPtr->getMasterRate();

	jbTorus.setFrequency(finalFinalFINALfrequency);
}

void JerobeamTorusCoreOversampled::processSampleFrame(double * outL, double * outR)
{
	if (harmonicSmoother.needsSmoothing() || octaveSmoother.needsSmoothing())
	{
		harmonicSmoother.inc();
		octaveSmoother.inc();
		calculatePitchToFrequency();
	}

	if (oversampling > 1)
	{
		for (int i = 0; i < oversampling; i++)
		{
			JerobeamTorusCore::processSampleFrame(outL, outR);

			*outL *= internalGain;
			*outR *= internalGain;

			*outL += dc_x + dc_x_y;
			*outR += dc_y + dc_x_y;

			/*channel mode*/
			MonoSynth::handleChannelMode(outL, outR);

			/*rotate*/
			rotate2D(rotation_sin, rotation_cos, outL, outR);

			/*envelope*/
			ampEnvVal = ampEnv.getSample();

			/*gain*/
			double finalAmp = outputAmp * ampEnvVal;

			/*filter*/
			bandpassModule->processSampleFrame(outL, outR);

			*outL *= finalAmp;
			*outR *= finalAmp;
			
			/*clip*/
			clipper.getSample(outL, outR);

			*outL = antiAliasFilterL.getSampleDirect1(*outL);
			*outR = antiAliasFilterR.getSampleDirect1(*outR);
		}
	}
	else
	{
		JerobeamTorusCore::processSampleFrame(outL, outR);

		*outL *= internalGain;
		*outR *= internalGain;

		*outL += dc_x + dc_x_y;
		*outR += dc_y + dc_x_y;

		/*channel mode*/
		MonoSynth::handleChannelMode(outL, outR);

		/*rotate*/
		rotate2D(rotation_sin, rotation_cos, outL, outR);

		/*envelope*/
		ampEnvVal = ampEnv.getSample();

		/*gain*/
		double finalAmp = outputAmp * ampEnvVal;

		/*filter*/
		bandpassModule->processSampleFrame(outL, outR);

		*outL *= finalAmp;
		*outR *= finalAmp;

		/*clip*/
		clipper.getSample(outL, outR);
	}
}

void JerobeamTorusCore::processSampleFrame(double *outL, double *outR)
{
	handleOneShotMode();

	jbTorus.getSampleFrame(&currentLeftChannelValue, &currentRightChannelValue);

	*outL = currentLeftChannelValue;
	*outR = currentRightChannelValue;

	/* Feedback handling */
	switch (feedbackSource)
	{
	case FeedbackSource::RampPhasor:
		feedbackValue = bpFeedbackFilter.getSample(jbTorus.phasor.getBipolarValue());
		break;
	case FeedbackSource::TriPhasor:
		feedbackValue = bpFeedbackFilter.getSample(jbTorus.getUnipolarTrianglePhasorValue()*2.0-1.0);
		break;
	case FeedbackSource::TriPhasor2:
		feedbackValue = bpFeedbackFilter.getSample(jbTorus.getUnipolarTrianglePhasorValue2()*2.0-1.0);
		break;
	case FeedbackSource::Left:
		feedbackValue = bpFeedbackFilter.getSample(*outL);
		break;
	case FeedbackSource::Right:
		feedbackValue = bpFeedbackFilter.getSample(*outR);
		break;
	case FeedbackSource::L_p_R:
		feedbackValue = bpFeedbackFilter.getSample((*outL+*outR)*0.5);
		break;
	case FeedbackSource::L_m_R:
		feedbackValue = bpFeedbackFilter.getSample(*outL-*outR);
		break;
	}

	feedbackValue *= feedbackAmp;
}

void JerobeamTorusCore::handleOneShotMode()
{
	if (resetMode != ResetMode::one_shot
		|| oneShotIsFinished
		|| ampEnv.getEnvState() == LinearADSR::EnvState::DELAY)
		return;

	if (phaseTimer >= 1)
	{
		oneShotIsFinished = true;
		ampEnv.triggerRelease();
		jbTorus.setFrequency(0);
		return;
	}

	jbTorus.setFrequency(finalFinalFINALfrequency);

	phaseTimer += abs(jbTorus.phasor.getIncrementAmount());
}
