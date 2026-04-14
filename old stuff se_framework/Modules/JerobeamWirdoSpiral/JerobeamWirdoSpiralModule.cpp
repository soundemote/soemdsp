#include "JerobeamWirdoSpiralModule.h"

JerobeamWirdoSpiralModule::JerobeamWirdoSpiralModule(CriticalSection *lockToUse, 
	jura::MetaParameterManager* metaManagerToUse)
  :  AudioModuleWithMidiIn(lockToUse, metaManagerToUse), modulationManager(lockToUse)
{
  ScopedLock scopedLock(*lock);

	moduleName = JucePlugin_Name; // correct name is vital for licensing system
	moduleVendor = JucePlugin_Manufacturer;

	String desktopPath = juce::File::getSpecialLocation(juce::File::userDesktopDirectory).getFullPathName();

#ifdef _WIN32
	presetPath     = jura::getApplicationDirectory() + "/Presets";
#elif __APPLE__
	presetPath     = File("~/Library/Audio/Presets/"+moduleVendor+"/"+moduleName+"/Presets").getFullPathName();
#elif __linux__ 
	presetPath     = getApplicationDirectory() + "/Presets";
#endif

	setActiveDirectory(presetPath);

  modulationManager.setMetaParameterManager(metaManagerToUse); // for attaching mod-depths to metas
  setModulationManager(&modulationManager);     // sets pointer in ModulatableAudioModule baseclass

	/* Setup Core DSP objects */
	breakpointEnv1Module = new jura::BreakpointModulatorAudioModule(lock);
	breakpointEnv1Module->setModuleName("Envelope1");
  breakpointEnv1Module->setModulationSourceName("Envelope1"); // must be unique for identification in state recall
	addChildAudioModule(breakpointEnv1Module);

	modulationManager.registerModulationSource(breakpointEnv1Module);

  createParameters();
}

void JerobeamWirdoSpiralModule::createParameters() //boing
{
	// fill paramStrIds, MUST HE CALLED HERE
	int i = 0;
	paramStrIds =
	{
		parGain.init(i++, SLIDER, "Gain", 0, 1, 1),
		parTune.init(i++, SLIDER, "Tune", -350, 70, 0, jura::Parameter::LINEAR_BIPOLAR),
		parOctave.init(i++, SLIDER, "Octave", -12, 12, 0, jura::Parameter::LINEAR_BIPOLAR),
		parOversampling.init(i++, SLIDER, "Oversampling", 1, 8, 0),

		parSharp.init(i++, SLIDER, "Sharp", 0, 1, 0),
		parCross.init(i++, SLIDER, "Cross", 0, 1, 0),
		parDensity.init(i++, SLIDER, "Density", -333, +333, 5, jura::Parameter::LINEAR_BIPOLAR),
		parLength.init(i++, SLIDER, "Length", 0, 2, 1),
		parRotate.init(i++, SLIDER, "Rotate", 0, 1, 0),
		parSplash.init(i++, SLIDER, "Splash", 0, 1, 0),
		parSplashDensity.init(i++, SLIDER, "SplashDensity", 0, 10, 0),
		parCut.init(i++, SLIDER, "Cut", 1, 1000, 1000),
		parScrap.init(i++, SLIDER, "Scrap", 0, 1, 1),
		parRingCut.init(i++, SLIDER, "RingCut", 0, 10, 10),
		parSplashSpeed.init(i++, SLIDER, "SplashSpeed", 0, 10, 0),
		parSyncCut.init(i++, SLIDER, "SyncCut", 0, 1, 0),
	};

	parOctave.interval = 1;
	parOversampling.interval = 1;

	// instantiate ptr, MUST BE CALLED HERE
	for (myparams * p : paramStrIds)
	{
		p->instantiateParameter();
		addObservedParameter(p->ptr);
	}

	parTune.setCallback([this](double v) { jbWirdoCores.setPitchOffset(parTune + parOctave*12); });
	parOctave.setCallback([this](double v) { jbWirdoCores.setPitchOffset(parTune + parOctave*12); });
	parOversampling.setCallback([this](double v) { jbWirdoCores.setOversampling(v); });

	parSharp.setCallback([this](double v) { jbWirdoCores.jbWirdo.setSharp(v); });
	parCross.setCallback([this](double v) { jbWirdoCores.jbWirdo.setCross(v); });
	parDensity.setCallback([this](double v) { jbWirdoCores.jbWirdo.setDensity(v); });
	parLength.setCallback([this](double v) { jbWirdoCores.jbWirdo.setLength(v); });
	parRotate.setCallback([this](double v) { jbWirdoCores.jbWirdo.setRotate(v); });
	parSplash.setCallback([this](double v) { jbWirdoCores.jbWirdo.setSplash(v); });
	parSplashDensity.setCallback([this](double v) { jbWirdoCores.jbWirdo.setSplashDensity(v); });
	parCut.setCallback([this](double v) { jbWirdoCores.jbWirdo.setCut(v); });
	parScrap.setCallback([this](double v) { jbWirdoCores.jbWirdo.setScrap(v); });
	parRingCut.setCallback([this](double v) { jbWirdoCores.jbWirdo.setRingCut(v); });
	parSplashSpeed.setCallback([this](double v) { jbWirdoCores.jbWirdo.setSplashSpeed(v); });
	parSyncCut.setCallback([this](double v) { jbWirdoCores.jbWirdo.setSyncCut(v); });
}

void JerobeamWirdoSpiralModule::processBlock(double **inOutBuffer, int numChannels, int numSamples)
{
  jassert(numChannels == 2);

	bool doApplyModulations = modulationManager.getNumConnections() != 0;

	double left, right;
  for(int n = 0; n < numSamples; n++)
  {
		if (doApplyModulations) modulationManager.applyModulations();
    jbWirdoCores.processSampleFrame(&left, &right);
		if (inOutBuffer[0]) inOutBuffer[0][n] = left * parGain;
		if (inOutBuffer[1]) inOutBuffer[1][n] = right * parGain;
  } 
}

void JerobeamWirdoSpiralModule::setSampleRate(double newSampleRate)
{
	jbWirdoCores.setSampleRate(newSampleRate);
	breakpointEnv1Module->setSampleRate(newSampleRate);
}

void JerobeamWirdoSpiralModule::noteOn(int noteNumber, int velocity)
{
	jbWirdoCores.noteOn(noteNumber, velocity);
	breakpointEnv1Module->noteOn(noteNumber, velocity);
}

void JerobeamWirdoSpiralModule::noteOff(int noteNumber)
{
	jbWirdoCores.noteOn(noteNumber, 0);
	breakpointEnv1Module->noteOff(noteNumber);
}

void JerobeamWirdoSpiralModule::setBeatsPerMinute(double newBpm)
{
	jbWirdoCores.setBPM(newBpm);
}

void JerobeamWirdoSpiralModule::setStateFromXml(const XmlElement& xmlState, 
  const juce::String& stateName, bool markAsClean)
{
  AudioModuleWithMidiIn::setStateFromXml(xmlState, stateName, markAsClean);
  //patchComments = xmlState.getStringAttribute("Comments", "");
}

XmlElement* JerobeamWirdoSpiralModule::getStateAsXml(const juce::String& stateName, bool markAsClean)
{
	XmlElement* state = AudioModuleWithMidiIn::getStateAsXml(stateName, markAsClean);
  //state->setAttribute("Comments", patchComments);
  return state;
}