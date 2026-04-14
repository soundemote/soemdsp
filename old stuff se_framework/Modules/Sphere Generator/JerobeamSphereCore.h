#pragma once

#include "se_framework/ElanSynthLib/ElanSynthLib.h"
#include "se_framework/ElanSynthLib/myparams.h"

class JerobeamSphereCore : public MonoSynth, public MidiSlave
{
public:
	enum ResetMode { on_note, when_not_legato, when_silent, one_shot, never };
	enum EnvResetMode { env_always, env_on_note, env_when_not_legato, env_never };
	enum KeytrackingMode { On, Off };

	static const vector<String> ResetModeMenuItems;
	static const vector<String> EnvResetModeMenuItems;
	static const vector<String> KeytrackingMenuItems;

	JerobeamSphereCore(MidiMaster * masterFrequencyToUse);
	virtual ~JerobeamSphereCore() = default;

	/* Mono Synth */
	void setSampleRate(double v);
	void processSampleFrame(double *outL, double *outR) override;

	void triggerAttack() override;
	void triggerNoteOnLegato() override;
	void triggerNoteOffLegato() override;
	void triggerRelease() override;

	void triggerMasterRateChange() override { updateFrequency(); }
	void triggerPitchChange() override { calculatePitchToFrequency(); }

	bool isSilent() override;
	void reset();

	/* triggering */
	void setKeytrackingMode(KeytrackingMode v) { keytrackingMode = v; }
	void setResetMode(int v)
	{
		resetMode = (ResetMode)v;

		//triggerFrequencyChange();
	}
	void setAmpEnvResetMode(EnvResetMode v)
	{
		ampEnvResetMode = v;
	}

	/* Tempo */
	void setTempoSelection(int v) { tempoSelection = v; calculateBeatFrequency(); }
	void setTempoMultiplier(double v) { tempoMultiplier = v; calculateBeatFrequency(); }

	/* Pitch Glide Handling */
	void setSemitoneOffset(double v) { semitoneOffset = v; calculatePitchToFrequency(); }
	void setOctaveOffset(double v) { octaveSmoother.setTargetValue(round(v) * 12); calculatePitchToFrequency(); }
	void setHarmonicMultiplier(double v) { harmonicSmoother.setTargetValue(round(v)); calculatePitchToFrequency(); }
	void setFrequencyOffset(double v) { frequencyOffset = v; updateFrequency(); }
	void setHarmAndOctGlideAmt(double v)
	{
		harmonicSmoother.setSmoothingTime(v);
		octaveSmoother.setSmoothingTime(v);
	}

	/* Core Objects */
	JerobeamBoing jbSphere;
	FeedbackADSR ampEnv;
	double ampEnvVal;
	StereoTwoPoleBandpass bpFilter;

	/* Feedback */
	enum FeedbackSource { RampPhasor, TriPhasor, Left, Right, L_p_R, L_m_R };
	static const vector<String> FeedbackSourceMenuItems;
	void setFeedbackAmp(double v) { feedbackAmp = v; }
	void setFeedbackSource(int v) { feedbackSource = (FeedbackSource)v; }
	void setFeedbackHPCutoff(double v) { bpFeedbackFilter.setHighpassCutoff(v); }
	void setFeedbackLPCutoff(double v) { bpFeedbackFilter.setLowpassCutoff(v); }
	double feedbackValue = 0.0; // updated per sample, use this to send feedback to various parameters

	/* One shot */
	void handleOneShotMode();

	/* Rotation */
	// -1 to +1
	void setRotation(double v) { sinCos(valueToRotation(-v), &rotation_sin, &rotation_cos); }

	/* DC */
	void setInternalGain(double v) { internalGain  = v; }
	void setDCOffset(double v) { dc_x_y = v; }
	void setXOffset(double v) { dc_x = v; }
	void setYOffset(double v) { dc_y = v; }

	/* Clipping */
	static const vector<String> ClippingModeMenuItems;
	void setClipMode(int v) { clipper.setClipMode(v); }
	void setClipLevel(double v) { clipper.setClipLevel(v); }

protected:

	double currentLeftChannelValue = 0, currentRightChannelValue = 0;
	double currentFrequency;
	double phaseTimer = 0;

	int tempoSelection = 0;
	double tempoMultiplier = 1;

	bool oneShotIsFinished = true;

	void calculateBeatFrequency()
	{
		currentBeatFreq = midiMasterPtr->getBPMFactor() * MidiMaster::TempoMultipliers[tempoSelection] * powerOfTwo((int)tempoMultiplier);
		calculatePitchToFrequency();
	}

	void calculatePitchToFrequency()
	{
		if (keytrackingMode == KeytrackingMode::On)
			currentFrequency = rosic::pitchToFreq((midiMasterPtr->getCurrentNoteSmoothed() + semitoneOffset) + octaveSmoother.getCurrentValue()) * harmonicSmoother.getCurrentValue();
		else
			currentFrequency = 0;

		updateFrequency();
	}

	void updateFrequency()
	{
		jbSphere.setFrequency((currentFrequency + currentBeatFreq + frequencyOffset) * midiMasterPtr->getMasterRate());
	}

	/* options */
	ResetMode resetMode = ResetMode::never;
	EnvResetMode ampEnvResetMode = EnvResetMode::env_never;
	KeytrackingMode keytrackingMode = KeytrackingMode::On;

	/* feedback */
	MonoTwoPoleBandpass bpFeedbackFilter;
	FeedbackSource feedbackSource = FeedbackSource::RampPhasor;
	double feedbackAmp;

	/* pitch */
	double semitoneOffset = 0;
	double currentPitchToFreq = 1;
	double currentBeatFreq = 0;
	double frequencyOffset = 0;

	ParamSmoother harmonicSmoother{ 1 };
	ParamSmoother octaveSmoother;

	/* rotation */
	double
		rotation_sin = 0,
		rotation_cos = 1;

	/* dc */
	double
		internalGain = 1,
		dc_x_y = 0,
		dc_x = 0,
		dc_y = 0;

	/* clipping */
	StereoClipper clipper;
};

class JerobeamSphereCoreOversampled : public JerobeamSphereCore
{
public:
	JerobeamSphereCoreOversampled(MidiMaster * MidiFrequencyMasterToUse)
		: JerobeamSphereCore(MidiFrequencyMasterToUse)
	{
	}

	virtual ~JerobeamSphereCoreOversampled() = default;

	void setOversampling(int v)
	{
		oversampling = v;
		JerobeamSphereCore::setSampleRate(oversampling * hostSampleRate);

		harmonicSmoother.setSampleRate(hostSampleRate);
		octaveSmoother.setSampleRate(hostSampleRate);

		antiAliasFilterL.setSubDivision(oversampling);
		antiAliasFilterR.setSubDivision(oversampling);
	}

	void setSampleRate(double v)
	{
		hostSampleRate = v;
		JerobeamSphereCore::setSampleRate(oversampling * hostSampleRate);
	}

	void processSampleFrame(double *outL, double *outR) override;
protected:
	double hostSampleRate = 44100.0;
	double oversampling = 1.0;
	rosic::rsEllipticSubBandFilter antiAliasFilterL, antiAliasFilterR;
};
