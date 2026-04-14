#pragma once

#include <array>

#include "JerobeamRadarCore.h"
#include "../../Source/BasicEnvelopePlugin/BasicEnvelopeModule.h"
#include "../../Source/BasicOscillatorPlugin/BasicOscillatorModule.h"
#include "../../Source/MidiModSourcePlugin/MidiModSourceModule.h"
#include "se_framework/ElanSynthLib/SynthesizerComponents/utility/Oscilloscope.h"

class JerobeamRadarModule : public BasicModule, public MidiSlave
{
  friend class JerobeamRadarEditor;

public:
  JerobeamRadarModule(juce::CriticalSection *lockToUse, jura::MetaParameterManager* metaManagerToUse);

	// plugin
	jura::AudioModuleEditor * createEditor(int id = 0) override;
	void setupModulesAndModulation() override;
	void createParameters() override;
	void setupCallbacks() override;
	void setInitPatchValues() override;
	void setHelpTextAndLabels() override;

	void handleMidiMessage(MidiMessage message)
	{
		ScopedLock scopedLock(*lock);

		MIDIMASTER.handleMidiMessage(message);
	}

	XmlElement convertXmlStateIfNecessary(const XmlElement & xmlState) override
	{
		return xmlState;
	}

	void setStateFromXml(const juce::XmlElement& xmlState, const juce::String& stateName, bool markAsClean) override
	{
		// convert xml to string
		XmlElement * newXml;
		String xmlStr = xmlState.createDocument({}, false, false);

		int PatchFormat = xmlStr.contains("<Filter PatchFormat=") ? 2 : 1;

		xmlStr = xmlStr.replace("Radar_Generator", "RadarGenerator");
		xmlStr = xmlStr.replace("AudioModule", "RadarGenerator");

		if (PatchFormat == 1)
		{
			double lpfValue;
			double hpfValue;

			// for modulation and I think metaparameter stuff
			xmlStr = xmlStr.replace("RadarGenerator.Lowpass", "RadarGenerator.Filter.Lowpass");
			xmlStr = xmlStr.replace("RadarGenerator.Highpass", "RadarGenerator.Filter.Highpass");

			lpfValue = xmlState.getDoubleAttribute("Lowpass", jbRadarCore.bandpassModule->parLPF.def);
			hpfValue = xmlState.getDoubleAttribute("Highpass", jbRadarCore.bandpassModule->parHPF.def);

			jbRadarCore.bandpassModule->parLPF.setValue(lpfValue);
			jbRadarCore.bandpassModule->parHPF.setValue(hpfValue);
		}

		// convert string to xml
		newXml = stringToXml(xmlStr);

		// finally do all the standard stuff, no more preset format issues and conversions
		BasicModule::setStateFromXml(*newXml, stateName, markAsClean);

		// get modulation connections
		XmlElement * modXml = newXml->getChildByName("Modulations");
		if (modXml != nullptr)
			modulationManager.setStateFromXml(*modXml);
		else
			modulationManager.removeAllConnections();

		delete newXml;

		sendChangeMessage();
	}

	XmlElement * getStateAsXml(const juce::String & stateName, bool markAsClean)
	{
		XmlElement* state = AudioModuleWithMidiIn::getStateAsXml(stateName, markAsClean);

		state->addChildElement(modulationManager.getStateAsXml());

		return state;
	}

	// host
	void setSampleRate(double v) override;
	void setBeatsPerMinute(double newBpm) override;
	void setPlayheadInfo(AudioPlayHead::CurrentPositionInfo info) override;
	void processBlock(double **inOutBuffer, int numChannels, int numSamples) override;

	// midi
	void triggerAllNotesOff() override;
	void triggerAttack() override;
	void triggerNoteOnLegato() override;
	void triggerNoteOffLegato() override;
	void triggerRelease() override;

	myparams parChannelMode{ &paramManager };
	myparams parClippingMode{ &paramManager };
	ParameterProfile::ClipLevel parClippingLevel{ &paramManager };
	ParameterProfile::Gain parGain{ &paramManager };
	myparams parStereoRotate{ &paramManager };
	ParameterProfile::BipolarAmplitude parDC{ &paramManager };
	ParameterProfile::BipolarAmplitude parX{ &paramManager };
	ParameterProfile::BipolarAmplitude parY{ &paramManager };
	myparams parBP1Reset{ &paramManager };
	myparams parBP2Reset{ &paramManager };
	myparams parResetMode{ &paramManager };
	myparams parNoteChangeMode{ &paramManager };
	myparams parResetWhenBeatChanges{ &paramManager };
	myparams parFreqMode{ &paramManager };
	myparams parGlideMode{ &paramManager };
	ParameterProfile::ClipLevel parClip{ &paramManager };
	ParameterProfile::BipolarCoarseFrequency parBeatFreq{ &paramManager };
	ParameterProfile::Tempo parBeatTempo{ &paramManager };
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
	/* RADAR */
	ParameterProfile::Linear parDensity{ &paramManager };
	ParameterProfile::WaveSharpness parSharp{ &paramManager };
	ParameterProfile::Linear parSharpCurve{ &paramManager };
	ParameterProfile::Linear parSubPhase{ &paramManager };
	ParameterProfile::BipolarFineFrequency parSubPhaseRotation{ &paramManager };
	ParameterProfile::Linear parDirection{ &paramManager };
	ParameterProfile::Linear parShade{ &paramManager };
	ParameterProfile::Linear parLap{ &paramManager };
	myparams parRingCut{ &paramManager };
	myparams parCurveStart{ &paramManager };
	myparams parCurveEnd{ &paramManager };
	myparams parPow2Bend{ &paramManager };
	myparams parPhaseInv{ &paramManager };
	myparams parTunnelInv{ &paramManager };
	myparams parSpiralReturn{ &paramManager };
	ParameterProfile::Linear parLength{ &paramManager };
	ParameterProfile::Linear parRatio{ &paramManager };
	ParameterProfile::Linear parFrontRing{ &paramManager };
	ParameterProfile::Linear parMorph{ &paramManager };
	ParameterProfile::Linear parZDepth{ &paramManager };
	ParameterProfile::Linear parInner{ &paramManager };
	ParameterProfile::Linear parProtrudeX{ &paramManager };
	ParameterProfile::Linear parProtrudeY{ &paramManager };

	/* Core DSP objects */
	MidiMaster MIDIMASTER;
	jura::ModulationManager modulationManager{ lock };
	JerobeamRadarCoreOversampled jbRadarCore{ &MIDIMASTER };
	vector<BasicEnvelopeModule *> BasicEnvelopeModuleArray{ 6 };
	vector<BasicOscillatorModule *> BasicOscillatorModuleArray{ 8 };
	vector<MidiModSourceModule *> MidiModSourceModuleArray{ 12 };
	jura::BreakpointModulatorAudioModule * Breakpoint1Module, *Breakpoint2Module;

	InternalModSrc modFeedback{ "Feedback Source", "Feedback Source", &jbRadarCore.feedbackValue };
	InternalModSrc ampEnvOut{ "Amp Env", "Amp Env", &jbRadarCore.ampEnvVal };

	rosic::EqualizerStereo equalizer;
	jura::EqualizerAudioModule * equalizerModule;

	ElanLimiterModule * limiterModule = nullptr;
	DelayModule * delayModule = nullptr;
	OscilloscopeModule * oscilloscopeModule;
	
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(JerobeamRadarModule)
};
