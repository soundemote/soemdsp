#pragma once

#include "RhythmAndPitchGeneratorCore.h"

class JUCE_API RaPG_EnvelopeModule : public BasicModule
{
public:
	RaPG_EnvelopeModule(CriticalSection *lockToUse, jura::MetaParameterManager * metaManagerToUse)
		: BasicModule(lockToUse, metaManagerToUse)
	{
		BasicModule::setupParametersAndModules();
	}
	~RaPG_EnvelopeModule() = default;

	// plugin
	void createParameters() override;

	// host
	double getSample() { return envelope.getSample(); }
	void reset() override {}
	void setSampleRate(double v) override { envelope.setSampleRate(v); }

	CurveEnvelope envelope;

	bool triggerOnNote = false;
	bool resetOnNote = false;
	bool triggerOnClick = false;
	bool resetOnClick = false;
	bool triggerOnMasterReset = false;
	bool resetOnMasterReset = false;

	double ghostAmplitude = 0.5;

protected:

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RaPG_EnvelopeModule)
};

class JUCE_API RaPG_PitchModule : public BasicModule
{
public:
	RaPG_PitchModule(CriticalSection *lockToUse, jura::MetaParameterManager * metaManagerToUse)
		: BasicModule(lockToUse, metaManagerToUse)
	{
		BasicModule::setupParametersAndModules();
	}
	virtual ~RaPG_PitchModule() = default;

	void createParameters() override;

	// host
	void reset() override
	{
		pitchQuantizer.reset();
	}

	void setSampleRate(double v) override
	{
		pitchQuantizer.setSampleRate(v);
	}

	void getSample()
	{
		pitchQuantizer.run();
	}

	void setBeatsPerMinute(double v) override {}

	RaPG_PitchQuantizer pitchQuantizer;

protected:

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RaPG_PitchModule)
};

class JUCE_API RaPG_ClockModule : public BasicModule
{
public:
	RaPG_ClockModule(CriticalSection *lockToUse, jura::MetaParameterManager * metaManagerToUse, RaPG_RhythmGenerator * masterRhythmGen);
	~RaPG_ClockModule() = default;

	// plugin
	jura::AudioModuleEditor * createEditor(int type = 0) override { return nullptr; }

	void setSampleRate(double v) override;
	void createParameters() override;

	RaPG_ClockDivider clock;
	RaPG_RhythmGenerator * masterRhythmGen;

protected:

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RaPG_ClockModule)
};

class  RaPG_RhythmModule : public BasicModule
{
public:
	RaPG_RhythmModule(CriticalSection *lockToUse, jura::MetaParameterManager * metaManagerToUse)
		: BasicModule(lockToUse, metaManagerToUse)
	{
		BasicModule::setupParametersAndModules();
	}
	~RaPG_RhythmModule() = default;

	// plugin
	void setupModulesAndModulation() override
	{
		int i = 0;
		for (auto & m : ClockModules)
		{
			++i;
			m = new RaPG_ClockModule(lock, metaParamManager, &rhythmGen);
			rhythmGen.AllClocksArray.push_back(&m->clock);
			m->clock.id = i-1;
			m->setModuleName("Clock" + String(i));
			addChildAudioModule(m);
		}
	}
	void createParameters() override;

	// host
	void reset() override { rhythmGen.reset(); }
	void getSample()
	{
		rhythmGen.run();
	}
	void setSampleRate(double v) override
	{
		for (auto & clock : ClockModules)
		{
			clock->setSampleRate(v);
		}
	}
	void setBeatsPerMinute(double v) override {}
	void processBlock(double **inOutBuffer, int numChannels, int numSamples) override {}

	bool resetOnUserNote = false;
	bool resetOnArpNote = false;
	bool resetIfNotUserLegato = false;
	bool resetIfNotArpLegato = false;
	bool resetOnArpStep = false;
	bool resetOnArpPattern = false;

	RaPG_RhythmGenerator rhythmGen;
	vector<RaPG_ClockModule *> ClockModules{ 7 };

protected:

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RaPG_RhythmModule)
};

class RaPG_OscillatorModule : public BasicModule
{
public:
	RaPG_OscillatorModule(CriticalSection *lockToUse, jura::MetaParameterManager * metaManagerToUse)
		: BasicModule(lockToUse, metaManagerToUse)
	{
		BasicModule::setupParametersAndModules();
	}
	virtual ~RaPG_OscillatorModule() = default;

