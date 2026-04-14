#pragma once

using jura::MetaParameterManager;
using jura::ModulationManager;
using jura::AudioModuleWithMidiIn;
using jura::AudioModuleEditor;

class MidiModSourceModule : public BasicModule, public jura::ModulationSource, public MidiSlave
{
  friend class MidiModSourceEditor;

public:
	MidiModSourceModule(CriticalSection * lockToUse, MetaParameterManager * metaManagerToUse, ModulationManager * modulationManager, MidiMaster * midiMasterToUse);
	~MidiModSourceModule() = default;

	// plugin
	//AudioModuleEditor * createEditor(int type = 0) override;
	void setupModulesAndModulation() override;
	void createParameters() override;
	void setupCallbacks() override;
	void setInitPatchValues() override;
	void setHelpTextAndLabels() override;

	// host
	void setSampleRate(double v) override { midiModSource.setSampleRate(sampleRate = v); }
	void processBlock(double **inOutBuffer, int numChannels, int numSamples) override;
	void processSampleFrame(double * left, double * right) override;

	// midi
	vector<String> MidiSrcSelMenuItems{ "Pitchwheel", "Modwheel", "Velocity", "ChannelPressure", "Keytracking", "MIDI CC" };
	enum MidiSrcSel { pitchwheel, modwheel, velocity, channelpressure, keytrack, cc };

	void triggerVelocityChange() override;
	void triggerNoteChange() override;
	void triggerPitchBendChange() override;
	void triggerChannelPressureChange() override;
	void triggerMidiControllerChange() override;

	// ModulationSource
	double renderModulation() override
	{
		processSampleFrame(&modValue, &modValue);
		return modValue;
	}
	void updateDisplayName() 
	{
		String newName = getModulationSourceName();
		switch ((MidiSrcSel)(int)parSource)
		{
		case MidiModSourceModule::pitchwheel:
			newName+=": Pitchwheel";
			break;
		case MidiModSourceModule::modwheel:
			newName+=": Modwheel";
			break;
		case MidiModSourceModule::cc:
			newName+=": CC"+String(parCCSelection);
			break;
		case MidiModSourceModule::velocity:
			newName+=": Velocity";
			break;
		case MidiModSourceModule::channelpressure:
			newName+=": Pressure";
			break;
		case MidiModSourceModule::keytrack:
			newName+=": Keytrack";
			break;
		}
		setModulationSourceDisplayName(newName);
	}

	myparams parSource{ &paramManager };
	myparams parCCSelection{ &paramManager };
	ParameterProfile::ExponentialNoSmoothing parSmoothing{ &paramManager };
	ParameterProfile::Linear parAmplitude{ &paramManager };
	ParameterProfile::Linear parOffset{ &paramManager };
	ParameterProfile::LinearQuantized parKeytrackCenter{ &paramManager };

	/* Core DSP objects */
	MidiModSource midiModSource;

protected:

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiModSourceModule)
};
