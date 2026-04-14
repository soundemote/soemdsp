#pragma once

#include "utils/lmdsp.hpp"
#include <jura_framework/audio/jura_AudioModule.h>
#include "se_framework/ElanSynthLib/myparams.h"
#include "se_framework/ElanSynthLib/ElanSynthLib.h"

#include "../../PrettyScope/Source/ScopeAudioBuffer.h"
#include "../../PrettyScope/Source/OpenGLBrush.h"
#include "../../ChaosFilter/Source/ChaosFilterCore.h"
#include <map>

#include "ChaosArpCore.h"

#pragma warning(disable: 4267) // size_t to GLsizei
#pragma warning(disable: 4244) // size_t to to int

using namespace juce;
using std::vector;

class JUCE_API ChaosArp_EnvelopeModule : public BasicModule
{
public:
	ChaosArp_EnvelopeModule(CriticalSection *lockToUse, jura::MetaParameterManager * metaManagerToUse)
		: BasicModule(lockToUse, metaManagerToUse)
	{
		BasicModule::setupParametersAndModules();
	}
	~ChaosArp_EnvelopeModule() = default;

	// plugin
	jura::AudioModuleEditor * createEditor(int type = 0) override;
	//void setupModulesAndModulation() override;
	void createParameters() override;
	//void setupCallbacks() override;
	//void setInitPatchValues() override;
	//void setHelpTextAndLabels() override;

	// host
	void reset() override {}
	void setSampleRate(double v) override { envelope.setSampleRate(v); }
	void setBeatsPerMinute(double v) override { envelope.setBPM(v); }

	// midi
	//void allNotesOff() override;
	//void setPitchBend(int channel, int value) override;
	//void setChannelPressure(int channel, int value) override;
	//void setMidiController(int channel, int contoller, int value) override;

	//void triggerAttack() override;
	//void triggerNoteOnLegato() override;
	//void triggerNoteOffLegato() override;
	//void triggerRelease() override;

	CurveEnvelope envelope;

	double invertModifier = +1;
	bool isUnipolar = false;

	bool resetOnUserNote = false;
	bool resetOnArpNote = false;
	bool resetIfNotUserLegato = false;
	bool resetIfNotArpLegato = false;
	bool resetOnArpStep = false;
	bool resetOnArpPattern = false;

	bool triggerOnUserNote = false;
	bool triggerOnArpNote = false;
	bool triggerIfNotUserLegato = false;
	bool triggerIfNotArpLegato = false;
	bool triggerOnArpStep = false;
	bool triggerOnArpPattern = false;

protected:

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChaosArp_EnvelopeModule)
};

class JUCE_API ChaosArp_LFOModule : public BasicModule
{
public:
	ChaosArp_LFOModule(CriticalSection *lockToUse, jura::MetaParameterManager * metaManagerToUse)
		: BasicModule(lockToUse, metaManagerToUse)
	{
		BasicModule::setupParametersAndModules();
	}
	~ChaosArp_LFOModule() = default;

	// plugin
	jura::AudioModuleEditor * createEditor(int type = 0) override;
	//void setupModulesAndModulation() override;
	void createParameters() override;
	//void setupCallbacks() override;
	//void setInitPatchValues() override;
	//void setHelpTextAndLabels() override;

	// host
	void reset() override {}
	void setSampleRate(double newSampleRate) override {}
	void setBeatsPerMinute(double v) override {}
	void processBlock(double **inOutBuffer, int numChannels, int numSamples) override {}

	bool resetOnUserNote = false;
	bool resetOnArpNote = false;
	bool resetIfNotUserLegato = false;
	bool resetIfNotArpLegato = false;
	bool resetOnArpStep = false;
	bool resetOnArpPattern = false;

	LFOCore lfo;

protected:

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChaosArp_LFOModule)
};

class JUCE_API ChaosArp_FilterModule : public BasicModule
{
public:
	ChaosArp_FilterModule(CriticalSection *lockToUse, jura::MetaParameterManager * metaManagerToUse)
		: BasicModule(lockToUse, metaManagerToUse)
	{
		BasicModule::setupParametersAndModules();
	}
	~ChaosArp_FilterModule() = default;

