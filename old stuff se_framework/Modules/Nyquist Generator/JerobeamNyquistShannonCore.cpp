#include "JerobeamNyquistShannonCore.h"

const vector<String> JerobeamNyquistShannonCore::FeedbackSourceMenuItems{ "RampPhasor", "TriPhasor", "Left", "Right", "L+R", "L-R" };
const vector<String> JerobeamNyquistShannonCore::ClippingModeMenuItems{ "Hard", "Soft" };
const vector<String> JerobeamNyquistShannonCore::KeytrackingMenuItems{ "Keytracking: On", "Keytracking: Off" };
const vector<String> JerobeamNyquistShannonCore::ResetModeMenuItems{ "Always", "Legato", "When silent", "One-shot", "Never" };
const vector<String> JerobeamNyquistShannonCore::EnvResetModeMenuItems{ "Always", "On-note", "Legato", "Never" };

JerobeamNyquistShannonCore::JerobeamNyquistShannonCore(MidiMaster * MasterFrequencyToUse)
{
	harmonicSmoother.setSmootherType(ParamSmoother::type::LINEAR);
	octaveSmoother.setSmootherType(ParamSmoother::type::LINEAR);
}

void JerobeamNyquistShannonCore::setSampleRate(double v)
{
	jbNyquistShannon.setSampleRate(v);
	bpFeedbackFilter.setSampleRate(v);
	ampEnv.setSampleRate(v);
	bandpassModule->jura::AudioModule::setSampleRate(v);
}

void JerobeamNyquistShannonCore::triggerRelease()
{
	// one shot mode will handle release specially
	if (resetMode != ResetMode::one_shot)
		ampEnv.triggerRelease();
}

bool JerobeamNyquistShannonCore::isSilent()
{
	return ampEnv.isIdle() &&
		!harmonicSmoother.needsSmoothing() &&
		!octaveSmoother.needsSmoothing();
}

void JerobeamNyquistShannonCore::triggerAttack()
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

void JerobeamNyquistShannonCore::triggerNoteOnLegato()
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

void JerobeamNyquistShannonCore::triggerNoteOffLegato()
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

void JerobeamNyquistShannonCore::reset()
{
	jbNyquistShannon.reset();
	phaseTimer = 0;
}

void JerobeamNyquistShannonCore::handleOneShotMode()
{
	if (resetMode != ResetMode::one_shot
		|| oneShotIsFinished
		|| ampEnv.getEnvState() == LinearADSR::EnvState::DELAY)
		return;

	if (phaseTimer >= 1)
	{
		oneShotIsFinished = true;
		ampEnv.triggerRelease();
		jbNyquistShannon.setFrequencyA(0);
		return;
	}

	jbNyquistShannon.setFrequencyA(finalFinalFINALfrequency);

	phaseTimer += abs(jbNyquistShannon.phasor.getIncrementAmount());
}

void JerobeamNyquistShannonCore::calculateBeatFrequency()
{
	if (midiMasterPtr == nullptr)
		return;

	currentBeatFreq = midiMasterPtr->getBPMFactor() * MidiMaster::TempoMultipliers
		[
			(size_t)clip((double)tempoSelection, 0.0, double(MidiMaster::TempoMultipliers.size()-1))
		] * powerOfTwo((int)tempoMultiplier);
	calculatePitchToFrequency();
}

void JerobeamNyquistShannonCore::calculatePitchToFrequency()
{
	if (midiMasterPtr == nullptr)
		return;

	if (keytrackingMode == KeytrackingMode::On)
		currentFrequency = clip(pitchToFreq((getMidiState().noteSmoothed + semitoneOffset) + octaveSmoother.getCurrentValue()) * harmonicSmoother.getCurrentValue(), -1.e+100, +1.e+100);
	else
		currentFrequency = 0;

	updateFrequency();
}

void JerobeamNyquistShannonCore::updateFrequency()
{
	if (midiMasterPtr == nullptr)
		return;

	finalFinalFINALfrequency = (currentFrequency + currentBeatFreq + frequencyOffset) * midiMasterPtr->getMasterRate();

	jbNyquistShannon.setFrequencyA(finalFinalFINALfrequency);
}

void JerobeamNyquistShannonCoreOversampled::processSampleFrame(double * outL, double * outR)
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
			JerobeamNyquistShannonCore::processSampleFrame(outL, outR);

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
		JerobeamNyquistShannonCore::processSampleFrame(outL, outR);

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

void JerobeamNyquistShannonCore::processSampleFrame(double *outL, double *outR)
{
	handleOneShotMode(); // need double phase increment due to how mushroom generator calculates frequency

	jbNyquistShannon.getSampleFrame(&currentLeftChannelValue, &currentRightChannelValue);

	*outL = currentLeftChannelValue;
	*outR = currentRightChannelValue;

	/* Feedback handling */
	switch (feedbackSource)
	{
	case FeedbackSource::RampPhasor:
		feedbackValue = bpFeedbackFilter.getSample(jbNyquistShannon.phasor.getBipolarValue());
		break;
	case FeedbackSource::TriPhasor:
		feedbackValue = bpFeedbackFilter.getSample(jbNyquistShannon.getUnipolarTrianglePhasorValue()*2.0-1.0);
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
