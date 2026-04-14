#pragma once

using jura::ModulationManager;
using jura::MetaParameterManager;
using jura::AudioModuleWithMidiIn;

class BasicEnvelopeModule : public BasicModule, public jura::ModulationSource, public MidiSlave
{
  friend class BasicEnvelopeEditor;

public:
	enum EnvResetMode { env_always, env_on_note, env_when_not_legato, env_never };
	static const vector<String> EnvResetModeMenuItems;

	BasicEnvelopeModule(juce::CriticalSection* lockToUse, MetaParameterManager* metaManagerToUse, ModulationManager* modulationManager, MidiMaster* midiMasterToUse);

	// plugin
	//jura::AudioModuleEditor *createEditor(int type = 0) override;
	void createParameters() override;
	void setupCallbacks() override;

	virtual void triggerAttack() override;
	virtual void triggerNoteOnLegato() override;
	virtual void triggerNoteOffLegato() override;
	virtual void triggerRelease() override;

	// host
	void setSampleRate(double v) override { basicEnvelope.setSampleRate(sampleRate = v); }
	void processBlock(double** inOutBuffer, int numChannels, int numSamples) override;
	void processSampleFrame(double* left, double* right) override;
		// ModulationSource
	double getModulatorOutputSample() //override
	{
		processSampleFrame(&modValue, &modValue);
		return modValue;
	}

	// dsp
	void setNoiseSeed(int v) { basicEnvelope.setNoiseSeed(v); }

	myparams parResetMode{ &paramManager };
	ParameterProfile::VelocityInfluence parVelInfluence{ &paramManager };

	ParameterProfile::ExponentialTime parDelay{ &paramManager };
	ParameterProfile::Linear parAttack{ &paramManager };
	ParameterProfile::Linear parDecay{ &paramManager };
	ParameterProfile::Linear parSustain{ &paramManager };
	ParameterProfile::Linear parRelease{ &paramManager };
	ParameterProfile::Exponential parGlobalTime{ &paramManager };

	ParameterProfile::Linear parAttackFB{ &paramManager };
	ParameterProfile::Linear parDecayFB{ &paramManager };
	ParameterProfile::Linear parReleaseFB{ &paramManager };
	ParameterProfile::Exponential parGlobalFB{ &paramManager };

	myparams parAttackShape{ &paramManager };
	myparams parDecayShape{ &paramManager };
	myparams parReleaseShape{ &paramManager };
	
	ParameterProfile::BipolarAmplitude parAmplitude{ &paramManager };
	myparams parLoop{ &paramManager };
	myparams parMakeBipolar{ &paramManager };
	myparams parInvert{ &paramManager };

	/* Core DSP objects */
	FeedbackADSR basicEnvelope;
	int resetMode = 0;

protected:

	double 
		EnvOutPos = 0,
		EnvOutNeg = 0,
		gain = 0;

	bool makeBipolar = false;

	//InternalModSrc modEnvOutPos{ "EnvOut +", &EnvOutPos };
	//InternalModSrc modEnvOutNeg{ "EnvOut -", &EnvOutNeg };

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BasicEnvelopeModule)
};

//class BasicEnvelopeModule : public AudioModuleWithMidiIn, public jura::ModulationSource
//{
//	friend class BasicEnvelopeEditor;
//
//public:
//	BasicEnvelopeModule(juce::CriticalSection * lockToUse, MetaParameterManager * metaManagerToUse, ModulationManager * modulationManager);
//
//	// plugin
//	jura::AudioModuleEditor *createEditor();
//	void createParameters();
//
//	// host
//	void setSampleRate(double v) { ; }
//	void processBlock(double **inOutBuffer, int numChannels, int numSamples);
//	void processSampleFrame(double * left, double * right);
//
//	// midi
//	void noteOn(int noteNumber, int velocity, bool isNoteTied)
//	{
//		switch (resetMode)
//		{
//		default:
//			basicEnvelope.reset();
//			break;
//		case MonoSynth::ResetMode::when_not_legato:
//			if (!isNoteTied)
//				basicEnvelope.reset();
//			break;
//		}
//	}
//	virtual void noteOff(int noteNumber) { ; }
//
//	// ModulationSource
//	void updateModulationValue() override { processSampleFrame(&modValue, &modValue); }
//
//	// dsp
//	void setNoiseSeed(double v) { ; }
//
//	ParamManager paramManager;
//
//	myparams
//		parResetMode,
//		parVelInfluence,
//
//		parDelay,
//		parAttack,
//		parDecay,
//		parSustain,
//		parRelease,
//		parGlobalTime,
//
//		parAttackFB,
//		parDecayFB,
//		parReleaseFB,
//		parGlobalFB,
//
//		parAttackShape,
//		parDecayShape,
//		parReleaseShape,
//
//		parAmplitude,
//		parLoop,
//		parMakeBipolar;
//
//protected:
//	/* Core DSP objects */
//	RAPT::rsBouncillator<double> basicEnvelope;
//
//	int resetMode;
//
//	double
//		EnvOutPos = 0,
//		EnvOutNeg = 0,
//		gain = 0;
//
//	bool makeBipolar = false;
//
//	//InternalModSrc modEnvOutPos{ "EnvOut +", &EnvOutPos };
//	//InternalModSrc modEnvOutNeg{ "EnvOut -", &EnvOutNeg };
//
//	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BasicEnvelopeModule)
//};
//
