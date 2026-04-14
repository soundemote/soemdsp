#include "JerobeamMushroomCore.h"

const vector<String> JerobeamMushroomCore::FeedbackSourceMenuItems{ "RampPhasor", "TriPhasor", "Left", "Right", "L+R", "L-R" };
const vector<String> JerobeamMushroomCore::ClippingModeMenuItems{ "Hard", "Soft" };
const vector<String> JerobeamMushroomCore::KeytrackingMenuItems{ "Keytracking: On", "Keytracking: Off" };
const vector<String> JerobeamMushroomCore::ResetModeMenuItems{ "Always", "Legato", "When silent", "One-shot", "Never" };
const vector<String> JerobeamMushroomCore::EnvResetModeMenuItems{ "Always", "On-note", "Legato", "Never" };

JerobeamMushroomCore::JerobeamMushroomCore(MidiMaster * MasterFrequencyToUse)
{
	harmonicSmoother.setSmootherType(ParamSmoother::type::LINEAR);
	octaveSmoother.setSmootherType(ParamSmoother::type::LINEAR);
}

void JerobeamMushroomCore::setSampleRate(double v)
{
	jbMushroom.setSampleRate(v);
	bpFeedbackFilter.setSampleRate(v);
	ampEnv.setSampleRate(v);
	bandpassModule->jura::AudioModule::setSampleRate(v);
}

void JerobeamMushroomCore::triggerRelease()
{
	// one shot mode will handle release specially
	if (resetMode != ResetMode::one_shot)
		ampEnv.triggerRelease();
}

bool JerobeamMushroomCore::isSilent()
{
	return ampEnv.isIdle() &&
		!harmonicSmoother.needsSmoothing() &&
		!octaveSmoother.needsSmoothing();
}

void JerobeamMushroomCore::triggerAttack()
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

void JerobeamMushroomCore::triggerNoteOnLegato()
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

void JerobeamMushroomCore::triggerNoteOffLegato()
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

void JerobeamMushroomCore::reset()
{
	jbMushroom.reset();
	phaseTimer = 0;
}

void JerobeamMushroomCore::handleOneShotMode()
{
	if (resetMode != ResetMode::one_shot
		|| oneShotIsFinished
		|| ampEnv.getEnvState() == LinearADSR::EnvState::DELAY)
		return;

	if (phaseTimer >= 1)
	{
		oneShotIsFinished = true;
		ampEnv.triggerRelease();
		jbMushroom.setFrequency(0);
		return;
	}

	jbMushroom.setFrequency(finalFinalFINALfrequency);

	phaseTimer += abs(jbMushroom.phasor.getIncrementAmount());
}

void JerobeamMushroomCore::calculateBeatFrequency()
{
	if (midiMasterPtr == nullptr)
		return;

	currentBeatFreq = midiMasterPtr->getBPMFactor() * MidiMaster::TempoMultipliers
		[
			(size_t)clip((double)tempoSelection, 0.0, double(MidiMaster::TempoMultipliers.size()-1))
		] * powerOfTwo((int)tempoMultiplier);
	calculatePitchToFrequency();
}

void JerobeamMushroomCore::calculatePitchToFrequency()
{
	if (midiMasterPtr == nullptr)
		return;

	if (keytrackingMode == KeytrackingMode::On)
		currentFrequency = clip(pitchToFreq((getMidiState().noteSmoothed + semitoneOffset) + octaveSmoother.getCurrentValue()) * harmonicSmoother.getCurrentValue(), -1.e+100, +1.e+100);
	else
		currentFrequency = 0;

	updateFrequency();
}

void JerobeamMushroomCore::updateFrequency()
{
	if (midiMasterPtr == nullptr)
		return;

	finalFinalFINALfrequency = (currentFrequency + currentBeatFreq + frequencyOffset) * midiMasterPtr->getMasterRate();

	jbMushroom.setFrequency(finalFinalFINALfrequency);
}

void JerobeamMushroomCoreOversampled::processSampleFrame(double * outL, double * outR)
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
			JerobeamMushroomCore::processSampleFrame(outL, outR);

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
		JerobeamMushroomCore::processSampleFrame(outL, outR);

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

void JerobeamMushroomCore::processSampleFrame(double *outL, double *outR)
{
	handleOneShotMode(); // need double phase increment due to how mushroom generator calculates frequency

	jbMushroom.getSampleFrame(&currentLeftChannelValue, &currentRightChannelValue);

	*outL = currentLeftChannelValue;
	*outR = currentRightChannelValue;

	/* Feedback handling */
	switch (feedbackSource)
	{
	case FeedbackSource::RampPhasor:
		feedbackValue = bpFeedbackFilter.getSample(jbMushroom.phasor.getBipolarValue());
		break;
	case FeedbackSource::TriPhasor:
		feedbackValue = bpFeedbackFilter.getSample(jbMushroom.getUnipolarTrianglePhasorValue()*2.0-1.0);
		break;
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
