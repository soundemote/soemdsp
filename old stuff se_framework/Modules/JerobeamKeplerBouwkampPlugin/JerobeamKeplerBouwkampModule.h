#pragma once

#include "JerobeamKeplerBouwkampCore.h"
#include "se_framework/ElanSynthLib/myparams.h"

class JerobeamKeplerBouwkampModule : public jura::AudioModuleWithMidiIn
{
  friend class JerobeamKeplerBouwkampEditor;

public:
	juce::String presetPath;
	juce::String moduleVendor = "Soundemote";

  JerobeamKeplerBouwkampModule(juce::CriticalSection *lockToUse, jura::MetaParameterManager* metaManagerToUse);

  virtual void createParameters();

	std::vector<myparams *> paramStrIds;

	myparams
		parGain,
		parClip,
		parTune,
		parOctave,
		parGlide,
		parOversampling,
		parOffX,
		parOffY,
		parLPF,
		parHPF;

	myparams
		parStart,
		parLength,
		parCircles,
		parZoom,
		parRotation,
		parTri;

	jura::AudioModuleEditor *createEditor() override;
  void processBlock(double **inOutBuffer, int numChannels, int numSamples) override;
  void setSampleRate(double newSampleRate) override; 
  void noteOn(int noteNumber, int velocity) override;
  void noteOff(int noteNumber) override;
  void setStateFromXml(const juce::XmlElement& xmlState, const juce::String& stateName, 
    bool markAsClean) override;
  juce::XmlElement* getStateAsXml(const juce::String& stateName, bool markAsClean) override;
	void setBeatsPerMinute(double newBpm) override;
	void setAfterTouch(int x) override {}

protected:

	/* Core DSP objects */
  JerobeamKeplerBouwkampCoreOversampled jbKeplerCore;
	jura::BreakpointModulatorAudioModule * breakpointEnv1Module;
	jura::ModulationManager modulationManager;

	double currentPitch;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(JerobeamKeplerBouwkampModule)
};