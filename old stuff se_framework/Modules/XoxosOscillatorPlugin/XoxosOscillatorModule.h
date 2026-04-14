#pragma once

class XoxosOscillatorModule : public BasicModule
{
  friend class XoxosOscillatorEditor;

public:
	XoxosOscillatorModule(juce::CriticalSection *lockToUse, jura::MetaParameterManager* metaManagerToUse);

	// plugin
	jura::AudioModuleEditor* createEditor(int type = 0) override;
	virtual void createParameters();

	// host
	void setSampleRate(double v) override;
	void setBeatsPerMinute(double newBpm) override;
	void processBlock(double** inOutBuffer, int numChannels, int numSamples) override;

	// midi
	void noteOn(int noteNumber, int velocity) override;
	void noteOff(int noteNumber) override;	

	myparams // common
		parGain;

	myparams // xoxos oscillator
		parFrequency,
		parAngle,
		parScale,
		parOffset;

protected:
	/* Core DSP objects */
	XoxosOscillatorCoreOversampled xoxosOscillatorCore;
	jura::BreakpointModulatorAudioModule * breakpointEnv1Module;

	/* Core Objects*/
	ParamManager paramManager;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(XoxosOscillatorModule)
};
