#pragma once

#include "JerobeamWirdoSpiralCore.h"
#include "se_framework/ElanSynthLib/myparams.h"

class JerobeamWirdoSpiralModule : public jura::AudioModuleWithMidiIn
{
  friend class JerobeamWirdoSpiralEditor;

public:
	juce::String presetPath;
	juce::String moduleVendor = "Soundemote";

  JerobeamWirdoSpiralModule(juce::CriticalSection *lockToUse, jura::MetaParameterManager* metaManagerToUse);

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
		parSharp,
		parCross,
		parDensity,
		parLength,
		parRotate,
		parSplash,
		parSplashDensity,
		parCut,
		parScrap,
		parRingCut,
		parSplashSpeed,
		parSyncCut;

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
  JerobeamWirdoSpiralCoreOversampled jbWirdoCores;
	jura::BreakpointModulatorAudioModule * breakpointEnv1Module;
	jura::ModulationManager modulationManager;

	double currentPitch;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(JerobeamWirdoSpiralModule)
};