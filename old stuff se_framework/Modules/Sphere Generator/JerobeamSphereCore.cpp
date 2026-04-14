#include "JerobeamSphereCore.h"

const vector<String> JerobeamSphereCore::FeedbackSourceMenuItems{ "RampPhasor", "TriPhasor", "Left", "Right", "L+R", "L-R" };
const vector<String> JerobeamSphereCore::ClippingModeMenuItems{ "Hard", "Soft" };
const vector<String> JerobeamSphereCore::KeytrackingMenuItems{ "Keytracking: On", "Keytracking: Off" };
const vector<String> JerobeamSphereCore::ResetModeMenuItems{ "Always", "Legato", "When silent", "One-shot", "Never" };
const vector<String> JerobeamSphereCore::EnvResetModeMenuItems{ "Always", "On-note", "Legato", "Never" };

JerobeamSphereCore::JerobeamSphereCore(MidiMaster * MasterFrequencyToUse)
{
	harmonicSmoother.setSmootherType(ParamSmoother::type::LINEAR);
	octaveSmoother.setSmootherType(ParamSmoother::type::LINEAR);
}

void JerobeamSphereCore::setSampleRate(double v)
{
	jbSphere.setSampleRate(v);
	bpFeedbackFilter.setSampleRate(v);
	bpFilter.setSampleRate(v);
	ampEnv.setSampleRate(v);
}

void JerobeamSphereCore::triggerRelease()
{
	// one shot mode will handle release specially
	if (resetMode != ResetMode::one_shot)
		ampEnv.triggerRelease();
}

bool JerobeamSphereCore::isSilent()
{
	return ampEnv.isIdle() &&
		!harmonicSmoother.needsSmoothing() &&
		!octaveSmoother.needsSmoothing();
}

void JerobeamSphereCore::triggerAttack()
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
		ampEnv.triggerAttack(midiMasterPtr->getCurrentVelocityNormalized(), true);
		break;
	case EnvResetMode::env_never:
		ampEnv.triggerAttack(midiMasterPtr->getCurrentVelocityNormalized(), true);
		break;
	}
}

void JerobeamSphereCore::triggerNoteOnLegato()
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
		ampEnv.triggerAttack(midiMasterPtr->getCurrentVelocityNormalized(), false);
		break;
	case EnvResetMode::env_when_not_legato:
	case EnvResetMode::env_never:
		break;
	}
}

void JerobeamSphereCore::triggerNoteOffLegato()
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
		ampEnv.triggerAttack(midiMasterPtr->getCurrentVelocityNormalized(), false);
		break;
	case EnvResetMode::env_on_note:
	case EnvResetMode::env_when_not_legato:
	case EnvResetMode::env_never:
		break;
	}
}

void JerobeamSphereCore::reset()
{
	jbSphere.reset();
	phaseTimer = 0;
}

void JerobeamSphereCoreOversampled::processSampleFrame(double * outL, double * outR)
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
			JerobeamSphereCore::processSampleFrame(outL, outR);

			*outL *= internalGain;
			*outR *= internalGain;

			*outL += dc_x + dc_x_y;
			*outR += dc_y + dc_x_y;

			/*channel mode*/
			MonoSynth::handleChannelMode(outL, outR);

			/*rotate*/
			rotate2D(rotation_sin, rotation_cos, outL, outR);

			/*filter*/
			bpFilter.getSample(outL, outR);

			/*envelope*/
			ampEnvVal = ampEnv.getSample();

			/*gain*/
			double finalAmp = outputAmp * ampEnvVal;

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
		JerobeamSphereCore::processSampleFrame(outL, outR);

		*outL *= internalGain;
		*outR *= internalGain;

		*outL += dc_x + dc_x_y;
		*outR += dc_y + dc_x_y;

		/*channel mode*/
		MonoSynth::handleChannelMode(outL, outR);

		/*rotate*/
		rotate2D(rotation_sin, rotation_cos, outL, outR);

		/*filter*/
		bpFilter.getSample(outL, outR);

		/*envelope*/
		ampEnvVal = ampEnv.getSample();

		/*gain*/
		double finalAmp = outputAmp * ampEnvVal;

		*outL *= finalAmp;
		*outR *= finalAmp;

		/*clip*/
		clipper.getSample(outL, outR);
	}
}

void JerobeamSphereCore::processSampleFrame(double *outL, double *outR)
{
	handleOneShotMode(); // need double phase increment due to how mushroom generator calculates frequency

	jbSphere.getSampleFrame(&currentLeftChannelValue, &currentRightChannelValue);

	*outL = currentLeftChannelValue;
	*outR = currentRightChannelValue;

	/* Feedback handling */
	switch (feedbackSource)
	{
	case FeedbackSource::RampPhasor:
		feedbackValue = bpFeedbackFilter.getSample(jbSphere.phasor.getBipolarValue());
		break;
	case FeedbackSource::TriPhasor:
		feedbackValue = bpFeedbackFilter.getSample(jbSphere.getUnipolarTrianglePhasorValue()*2.0-1.0);
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

void JerobeamSphereCore::handleOneShotMode()
{
	if (resetMode != ResetMode::one_shot
		|| oneShotIsFinished
		|| ampEnv.getEnvState() == LinearADSR::EnvState::DELAY)
		return;

	if (phaseTimer >= 0.5)
	{
		oneShotIsFinished = true;
		ampEnv.triggerRelease();
		jbSphere.setFrequency(0);
		return;
	}

	phaseTimer += abs(jbSphere.phasor.getIncrementAmount());
}
