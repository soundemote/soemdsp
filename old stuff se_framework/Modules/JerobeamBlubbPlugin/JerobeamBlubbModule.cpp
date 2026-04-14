#include "JerobeamBlubbModule.h"

JerobeamBlubbModule::JerobeamBlubbModule(CriticalSection *lockToUse, 
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

void JerobeamBlubbModule::createParameters() //boing
{
	// fill paramStrIds, MUST HE CALLED HERE
	int i = 0;
	paramStrIds =
	{
		parGain.init(i++, SLIDER, "Gain", 0, 1, 1),
		parTune.init(i++, SLIDER, "Tune", -350, 70, 0, jura::Parameter::LINEAR_BIPOLAR),
		parOctave.init(i++, SLIDER, "Octave", -12, 12, 0, jura::Parameter::LINEAR_BIPOLAR),
		parOversampling.init(i++, SLIDER, "Oversampling", 1, 64, 0),

		parSize.init(i++, SLIDER, "Size", 0, 1, 0),
		parDensity.init(i++, SLIDER, "Density", 0, 1, 0),
		parAttack.init(i++, SLIDER, "Attack", 1.e-6, 5, 1.e-6),
		parRelease.init(i++, SLIDER, "Release", 1.e-6, 5, 1.e-6),
		parHiTopFreq.init(i++, SLIDER, "HiTopFreq", 0, 100, 440.0),
		parMovementSlewSeconds.init(i++, SLIDER, "MovementSlewSeconds", 0, 5, 0),
		parRotX.init(i++, SLIDER, "RotX", -4, 4, 0),
		parRotY.init(i++, SLIDER, "RotY", -4, 4, 0),
		parRotZ.init(i++, SLIDER, "RotZ", -4, 4, 0),
		parZDepth.init(i++, SLIDER, "ZDept", -10, 10, 0),
		parRho.init(i++, SLIDER, "Rho", -50, 100, 10),
		parSigma.init(i++, SLIDER, "Sigma", -50, 100, 28),
		parBeta.init(i++, SLIDER, "Beta", -50, 50, 2.6),
		parLorenzPitch.init(i++, SLIDER, "LorenzPitch", -50, 200, 1),
	};

	parOctave.interval = 1;
	parOversampling.interval = 1;

	// instantiate ptr, MUST BE CALLED HERE
	for (myparams * p : paramStrIds)
	{
		p->instantiateParameter();
		addObservedParameter(p->ptr);
	}

	parGain.setCallback([this](double v) { jbBlubbCore.setOutputAmplitude(v); });
	parTune.setCallback([this](double v) { jbBlubbCore.setPitchOffset(v + parOctave*12); });
	parOctave.setCallback([this](double v) { jbBlubbCore.setPitchOffset(parTune + v*12); });
	parOversampling.setCallback([this](double v) { jbBlubbCore.setOversampling(v); });

	parRotX.setCallback([this](double v) { jbBlubbCore.jbBlubb.setRotX(v); });
	parRotY.setCallback([this](double v) { jbBlubbCore.jbBlubb.setRotY(v); });
	parRotZ.setCallback([this](double v) { jbBlubbCore.jbBlubb.setRotZ(v); });

	parZDepth.setCallback([this](double v) { jbBlubbCore.jbBlubb.setZDepth(v); });

	parRho.setCallback([this](double v) { jbBlubbCore.jbBlubb.setRho(v); });
	parSigma.setCallback([this](double v) { jbBlubbCore.jbBlubb.setSigma(v); });
	parBeta.setCallback([this](double v) { jbBlubbCore.jbBlubb.setBeta(v); });
	parLorenzPitch.setCallback([this](double v) { jbBlubbCore.jbBlubb.setLorenzPitch(v); });
}

void JerobeamBlubbModule::setSampleRate(double newSampleRate)
{
	jbBlubbCore.setSampleRate(newSampleRate);
	breakpointEnv1Module->setSampleRate(newSampleRate);
}

void JerobeamBlubbModule::noteOn(int noteNumber, int velocity)
{
	jbBlubbCore.noteOn(noteNumber, velocity);
	breakpointEnv1Module->noteOn(noteNumber, velocity);
}

void JerobeamBlubbModule::noteOff(int noteNumber)
{
	jbBlubbCore.noteOn(noteNumber, 0);
	breakpointEnv1Module->noteOff(noteNumber);
}

void JerobeamBlubbModule::setBeatsPerMinute(double newBpm)
{
	jbBlubbCore.setBPM(newBpm);
}

void JerobeamBlubbModule::setStateFromXml(const XmlElement& xmlState, 
  const juce::String& stateName, bool markAsClean)
{
  AudioModuleWithMidiIn::setStateFromXml(xmlState, stateName, markAsClean);
  //patchComments = xmlState.getStringAttribute("Comments", "");
}

XmlElement* JerobeamBlubbModule::getStateAsXml(const juce::String& stateName, bool markAsClean)
{
	XmlElement* state = AudioModuleWithMidiIn::getStateAsXml(stateName, markAsClean);
  //state->setAttribute("Comments", patchComments);
  return state;
}

void JerobeamBlubbModule::processBlock(double **inOutBuffer, int numChannels, int numSamples)
{
	jassert(numChannels == 2);

	bool doApplyModulations = modulationManager.getNumConnections() != 0;

	double left, right;
	for (int n = 0; n < numSamples; n++)
	{
		if (doApplyModulations) modulationManager.applyModulations();
		jbBlubbCore.processSampleFrame(&left, &right);
		if (inOutBuffer[0]) inOutBuffer[0][n] = left * parGain;
		if (inOutBuffer[1]) inOutBuffer[1][n] = right * parGain;
	}
}