	// plugin
	void createParameters() override;

	// host
	void reset() override
	{
		oscillator.reset();
	}

	void setSampleRate(double v) override
	{
		oscillator.setSampleRate(v);
	}

	double getSample()
	{
		return oscillator.getSample();
	}

	bool resetOnNote = false;
	bool resetOnClick = false;
	bool resetOnMasterReset = false;
	double ampFilterEnvMod = 0;

	RaPG_Oscillator oscillator;	

protected:

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RaPG_OscillatorModule)
};

class RhythmAndPitchGeneratorModule : public BasicModule, public MidiSlave
{
public:
  RhythmAndPitchGeneratorModule(CriticalSection *lockToUse, jura::MetaParameterManager * metaManagerToUse);
	virtual ~RhythmAndPitchGeneratorModule() = default;

	// plugin
	jura::AudioModuleEditor * createEditor(int type = 0) override;
	void setupModulesAndModulation() override;
	void createParameters() override;

	void setStateFromXml(const XmlElement & xmlState, const String & stateName, bool markAsClean) override;
	XmlElement* getStateAsXml(const juce::String & stateName, bool markAsClean) override;
	bool saveStateToXmlFile(const juce::File & fileToSaveTo) override
	{
		bool b = StateFileManager::saveStateToXmlFile(fileToSaveTo);
		sendChangeMessage();
		return b;
	}

	// host
  void processSampleFrame(double * left, double * right) override;
	virtual void reset() override;
	virtual void setSampleRate(double newSampleRate) override;
	void setBeatsPerMinute(double v) override
	{
		bpm = v <= 0.0 ? 120.0 : v;
		MIDIMASTER.setBPM(bpm);
		sendChangeMessage();
	}
  virtual void processBlock(double **inOutBuffer, int numChannels, int numSamples) override;

	void handleMidiMessage(MidiMessage message) override;

	void setOversampling(int v);

	void triggerNoteOn() override;
	void triggerNoteOff() override;
	void triggerRelease() override;
	void triggerBPMChange() override;

	void recalculateMasterTimerFrequency();
	void recalculateResetBarTimerFrequency();

	MidiMaster MIDIMASTER;
	jura::rsSmoothingManager smootyMan;
	RAPT::rsEllipticSubBandFilter<double, double> antiAliasFilterL, antiAliasFilterR;
	RAPT::rsOnePoleFilter<double,double> dcFilter;

	RaPG_EnvelopeModule * envelopeModule;
	RaPG_OscillatorModule * oscillatorModule;
	RaPG_PitchModule * pitchModule;
	RaPG_RhythmModule * rhythmModule;

	PreciseTimer masterTimer;
	PreciseTimer resetBarsTimer;
	double globalResetBars = 2;
	int oversampling = 1;
	double hostSampleRate = 44100;
	double internalSampleRate = 44100;
	double masterRate = 1;
	double masterRateWithNoise = 1;
	bool doesMasterRateControlPitch = false;

	bool masterRateNeedsUpdating = false;
	double masterTimerFrequency = 1;
	double bars = 1;
	double bpm = 120;

	double oscillatorFinalOutValue = 0;
	double envelopeOut = 0;
	double masterVolume = 1;

	elan::RandomGeneratorTimed masterRateRandomGenerator;

	jura::ModulationManager modulationManager;
	vector<jura::ModulatableParameter2 *> parametersForSmoothing;

protected:

	InternalModSrc oscSource{ "osc_mod", "oscillator source" };
	InternalModSrc oscFiltered{ "osc_flt_mod", "oscillator filtered" };
	InternalModSrc oscFltAmp{ "osc_fltamp_mod", "oscillator flt amp" };
	InternalModSrc oscFinalOut{ "osc_final_mod", "oscillator final out" };
	InternalModSrc envSource{ "env_mod", "envelope source" };
	InternalModSrc pitchNoiseSource{ "pitch_noise_mod", "pitch noise source" };
	InternalModSrc rhythmClockIndex{ "rhythm_clock_index_mod", "rhythm clock index" };
	InternalModSrc rhythmNoiseSource{ "rhythm_noise_mod", "rhythm noise source" };
	InternalModSrc rhythmClickSource{ "rhythm_pulse_amp", "rythm pulse amplitude" };

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RhythmAndPitchGeneratorModule)
};
