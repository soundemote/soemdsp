#include "XoxosOscillatorModule.h"

XoxosOscillatorModule::XoxosOscillatorModule(CriticalSection *lockToUse, jura::MetaParameterManager* metaManagerToUse)
	: BasicModule(lockToUse, metaManagerToUse)
{
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

void XoxosOscillatorModule::createParameters()
{
	// fill paramManager, MUST HE CALLED HERE
	paramManager.addParameters(
	{
		// common
		parGain.init(SLIDER, "Gain", 0, 1, 1),
		parFrequency.init(SLIDER, "Frequency", 0, 20000, 0),
		parAngle.init(SLIDER, "Rotation", 0, 1, 0),
		parScale.init(SLIDER, "Sharpness", 0, 10, 0),
		parOffset.init(SLIDER, "Squareness", -1, 1, 0),
	});

	// instantiate ptr, MUST BE CALLED HERE
	for (myparams * p : paramManager)
	{
		p->instantiateParameter();
		addObservedParameter(p->ptr);
	}

	parGain.setCallback([this](double v) { xoxosOscillatorCore.setOutputAmplitude(parGain.getSmoothedVal()); });
	parFrequency.setCallback([this](double v) { xoxosOscillatorCore.osc.setFrequency(parFrequency.getSmoothedVal()); });
	parAngle.setCallback([this](double v) { xoxosOscillatorCore.osc.setAngle(parAngle.getSmoothedVal()); });
	parScale.setCallback([this](double v) { xoxosOscillatorCore.osc.setScale(parScale.getSmoothedVal()); });
	parOffset.setCallback([this](double v) { xoxosOscillatorCore.osc.setOffset(parOffset.getSmoothedVal()); });
}

void XoxosOscillatorModule::setSampleRate(double v)
{
	xoxosOscillatorCore.setSampleRate(v);
}

void XoxosOscillatorModule::noteOn(int noteNumber, int velocity)
{
	xoxosOscillatorCore.noteOn(noteNumber, velocity);
}

void XoxosOscillatorModule::noteOff(int noteNumber)
{
	xoxosOscillatorCore.noteOn(noteNumber, 0);
}

void XoxosOscillatorModule::setBeatsPerMinute(double newBpm)
{
	xoxosOscillatorCore.setBPM(newBpm);
}

void XoxosOscillatorModule::processBlock(double **inOutBuffer, int numChannels, int numSamples)
{
	jassert(numChannels == 2);

	bool doApplyModulations = modulationManager.getNumConnections() != 0;

	const bool doParamSmoothing = paramManager.getNumParamsToSmooth() != 0;

	double left, right;
	for (int n = 0; n < numSamples; n++)
	{
		if (doApplyModulations) 
			modulationManager.applyModulations();

		xoxosOscillatorCore.processSampleFrame(&left, &right);

		if (inOutBuffer[0]) inOutBuffer[0][n] = left;
		if (inOutBuffer[1]) inOutBuffer[1][n] = right;

		if (doParamSmoothing)
			paramManager.doSmoothing();
	}

	paramManager.cleanup();
}
