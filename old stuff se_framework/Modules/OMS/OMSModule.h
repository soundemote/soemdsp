#pragma once

#include <array>

/*
slider
https://regexr.com/41ns2
button
https://regexr.com/41nt3
combobox
https://regexr.com/41nu4
*/

using jura::ModulationConnection;

class JerobeamRadarModule : public OMSAlgorithm, public jura::ModulatableAudioModule, public MidiSlave
{
public:
	JerobeamRadarModule(CriticalSection* lockToUse
		, MetaParameterManager* metaManagerToUse = nullptr
		, ModulationManager* modManagerToUse = nullptr
		, MidiMaster * midiMasterPtr_ = nullptr);
	~JerobeamRadarModule() = default;

	void getStereoSample(double & outL, double & outR) override
	{
		radarCore.getSampleFrame(outL, outR);

		outL *= gain;
		outR *= gain;
	}

	void reset() override
	{
		radarCore.reset();
	}

	void setSampleRate(double v) override
	{
		radarCore.setSampleRate(v);
	}

	void setGain(double v)
	{
		gain = v;
	}

	void setFrequency(double v) override
	{
		radarCore.setFrequency(v);
	}

	Phasor* getPhasor() override
	{
		return &radarCore.phasor;
	}

	double getUnipolarTrianglePhasorValue() override
	{
		return radarCore.getUnipolarTrianglePhasorValue();
	}

	JerobeamRadar radarCore;

protected:	
	double gain = 1;

	void triggerMasterRateChange() override
	{
		getParameterByName("SubPhaseRotation")->callValueChangeCallbacks(getParameterByName("SubPhaseRotation")->getValue());
	}
};

class OMSModule
	: public BasicModule
	, public MidiSlave
	, public MouseKeyboard::Listener
	, public PresetInformation::SenderReceiver
{
  friend class OMSEditor;

public:
	OMSModule(CriticalSection* lockToUse, MetaParameterManager* metaManagerToUse = nullptr, ModulationManager* modulationManagerToUse = nullptr);

	~OMSModule() = default;

	// plugin
	jura::AudioModuleEditor* createEditor(int id = 0) override;

	// host
	void setSampleRate(double v) override;
	void setBeatsPerMinute(double newBpm) override;
	void setPlayheadInfo(AudioPlayHead::CurrentPositionInfo info) override;
	void processBlock(double** inOutBuffer, int numChannels, int numSamples) override;

	// midi
	void triggerAllNotesOff() override;
	void triggerAttack() override;
	void triggerNoteOnLegato() override;
	void triggerNoteOffLegato() override;
	void triggerRelease() override;

	MidiMaster MIDIMASTER;
	PresetInformation presetInformation;
	jura::ModulationManager modulationManager{ lock };
	OMSCoreOversampled * omsCore;

	// smoothing
	vector<jura::rsSmoothableParameter *> parametersForSmoothing;

	// CORE SYNTH MODULES
	JerobeamRadarModule* radarModule;
	//JerobeamNyquistModule* nyquistModule;
	//JerobeamSpiralModule* spiralModule;
	//JerobeamTorusModule* torusModule;
	//JerobeamMushroomModule* mushroomModule;

	// EFFECTS
	rosic::EqualizerStereo equalizer;
	jura::EqualizerAudioModule* equalizerModule = nullptr;
	ElanLimiterModule* limiterModule = nullptr;
	DelayModule* delayModule = nullptr;

	InternalModSrc modFeedback{ "Feedback Source", "Feedback Source", &omsCore->feedbackValue };
	InternalModSrc ampEnvOut{ "Amp Env", "Amp Env", &omsCore->ampEnvVal };

	OscilloscopeModule* oscilloscopeModule;
	PrettyScopeModule* psmod;	

	jura::Parameter* parSubPhaseRotation;
	jura::Parameter* parBP1Reset;
	jura::Parameter* parBP2Reset;

	XmlElement convertXmlStateIfNecessary(const XmlElement& xmlState) override
	{
		return xmlState;
	}

	void setStateFromXml(const juce::XmlElement& xmlState, const juce::String& stateName, bool markAsClean) override
	{
		// convert xml to string
		XmlElement* newXml;

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

			lpfValue = xmlState.getDoubleAttribute("Lowpass", omsCore->bandpassModule->parLPF.def);
			hpfValue = xmlState.getDoubleAttribute("Highpass", omsCore->bandpassModule->parHPF.def);

			omsCore->bandpassModule->parLPF.setValue(lpfValue);
			omsCore->bandpassModule->parHPF.setValue(hpfValue);
		}

		// convert string to xml
		newXml = stringToXml(xmlStr);

		// finally do all the standard stuff, no more preset format issues and conversions

		presetInformation.xmlToState(*newXml);

		BasicModule::setStateFromXml(*newXml, stateName, markAsClean);

		// get modulation connections
		XmlElement* modXml = newXml->getChildByName("Modulations");
		if (modXml != nullptr)
			modulationManager.setStateFromXml(*modXml);
		else
			modulationManager.removeAllConnections();

		delete newXml;

		sendChangeMessage();
	}

	XmlElement* getStateAsXml(const juce::String & stateName, bool markAsClean) override
	{
		XmlElement* state = AudioModuleWithMidiIn::getStateAsXml(stateName, markAsClean);

		state->addChildElement(presetInformation.stateToXml(lock));
		state->addChildElement(modulationManager.getStateAsXml());

		return state;
	}

	void handleMidiMessage(MidiMessage message)
	{
		ScopedLock scopedLock(*lock);

		MIDIMASTER.handleMidiMessage(message);
	}

	void keyStateChanged() override
	{
		if (getKeyboardState().isEventDown)
			MIDIMASTER.noteOn(getKeyboardState().note, getKeyboardState().velocity * 127);
		else
			MIDIMASTER.noteOff(getKeyboardState().note);
	}
	
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OMSModule)
};
