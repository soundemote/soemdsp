#pragma once

using namespace se;

class ChaosflyModule : public BasicModule, public MidiSlave
{
  friend class ChaosflyEditor;

public:
	ChaosflyModule(juce::CriticalSection * lockToUse, jura::MetaParameterManager * metaManagerToUse); 

	// plugin
  AudioModuleEditor *createEditor(int type = 0) override;

	void setupModulesAndModulation() override;
	void createParameters() override;
	void setStateFromXml(const juce::XmlElement& xmlState, const juce::String& stateName, bool markAsClean) override;

	// host
  void setSampleRate(double newSampleRate) override; 
	void setPlayheadInfo(AudioPlayHead::CurrentPositionInfo /*info*/) override {}
	void processBlock(double **inOutBuffer, int numChannels, int numSamples) override;

	void handleMidiMessage(MidiMessage message)
	{
		ScopedLock scopedLock(*lock);

		MIDIMASTER.handleMidiMessage(message);
	}

	// midi
	void allNotesOff() override;
	void setPitchBend(int /*chanel*/, int /*value*/) {}
	void setChannelPressure(int /*channel*/, int /*value*/) {}
	void setMidiController(int /*channel*/, int /*controller*/, int /*value*/) {}

	void triggerPitchChange() override;
	void triggerAttack() override;
	void triggerNoteOnLegato() override;
	void triggerNoteOffLegato() override;
	void triggerRelease() override;

	double outputGain;
	double outputGain2;
	double hostSampleRate;
	 
	vector<String> filterModes =
	{
		"Flat",
		"Lowpass 6 dB/oct",
		"Lowpass 12 dB/oct",
		"Lowpass 18 dB/oct",
		"Lowpass 24 dB/oct",
		"Highpass 6 dB/oct",
		"Highpass 12 dB/oct",
		"Highpass 18 dB/oct",
		"Highpass 24 dB/oct",
		"Bandpass 6/6 dB/oct",
		"Bandpass 6/12 dB/oct",
		"Bandpass 6/18 dB/oct",
		"Bandpass 12/6 dB/oct",
		"Bandpass 12/12 dB/oct",
		"Bandpass 18/6 dB/oct"
	};

	vector<String> inputOscSchapes = { "Sine", "Triangle", "Square", "SawUp", "SawDown" };
	vector<String> oscillatorShapes = { "Sine", "Triangle", "Square", "SawUp", "SawDown", "Ellipse" };
	vector<String> syncModes = { "Off", "Reset", "Jump", "Reverse" };
	vector<String> duckShapes = { "Linear", "Cubic", "Quintic", "Heptic" };
	vector<String> channelModes = { "Stereo", "Mono", "Osc1", "Osc2" };

protected:

	/* Core DSP objects */
	MidiMaster MIDIMASTER;
	jura::ModulationManager modulationManager;
	ChaosflyCoreOversampled chaosfly;
	BreakpointModulatorAudioModule *modEnvModule, *ampEnvModule;

	jura::rsSmoothingManager smootyMan;

	InternalModSrc inOscOutput{"input_oscillator_source", "input oscillator source"};

	vector<ScopedPointer<BreakpointModulator>> BreakpointModulatorArray{ 10 };
	vector<BreakpointModulatorAudioModule*> BreakpointModulatorAudioModuleArray{ 10 };

	enum class ResetMode { Never, Always, Legato, numModes };
	enum class TrigMode { Always, Legato, numModes };

	ResetMode modEnvResetMode = ResetMode::Always;
	ResetMode ampEnvResetMode = ResetMode::Always;

	TrigMode modEnvTrigMode = TrigMode::Always;
	TrigMode ampEnvTrigMode = TrigMode::Always;

	ResetMode osc1ResetMode = ResetMode::Always;
	ResetMode osc2ResetMode = ResetMode::Always;

  juce::String patchComments; // a string to take notes about the patch

	vector<jura::ModulatableParameter2 *> parametersForSmoothing;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChaosflyModule)
};
