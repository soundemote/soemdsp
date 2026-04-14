#pragma once

#include "ChaoticaCore.h"
#include "se_framework/ElanSynthLib/myparams.h"

using jura::MetaParameterManager;
using jura::ModulationManager;

class ChaoticaModule : public BasicMidiPlugin
{
  friend class ChaoticaEditor;

public:
	ChaoticaModule(CriticalSection * lockToUse, jura::MetaParameterManager * metaManagerToUse);

	// plugin
	jura::AudioModuleEditor *createEditor() override;
	void createParameters();

	// host
	void setSampleRate(double v) override;
	void setBeatsPerMinute(double newBpm) override;
	void processBlock(double **inOutBuffer, int numChannels, int numSamples) override;

	// midi
	void noteOn(int noteNumber, int velocity) override { chaoticaCore.noteOn(noteNumber, velocity); }
	void noteOff(int noteNumber) override { chaoticaCore.noteOff(noteNumber); }

	ParamManager paramManager;

	myparams 
		// common
		parGain,
		parOversampling,
		parSmoothing,

		// chaotica
		parStepSize,
		parRate,
		parChaoticaGain,
		parNLDrive,
		par2D,
		parDamping,
		parOffset,
		parQ,
		parXRotation,
		parYRotation,

		parXFilter,
		parYFilter,
		parZFilter,

		parSawFreq,

		// envelope
		parEnvDelay,
		parEnvAttack,
		parEnvDecay,
		parEnvSustain,
		parEnvRelease;

protected:

	/* Core DSP objects */
  ChaoticaCoreOversampled chaoticaCore;
	jura::BreakpointModulatorAudioModule * breakpointEnv1Module;

	InternalModSrc sawMod{ "saw_mod", &chaoticaCore.sawVal };
	
	double currentPitch;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChaoticaModule)
};