	// plugin
	jura::AudioModuleEditor * createEditor(int type = 0) override;
	//void setupModulesAndModulation() override;
	void createParameters() override;
	//void setupCallbacks() override;
	//void setInitPatchValues() override;
	//void setHelpTextAndLabels() override;

	// host
	void reset() override {}
	void setSampleRate(double newSampleRate) override {}
	void setBeatsPerMinute(double v) override {}
	void processBlock(double **inOutBuffer, int numChannels, int numSamples) override {}

protected:

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChaosArp_FilterModule)
};


class JUCE_API ChaosArp_ChaosOscModule : public BasicModule
{
public:
	ChaosArp_ChaosOscModule(CriticalSection *lockToUse, jura::MetaParameterManager * metaManagerToUse)
		: BasicModule(lockToUse, metaManagerToUse)
	{
		BasicModule::setupParametersAndModules();
	}
	virtual ~ChaosArp_ChaosOscModule() = default;

	// plugin
	jura::AudioModuleEditor * createEditor(int type = 0) override;
	void setupModulesAndModulation() override
	{
		oscilloscopeModule = new OscilloscopeModule(lock, metaParamManager, &modulationManager);
		oscilloscopeModule->setModuleName(getModuleName()+"_Oscilloscope");
		addChildAudioModule(oscilloscopeModule);

		oscilloscopeModule->setOneDimensional(true);
		oscilloscopeModule->setSync(true);
		oscilloscopeModule->setNumCycles(3);
	}
	void createParameters() override;
	//void setupCallbacks() override;
	//void setInitPatchValues() override;
	//void setHelpTextAndLabels() override;

	// host
	void reset() override
	{
		chaosOscCore.reset();
	}

	void setSampleRate(double v) override
	{
		chaosOscCore.setSampleRate(v);
		oscilloscopeModule->setSampleRate(v);
	}

	double getSample()
	{
		chaosOscCore.getSample();

		double ret = chaosOscCore.currentArpOutValue * 0.1;
		double x = ret * 0.5;

		oscilloscopeModule->processStereoFrame(&x, &x);

		return ret;
	}

	void setBeatsPerMinute(double v) override {}

	ChaosOscCore chaosOscCore;
	OscilloscopeModule * oscilloscopeModule;

	bool resetOnUserNote = false;
	bool resetOnArpNote = false;
	bool resetIfNotUserLegato = false;
	bool resetIfNotArpLegato = false;
	bool resetOnArpStep = false;
	bool resetOnArpPattern = false;

protected:

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChaosArp_ChaosOscModule)
};

class JUCE_API ChaosArp_MainOscModule : public BasicModule
{
public:
	ChaosArp_MainOscModule(CriticalSection *lockToUse, jura::MetaParameterManager * metaManagerToUse)
		: BasicModule(lockToUse, metaManagerToUse)
	{
		BasicModule::setupParametersAndModules();
	}
	virtual ~ChaosArp_MainOscModule() = default;

	// plugin
	jura::AudioModuleEditor * createEditor(int type = 0) override;
	//void setupModulesAndModulation() override
	//{
	//}
	void createParameters() override;
	//void setupCallbacks() override;
	//void setInitPatchValues() override;
	//void setHelpTextAndLabels() override;

	// host
	void reset() override
	{
		outputOscCore.reset();
	}

	void setSampleRate(double v) override
	{
	}

	double getSample()
	{
	}

	void setBeatsPerMinute(double v) override {}

	//MainOscCore mainOscCore;

	bool resetOnUserNote = false;
	bool resetOnArpNote = false;
	bool resetIfNotUserLegato = false;
	bool resetIfNotArpLegato = false;
	bool resetOnArpStep = false;
	bool resetOnArpPattern = false;

	OutputOscCore outputOscCore;

protected:

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChaosArp_MainOscModule)
};

class JUCE_API ChaosArp_ArpeggiatorModule : public BasicModule
{
public:
	ChaosArp_ArpeggiatorModule(CriticalSection *lockToUse, jura::MetaParameterManager * metaManagerToUse)
		: BasicModule(lockToUse, metaManagerToUse)
	{
		BasicModule::setupParametersAndModules();
	}
	virtual ~ChaosArp_ArpeggiatorModule() = default;

