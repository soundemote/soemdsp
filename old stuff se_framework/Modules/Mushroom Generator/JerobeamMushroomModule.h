#pragma once

#include <array>

#include "JerobeamMushroomCore.h"
#include "../../Source/BasicEnvelopePlugin/BasicEnvelopeModule.h"
#include "../../Source/BasicOscillatorPlugin/BasicOscillatorModule.h"
#include "../../Source/MidiModSourcePlugin/MidiModSourceModule.h"
#include "se_framework/ElanSynthLib/SynthesizerComponents/utility/Oscilloscope.h"

class JerobeamMushroomModule : public BasicModule, public MidiSlave
{
  friend class JerobeamMushroomEditor;

public:
  JerobeamMushroomModule(juce::CriticalSection *lockToUse, jura::MetaParameterManager* metaManagerToUse);
	virtual ~JerobeamMushroomModule() = default;

	// plugin
	jura::AudioModuleEditor *createEditor(int type = 0) override;
	void setupModulesAndModulation() override;
	void createParameters() override;
	void setupCallbacks() override;
	void setInitPatchValues() override;
	void setHelpTextAndLabels() override;

	MidiMaster MIDIMASTER;
	ModulationManager modulationManager{ lock };
	JerobeamMushroomCoreOversampled jbMushroomCore{ &MIDIMASTER };

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

		xmlStr = xmlStr.replace("Mushroom_Generator", "MushroomGenerator");
		xmlStr = xmlStr.replace("AudioModule", "MushroomGenerator");

		if (PatchFormat == 1)
		{
			double lpfValue;
			double hpfValue;

			// for modulation and I think metaparameter stuff
			xmlStr = xmlStr.replace("MushroomGenerator.Lowpass", "MushroomGenerator.Filter.Lowpass");
			xmlStr = xmlStr.replace("MushroomGenerator.Highpass", "MushroomGenerator.Filter.Highpass");

			// get old formatted lowpass highpass values and apply
			lpfValue = xmlState.getDoubleAttribute("Lowpass", jbMushroomCore.bandpassModule->parLPF.def);
			hpfValue = xmlState.getDoubleAttribute("Highpass", jbMushroomCore.bandpassModule->parHPF.def);

			jbMushroomCore.bandpassModule->parLPF.setValue(lpfValue);
			jbMushroomCore.bandpassModule->parHPF.setValue(hpfValue);
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

	XmlElement * getStateAsXml(const juce::String & stateName, bool markAsClean) override
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
	myparams parClippingLevel{ &paramManager };
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

	/* MUSHROOM */
	ParameterProfile::BipolarPhase parPhaseOffset{ &paramManager };
	ParameterProfile::BipolarAmplitude parCoreGain{ &paramManager };
	ParameterProfile::Linear parGrow{ &paramManager };
	ParameterProfile::WaveDensity parDensity{ &paramManager };
	ParameterProfile::Linear parCapStemRatio{ &paramManager };
	ParameterProfile::Linear parCapStemTransition{ &paramManager };
	ParameterProfile::Linear parClusterSpread{ &paramManager };
	ParameterProfile::Linear parStemTwist{ &paramManager };
	ParameterProfile::WaveSharpness parSharp{ &paramManager };
	ParameterProfile::LinearQuantized parNumMushrooms{ &paramManager };
	ParameterProfile::Linear parWidth{ &paramManager };
	ParameterProfile::Linear parStemWidth{ &paramManager };
	ParameterProfile::Linear parMirror{ &paramManager };
	ParameterProfile::Linear parCapStemRotation{ &paramManager };
	ParameterProfile::Linear parClusterRotation{ &paramManager };
	ParameterProfile::BipolarFineFrequency parCapStemRotSpd{ &paramManager };
	ParameterProfile::BipolarFineFrequency parClusterRotSpd{ &paramManager };

	/* Core DSP objects */
	vector<BasicEnvelopeModule *> BasicEnvelopeModuleArray{ 6 };
	vector<BasicOscillatorModule *> BasicOscillatorModuleArray{ 8 };
	vector<MidiModSourceModule *> MidiModSourceModuleArray{ 12 };
	jura::BreakpointModulatorAudioModule * Breakpoint1Module, *Breakpoint2Module;

	InternalModSrc modFeedback{ "Feedback Source", "Feedback Source", &jbMushroomCore.feedbackValue };
	InternalModSrc ampEnvOut{ "Amp Env", "Amp Env", &jbMushroomCore.ampEnvVal };

	rosic::EqualizerStereo equalizer;
	jura::EqualizerAudioModule * equalizerModule;

	ElanLimiterModule * limiterModule = nullptr;
	DelayModule * delayModule = nullptr;
	OscilloscopeModule * oscilloscopeModule;
	
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(JerobeamMushroomModule)
};
