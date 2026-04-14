#pragma once

#include <array>

#include "JerobeamSpiralCore.h"
#include "se_framework/ElanSynthLib/SynthesizerComponents/utility/Oscilloscope.h"

class JerobeamSpiralModule : public BasicModule, public MidiSlave
{
  friend class JerobeamSpiralEditor;

public:
  JerobeamSpiralModule(juce::CriticalSection *lockToUse, jura::MetaParameterManager* metaManagerToUse);

	// plugin
	jura::AudioModuleEditor *createEditor(int type = 0) override;
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

		xmlStr = xmlStr.replace("Spiral_Generator", "SpiralGenerator");
		xmlStr = xmlStr.replace("AudioModule", "SpiralGenerator");

		if (PatchFormat == 1)
		{
			double lpfValue;
			double hpfValue;

			// for modulation and I think metaparameter stuff
			xmlStr = xmlStr.replace("SpiralGenerator.Lowpass", "SpiralGenerator.Filter.Lowpass");
			xmlStr = xmlStr.replace("SpiralGenerator.Highpass", "SpiralGenerator.Filter.Highpass");

			lpfValue = xmlState.getDoubleAttribute("Lowpass", jbSpiralCore.bandpassModule->parLPF.def);
			hpfValue = xmlState.getDoubleAttribute("Highpass", jbSpiralCore.bandpassModule->parHPF.def);

			jbSpiralCore.bandpassModule->parLPF.setValue(lpfValue);
			jbSpiralCore.bandpassModule->parHPF.setValue(hpfValue);
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

	/* SPIRAL */
	ParameterProfile::BipolarAmplitude parCoreGain{ &paramManager };
	ParameterProfile::BipolarPhase parPhase{ &paramManager };
	ParameterProfile::Exponential parDensity{ &paramManager };
	myparams parQuantizeDensity{ &paramManager };
	ParameterProfile::Linear parPosition{ &paramManager };
	ParameterProfile::BipolarFineFrequency parPositionSpeed{ &paramManager };
	ParameterProfile::Exponential parSize{ &paramManager };
	ParameterProfile::WaveSharpness parSharp{ &paramManager };
	ParameterProfile::Linear parSharpCurve{ &paramManager };
	ParameterProfile::Linear parSharpCrvMult{ &paramManager };
	ParameterProfile::Linear parMorph{ &paramManager };
	ParameterProfile::BipolarFineFrequency parMorphSpeed{ &paramManager };
	ParameterProfile::BipolarPhase parRotX{ &paramManager };
	ParameterProfile::BipolarFineFrequency parRotXSpeed{ &paramManager };
	ParameterProfile::BipolarPhase parRotY{ &paramManager };
	ParameterProfile::BipolarFineFrequency parRotYSpeed{ &paramManager };
	ParameterProfile::Linear parZDarkness{ &paramManager };
	ParameterProfile::Linear parZDepth{ &paramManager };
	ParameterProfile::LowpassFilterFrequency parZDarknessLPF{ &paramManager };
	ParameterProfile::HighpassFilterFrequency parZDarknessHPF{ &paramManager };

	/* Core DSP objects */
	MidiMaster MIDIMASTER;
	ModulationManager modulationManager{ lock };
	JerobeamSpiralCoreOversampled jbSpiralCore{ &MIDIMASTER };

	vector<BasicEnvelopeModule *> BasicEnvelopeModuleArray{ 6 };
	vector<BasicOscillatorModule *> BasicOscillatorModuleArray{ 8 };
	vector<MidiModSourceModule *> MidiModSourceModuleArray{ 12 };
	jura::BreakpointModulatorAudioModule * Breakpoint1Module, *Breakpoint2Module;

	InternalModSrc modFeedback{ "Feedback Source", "Feedback Source", &jbSpiralCore.feedbackValue };
	InternalModSrc ampEnvOut{ "Amp Env", "Amp Env", &jbSpiralCore.ampEnvVal };

	rosic::EqualizerStereo equalizer;
	jura::EqualizerAudioModule * equalizerModule;

	ElanLimiterModule * limiterModule = nullptr;
	DelayModule * delayModule = nullptr;
	OscilloscopeModule * oscilloscopeModule;
	
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(JerobeamSpiralModule)
};
