#include "JerobeamKeplerBouwkampModule.h"

JerobeamKeplerBouwkampModule::JerobeamKeplerBouwkampModule(CriticalSection *lockToUse, 
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

void JerobeamKeplerBouwkampModule::createParameters()
{
	// fill paramStrIds, MUST HE CALLED HERE
	int i = 0;
	paramStrIds =
	{
		parGain.init(i++, SLIDER, "Gain", 0, 1, 1),
		parTune.init(i++, SLIDER, "Tune", -350, 70, 0, jura::Parameter::LINEAR_BIPOLAR),
		parOctave.init(i++, SLIDER, "Octave", -12, 12, 0, jura::Parameter::LINEAR_BIPOLAR),
		parOversampling.init(i++, SLIDER, "Oversampling", 1, 8, 0),

		parStart.init(i++, SLIDER, "Start", 3, 20, 3),
		parLength.init(i++, SLIDER, "Length", 1, 20, 1),
		parCircles.init(i++, SLIDER, "Circles", 0, 1, 0),
		parZoom.init(i++, SLIDER, "Zoom", 0, 1, 0),
		parRotation.init(i++, SLIDER, "Rotation", -1, +1, 0, jura::Parameter::LINEAR_BIPOLAR),
		parTri.init(i++, SLIDER, "Tri", 0, 1, 0),
	};

	parOctave.interval = 1;
	parOversampling.interval = 1;
	parStart.interval = 1;
	parLength.interval = 1;

	// instantiate ptr, MUST BE CALLED HERE
	for (myparams * p : paramStrIds)
	{
		p->instantiateParameter();
		addObservedParameter(p->ptr);
	}

	parTune.setCallback([this](double v) { jbKeplerCore.setPitchOffset(parTune + parOctave*12); });
	parOctave.setCallback([this](double v) { jbKeplerCore.setPitchOffset(parTune + parOctave*12); });
	parOversampling.setCallback([this](double v) { jbKeplerCore.setOversampling(v); });

	parStart.setCallback([this](double v) { jbKeplerCore.jbKepler.setStart(v); });
	parLength.setCallback([this](double v) { jbKeplerCore.jbKepler.setLength(v); });
	parCircles.setCallback([this](double v) { jbKeplerCore.jbKepler.setCircles(v); });
	parZoom.setCallback([this](double v) { jbKeplerCore.jbKepler.setZoom(v); });
	parRotation.setCallback([this](double v) { jbKeplerCore.jbKepler.setRotation(v); });
	parTri.setCallback([this](double v) { jbKeplerCore.jbKepler.setTri(v); });
}

void JerobeamKeplerBouwkampModule::processBlock(double **inOutBuffer, int numChannels, int numSamples)
{
  jassert(numChannels == 2);

	bool doApplyModulations = modulationManager.getNumConnections() != 0;

	double left, right;
  for(int n = 0; n < numSamples; n++)
  {
		if (doApplyModulations) modulationManager.applyModulations();
    jbKeplerCore.processSampleFrame(&left, &right);
		if (inOutBuffer[0]) inOutBuffer[0][n] = left * parGain;
		if (inOutBuffer[1]) inOutBuffer[1][n] = right * parGain;
  } 
}

void JerobeamKeplerBouwkampModule::setSampleRate(double newSampleRate)
{
	jbKeplerCore.setSampleRate(newSampleRate);
	breakpointEnv1Module->setSampleRate(newSampleRate);
}

void JerobeamKeplerBouwkampModule::noteOn(int noteNumber, int velocity)
{
	jbKeplerCore.noteOn(noteNumber, velocity);
	breakpointEnv1Module->noteOn(noteNumber, velocity);
}

void JerobeamKeplerBouwkampModule::noteOff(int noteNumber)
{
	jbKeplerCore.noteOn(noteNumber, 0);
	breakpointEnv1Module->noteOff(noteNumber);
}

void JerobeamKeplerBouwkampModule::setBeatsPerMinute(double newBpm)
{
	jbKeplerCore.setBPM(newBpm);
}

void JerobeamKeplerBouwkampModule::setStateFromXml(const XmlElement& xmlState, 
  const juce::String& stateName, bool markAsClean)
{
  AudioModuleWithMidiIn::setStateFromXml(xmlState, stateName, markAsClean);
  //patchComments = xmlState.getStringAttribute("Comments", "");
}

XmlElement* JerobeamKeplerBouwkampModule::getStateAsXml(const juce::String& stateName, bool markAsClean)
{
	XmlElement* state = AudioModuleWithMidiIn::getStateAsXml(stateName, markAsClean);
  //state->setAttribute("Comments", patchComments);
  return state;
}