	// plugin
	jura::AudioModuleEditor * createEditor(int type = 0) override;
	//void setupModulesAndModulation() override;

	void createParameters() override;
	//void setupCallbacks() override;
	//void setInitPatchValues() override;
	//void setHelpTextAndLabels() override;

	// host
	void reset() override
	{
	}

	void setSampleRate(double v) override
	{
	}

	double getSample()
	{
	}

	void setBeatsPerMinute(double v) override {}

	NoteAndPitchQuantizer notePitchQuantizer;

protected:

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChaosArp_ArpeggiatorModule)
};


class JUCE_API ChaosArpModule : public BasicModule, public MidiSlave
{
public:
  ChaosArpModule(CriticalSection *lockToUse, jura::MetaParameterManager * metaManagerToUse);
	virtual ~ChaosArpModule() = default;

	// plugin
	jura::AudioModuleEditor * createEditor(int type = 0) override;
	void setupModulesAndModulation() override;
	void createParameters() override;
	void setupCallbacks() override {}
	void setInitPatchValues() override {}
	void setHelpTextAndLabels() override {}

	virtual void setStateFromXml(const XmlElement& xmlState, const juce::String& stateName, bool markAsClean) override;
	virtual XmlElement* getStateAsXml(const juce::String& stateName, bool markAsClean) override;

	// host
	virtual void reset() override;
	virtual void setSampleRate(double newSampleRate) override;
	void setBeatsPerMinute(double v) override { MIDIMASTER.setBPM(v); }
  virtual void processBlock(double **inOutBuffer, int numChannels, int numSamples) override;

	void handleMidiMessage(MidiMessage message)
	{
		ScopedLock scopedLock(*lock);

		MIDIMASTER.handleMidiMessage(message);
	}

	// midi
	void allNotesOff() {}
	void setAfterTouch(int channel, int key, int value) {}
	void setPitchBend(int channel, int value);
	void setChannelPressure(int channel, int value);
	void setMidiController(int channel, int contoller, int value);

	void triggerNoteOn() override {}
	void triggerNoteOff() override {}
	void triggerAttack() override {}
	void triggerNoteOnLegato() override {}
	void triggerNoteOffLegato() override {}
	void triggerRelease() override {}

	juce::String screenshotPath;
	juce::String colorMapsPath;

	// inquiry:
	jura::LoadableColorMap* getColorMapPointer() { return &colorMap; }

	// brushes
	std::vector<Brush*> brushes;
	LineBrush line1, line2;
	DotBrush dot1, dot2;

	// Smmooth out framerate text numbers
	ParamSmoother frameRateSmoother;

	// for automation of clearing canvas
	bool doClearCanvas;
	bool canvasWasCleared;

	float tailFadeFactor = 1.0;

	MidiMaster MIDIMASTER;

	// creates sawtooth scanning signal in 1D mode
	RAPT::rsScopeScreenScanner<double> screenScanner;

	ScopeAudioBuffer audioBuffer;

	ChaosArp_ArpeggiatorModule * arpModule;
	ChaosArp_MainOscModule * mainOscModule;
	ChaosArp_FilterModule * filterModule;
	ChaosArp_ChaosOscModule * chaosOsc1Module;
	ChaosArp_ChaosOscModule	*chaosOsc2Module;
	ChaosArp_LFOModule * lfoModule;
	ChaosArp_EnvelopeModule * ampEnvModule;
	ChaosArp_EnvelopeModule * filterEnvModule;

	double osc1ampOut = 0.0;
	double osc2ampOut = 0.0;
	
protected:

  jura::LoadableColorMap colorMap;

  double sampleRate  = 44100;

	/** Returns the value of the screen scanner sawtooth wave used in 1D mode. You should pass an
	input signal that will be used for analysis in sync mode. */
	double getScannerSaw(double in);

  // transform matrix coefficients:
  double Axx, Axy, Ayx, Ayy;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChaosArpModule)
};
