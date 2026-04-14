#pragma once

#include <array>

#include "JerobeamSphereCore.h"
#include "../../Source/BasicEnvelopePlugin/BasicEnvelopeModule.h"
#include "../../Source/BasicOscillatorPlugin/BasicOscillatorModule.h"
#include "../../Source/MidiModSourcePlugin/MidiModSourceModule.h"

class JerobeamSphereModule : public BasicMidiPlugin
{
  friend class JerobeamSphereEditor;

public:
	juce::String presetPath;
	juce::String moduleVendor = "Soundemote";

  JerobeamSphereModule(juce::CriticalSection *lockToUse, jura::MetaParameterManager* metaManagerToUse);

	// plugin
	jura::AudioModuleEditor *createEditor() override;
	void setupModulesAndModulation(jura::MetaParameterManager * metaManagerToUse);
	void createParameters() override;
	void setupCallbacks() override;
	void setInitPatchValues() override;
	void setHelpTextAndLabels() override;

	// host
	void setSampleRate(double v) override;
	void setBeatsPerMinute(double newBpm) override;
	void setPlayheadInfo(AudioPlayHead::CurrentPositionInfo info) override;
	void processBlock(double **inOutBuffer, int numChannels, int numSamples) override;

	// midi
	void allNotesOff() override;
	void setPitchBend(int pitchBendValue) override;
	void setChannelPressure(int channelPressureValue) override;
	void setMidiController(int controllerNumber, int controllerValue) override;

	void triggerAttack() override;
	void triggerNoteOnLegato() override;
	void triggerNoteOffLegato() override;
	void triggerRelease() override;

	myparams parChannelMode{ &paramManager };
	myparams parClippingMode{ &paramManager };
	myparams parClippingLevel{ &paramManager };
	ParameterProfile::Gain parGain{ &paramManager };
	myparams parStereoRotate{ &paramManager };
	ParameterProfile::BipolarAmplitude parDC{ &paramManager };
	ParameterProfile::BipolarAmplitude parX{ &paramManager };
	ParameterProfile::BipolarAmplitude parY{ &paramManager };
	ParameterProfile::HighpassFilterFrequency parLPF{ &paramManager };
	ParameterProfile::LowpassFilterFrequency parHPF{ &paramManager };
	myparams parBP1Reset{ &paramManager };
	myparams parBP2Reset{ &paramManager };
	myparams parResetMode{ &paramManager };
	myparams parNoteChangeMode{ &paramManager };
	myparams parResetWhenBeatChanges{ &paramManager };
	myparams parFreqMode{ &paramManager };
	myparams parGlideMode{ &paramManager };
	ParameterProfile::ClipLevel parClip{ &paramManager };
	ParameterProfile::BipolarCoarseFrequency parBeatFreq{ &paramManager };
	ParameterProfile::LinearQuantized parBeatTempo{ &paramManager };
	ParameterProfile::LinearQuantized parBeatMult{ &paramManager };
	ParameterProfile::Semitone parTune{ &paramManager };
	ParameterProfile::Octave parOctave{ &paramManager };
	ParameterProfile::BipolarRate parFreqMult{ &paramManager };
	ParameterProfile::Harmonic parHarmonicMult{ &paramManager };
	ParameterProfile::Exponential parGlideAmt{ &paramManager };
	ParameterProfile::Exponential parHarmOctGlideAmt{ &paramManager };
	ParameterProfile::Oversampling parOversampling{ &paramManager };
	ParameterProfile::LowpassFilterFrequency parFeedbackLPF{ &paramManager };
	ParameterProfile::HighpassFilterFrequency parFeedbackHPF{ &paramManager };
	ParameterProfile::ExponentialNoSmoothing parSmoothing{ &paramManager };
	myparams parFeedbackSrc{ &paramManager };
	ParameterProfile::FeedbackAmount parFeedbackAmp{ &paramManager };

	/* ENVELOPE */
	myparams parEnvResetMode{ &paramManager };
	ParameterProfile::ExponentialNoSmoothing parEnvGlobalTime{ &paramManager };
	ParameterProfile::Linear parEnvSustain{ &paramManager };
	ParameterProfile::Linear parEnvAttackFB{ &paramManager };
	ParameterProfile::Linear parEnvDecayFB{ &paramManager };
	ParameterProfile::Linear parEnvReleaseFB{ &paramManager };
	ParameterProfile::Exponential parEnvGlobalFB{ &paramManager };
	ParameterProfile::Linear parEnvAttackShape{ &paramManager };
	ParameterProfile::Linear parEnvDecayShape{ &paramManager };
	ParameterProfile::Linear parEnvReleaseShape{ &paramManager };
	ParameterProfile::VelocityInfluence parEnvVelInfluence{ &paramManager };

	ParameterProfile::BipolarAmplitude parCoreGain{ &paramManager };
	ParameterProfile::BipolarPhase parPhase{ &paramManager };
	/* SPHERE */
	ParameterProfile::Linear parDensity{ &paramManager };
	ParameterProfile::Linear parSharpness{ &paramManager };
	ParameterProfile::Linear parRotX{ &paramManager };
	ParameterProfile::Linear parRotY{ &paramManager };
	ParameterProfile::Linear parZDepth{ &paramManager };
	ParameterProfile::Linear parZAmount{ &paramManager };
	ParameterProfile::Linear parEnds{ &paramManager };
	ParameterProfile::Linear parBoing{ &paramManager };
	ParameterProfile::Linear parJump{ &paramManager };
	ParameterProfile::Linear parDirection{ &paramManager };
	ParameterProfile::Linear parShape{ &paramManager };
	ParameterProfile::Linear parVolume{ &paramManager };
	myparams parVolumePreJump{ &paramManager };

protected:

	/* Core DSP objects */
	JerobeamSphereCoreOversampled jbSphereCore;
	vector<BasicEnvelopeModule *> BasicEnvelopeModuleArray{ 6 };
	vector<BasicOscillatorModule *> BasicOscillatorModuleArray{ 8 };
	vector<MidiModSourceModule *> MidiModSourceModuleArray{ 12 };
	jura::BreakpointModulatorAudioModule * Breakpoint1Module, *Breakpoint2Module;

	InternalModSrc modFeedback{ "Feedback Source",  &jbSphereCore.feedbackValue };
	InternalModSrc ampEnvOut{ "Amp Env",  &jbSphereCore.ampEnvVal };

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(JerobeamSphereModule)
};
