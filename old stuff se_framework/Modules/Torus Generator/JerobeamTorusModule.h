#pragma once

#include <array>

class JerobeamTorusModule : public BasicModule, public MidiSlave
{
  friend class JerobeamTorusEditor;

public:
  JerobeamTorusModule(juce::CriticalSection *lockToUse, jura::MetaParameterManager* metaManagerToUse = nullptr);

	// plugin
	jura::AudioModuleEditor *createEditor(int type = 0) override;

	MidiMaster MIDIMASTER;
	ModulationManager modulationManager{ lock };
	JerobeamTorusCoreOversampled jbTorusCore{ &MIDIMASTER };

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

		xmlStr = xmlStr.replace("Torus_Generator", "TorusGenerator");
		xmlStr = xmlStr.replace("AudioModule", "TorusGenerator");

		if (PatchFormat == 1)
		{
			double lpfValue;
			double hpfValue;

			// for modulation and I think metaparameter stuff
			xmlStr = xmlStr.replace("TorusGenerator.Lowpass", "TorusGenerator.Filter.Lowpass");
			xmlStr = xmlStr.replace("TorusGenerator.Highpass", "TorusGenerator.Filter.Highpass");

			lpfValue = xmlState.getDoubleAttribute("Lowpass", jbTorusCore.bandpassModule->parLPF.def);
			hpfValue = xmlState.getDoubleAttribute("Highpass", jbTorusCore.bandpassModule->parHPF.def);

			jbTorusCore.bandpassModule->parLPF.setValue(lpfValue);
			jbTorusCore.bandpassModule->parHPF.setValue(hpfValue);
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

	/* Core DSP objects */

	InternalModSrc modFeedback{ "Feedback Source", "Feedback Source", &jbTorusCore.feedbackValue };
	InternalModSrc ampEnvOut{ "Amp Env", "Amp Env", &jbTorusCore.ampEnvVal };
	
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(JerobeamTorusModule)
};
