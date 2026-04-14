#include "BasicEnvelopeModule.h"

const vector<String> BasicEnvelopeModule::EnvResetModeMenuItems{ "Always", "On-note", "Legato", "Never" };

BasicEnvelopeModule::BasicEnvelopeModule(juce::CriticalSection * lockToUse, MetaParameterManager * metaManagerToUse, ModulationManager * modulationManager, MidiMaster * midiMasterToUse) :
	BasicModule(lockToUse, metaManagerToUse, modulationManager)
{
	/* Setup Core DSP objects */
	//modulationManager->registerModulationSource(&modEnvOutPos);
	//modulationManager->registerModulationSource(&modEnvOutNeg);

	midiMasterPtr = midiMasterToUse;
	BasicModule::setupParametersAndModules();

	setModuleTypeName("BasicEnvelope");
}

void BasicEnvelopeModule::createParameters()
{
	vector<String> ShapeMenuItems ={ "EXP", "EXP2", "LOG", "S-CRV" };

	// fill paramManager, MUST HE CALLED HERE
	parResetMode.initCombobox("Reset Mode", 3, { "Always", "On-note", "Legato", "Never", });

	parGlobalFB.initSlider("Speed", 100, 20000, 150);

	parVelInfluence.initSlider("Vel Influence", 0, 1, 0.5);

	parDelay.initSlider("Delay Time", 1.e-5, 5, 1.e-5);
	//parAttack.initSlider("Attack_Time", 1.e-6, 5, 1.e-6, jura::Parameter::EXPONENTIAL);
	//parDecay.initSlider("Decay_Time", 1.e-6, 5, 1.e-6, jura::Parameter::EXPONENTIAL);
	//parRelease.initSlider("Release_Time", 1.e-6, 5, 1.e-6, jura::Parameter::EXPONENTIAL);

	parAttackFB.initSlider("Attack", 1.e-6, 3, 1.e-6);
	parDecayFB.initSlider("Decay", 1.e-6, 3, 1.5);
	parSustain.initSlider("Sustain", 0, 1, 0);
	parReleaseFB.initSlider("Release", 1.e-6, 3, 1.5);

	parGlobalTime.initSlider("Linear Time", .001, 20000, 100);

	parAttackShape.initCombobox("Atk Shape", 2, ShapeMenuItems);
	parDecayShape.initCombobox("Dec Shape", 0, ShapeMenuItems);
	parReleaseShape.initCombobox("Rel Shape", 0, ShapeMenuItems);

	parAmplitude.initSlider("Gain", -1, +1, 1);
	parLoop.initButton("Loop_OnOff", 0);
	parMakeBipolar.initButton("Bipolar_OnOff", 0);
	parInvert.initButton("Invert", 0);

	parLoop.text = "Loop";
	parMakeBipolar.text = "Bipolar";
	parInvert.text = "Inv";
	parResetMode.text = "Reset";
	parVelInfluence.text = "Vel";

	parGlobalTime.text = "<- Linear / Curved ->";

	paramManager.instantiateParameters(this);
}

void BasicEnvelopeModule::setupCallbacks()
{
	parResetMode.setCallback([this](double v) { resetMode = (EnvResetMode)(int)v; });

	parVelInfluence.setCallback([this](double v) { basicEnvelope.setVelocityInfluence(v); });

	parGlobalTime.setCallback([this](double v) { basicEnvelope.setGlobalTime(v); });

	parDelay.setCallback([this](double v) { basicEnvelope.setDelayTime(v); });
	//parAttack.setCallback([this](double v) { basicEnvelope.setAttackTime(v); });
	//parDecay.setCallback([this](double v) { basicEnvelope.setDecayTime(v); });
	//parRelease.setCallback([this](double v) { basicEnvelope.setReleaseTime(v); });

	basicEnvelope.setAttackTime(0);
	basicEnvelope.setDecayTime(0);
	basicEnvelope.setReleaseTime(0);

	parAttackFB.setCallback([this](double v) { basicEnvelope.setAttackFeedback(v); });
	parDecayFB.setCallback([this](double v) { basicEnvelope.setDecayFeedback(v); });
	parReleaseFB.setCallback([this](double v) { basicEnvelope.setReleaseFeedback(v); });
	parSustain.setCallback([this](double v) { basicEnvelope.setSustainAmplitude(v); });
	parGlobalFB.setCallback([this](double v) { basicEnvelope.setGlobalFeedback(v); });

	parDelay.setCallback([this](double v) { basicEnvelope.setDelayTime(v); });
	parAttackShape.setCallback([this](double v) { basicEnvelope.setAttackShape((FeedbackADSR::Shape)(int)v); });
	parDecayShape.setCallback([this](double v) { basicEnvelope.setDecayShape((FeedbackADSR::Shape)(int)v); });
	parReleaseShape.setCallback([this](double v) { basicEnvelope.setReleaseShape((FeedbackADSR::Shape)(int)v); });

	parAmplitude.setCallback([this](double v) { gain = v; });
	parLoop.setCallback([this](double v) { basicEnvelope.setDoLoop(v > 0.5); });
	parMakeBipolar.setCallback([this](double v) { makeBipolar = v > 0.5; });
}

void BasicEnvelopeModule::triggerAttack()
{
	if (parResetMode != (double)EnvResetMode::env_never)
		basicEnvelope.reset();

	basicEnvelope.triggerAttack(getMidiState().velocity, true);
}

void BasicEnvelopeModule::triggerNoteOnLegato()
{
	switch ((int)parResetMode)
	{
	case EnvResetMode::env_always:
	case EnvResetMode::env_on_note:
		basicEnvelope.reset();
		basicEnvelope.triggerAttack(getMidiState().velocity, true);
		break;
	}
}

void BasicEnvelopeModule::triggerNoteOffLegato()
{
	if (parResetMode == (double)EnvResetMode::env_always)
	{
		basicEnvelope.reset();
		basicEnvelope.triggerAttack(getMidiState().velocity, true);
	}
}

void BasicEnvelopeModule::triggerRelease()
{
	basicEnvelope.triggerRelease();
}

void BasicEnvelopeModule::processSampleFrame(double * left, double * right)
{	
	if (basicEnvelope.isIdle())
		return;

	double out;

	if (makeBipolar)
	{
		out = basicEnvelope.getSample() * 2 - 1;
		EnvOutNeg = -out;

	}
	else
	{
		out = basicEnvelope.getSample();
		EnvOutNeg = 1-out;

	}

	EnvOutPos = out;

	if (parInvert)
		out = -EnvOutNeg;

	*left = *right = out * gain;
}

void BasicEnvelopeModule::processBlock(double **inOutBuffer, int numChannels, int numSamples)
{
	jassert(numChannels == 2);

	double left, right;
	for (int n = 0; n < numSamples; n++)
	{
		processSampleFrame(&left, &right);

		left = clamp(left, -1.0, 1.0);

		if (inOutBuffer[0]) inOutBuffer[0][n] = left;
		if (inOutBuffer[1]) inOutBuffer[1][n] = left;
	}
}

//void BasicEnvelopeModule::createParameters()
//{
//	vector<String> ShapeMenuItems ={ "EXP", "EXP2", "LOG", "S-CRV" };
//
//	// fill paramManager, MUST HE CALLED HERE
//	paramManager.addParameters(
//	{
//		parResetMode.initCombobox("Reset Mode", 0, MonoSynth::ResetModeMenuItems),
//
//		parGlobalFB.initSlider("Shape", 1.e-8, 500, 1, jura::Parameter::EXPONENTIAL),
//
//		parVelInfluence.initSlider("x", 0, 1, 0.5),
//
//		parDelay.initSlider("xx", 1.e-6, 5, 1.e-6, jura::Parameter::EXPONENTIAL),
//		//parAttack.initSlider("Attack_Time", 1.e-6, 5, 1.e-6, jura::Parameter::EXPONENTIAL),
//		//parDecay.initSlider("Decay_Time", 1.e-6, 5, 1.e-6, jura::Parameter::EXPONENTIAL),
//		//parRelease.initSlider("Release_Time", 1.e-6, 5, 1.e-6, jura::Parameter::EXPONENTIAL),
//
//		parAttackFB.initSlider("Attack", 1.e-6, 1.2, 1.e-6, jura::Parameter::LINEAR),
//		parDecayFB.initSlider("Decay", 1.e-6, 1.2, 1, jura::Parameter::LINEAR),
//		parSustain.initSlider("Floor", 0, 1, 0),
//		parReleaseFB.initSlider("Ceil", 1.e-6, 1.2, 1, jura::Parameter::LINEAR),
//
//		parGlobalTime.initSlider("xxx", 1.e-2, 1000, 1, jura::Parameter::EXPONENTIAL),
//
//		parAttackShape.initCombobox("xxxx", 2, ShapeMenuItems),
//		parDecayShape.initCombobox("xxxxx", 0, ShapeMenuItems),
//		parReleaseShape.initCombobox("xxxxxx", 0, ShapeMenuItems),
//
//		parAmplitude.initSlider("xxxxxxx", -1, +1, 1),
//		parLoop.initButton("xxxxxxx", 0),
//		parMakeBipolar.initButton("Bipolar", 0),
//	});
//
//	// instantiate ptr, MUST BE CALLED HERE
//	for (myparams * p : paramManager)
//	{
//		p->instantiateParameter();
//		addObservedParameter(p->ptr);
//	}
//	parResetMode.setCallback([this](double v) { resetMode = v; });
//
//	parVelInfluence.setCallback([this](double v) { ; });
//
//	parGlobalTime.setCallback([this](double v) { ; });
//
//	parDelay.setCallback([this](double v) { ; });
//	//parAttack.setCallback([this](double v) { basicEnvelope.setAttackTime(v); });
//	//parDecay.setCallback([this](double v) { basicEnvelope.setDecayTime(v); });
//	//parRelease.setCallback([this](double v) { basicEnvelope.setReleaseTime(v); });
//
//	parAttackFB.setCallback([this](double v) { basicEnvelope.setIncrement(v); });
//	parDecayFB.setCallback([this](double v) { basicEnvelope.setDecrement(v); });
//	parReleaseFB.setCallback([this](double v) { basicEnvelope.setCeil(v); });
//	parSustain.setCallback([this](double v) { basicEnvelope.setFloor(v); });
//	parGlobalFB.setCallback([this](double v) { basicEnvelope.setShape(v); });
//
//	parDelay.setCallback([this](double v) { ; });
//	parAttackShape.setCallback([this](double v) {; });
//	parDecayShape.setCallback([this](double v) { ; });
//	parReleaseShape.setCallback([this](double v) { ; });
//
//	parAmplitude.setCallback([this](double v) { gain = v; });
//	parLoop.setCallback([this](double v) { ; });
//	parMakeBipolar.setCallback([this](double v) { makeBipolar = v; });
//
//
//	parVelInfluence.stringConvertFunc = StringFunc2;
//	parDelay.stringConvertFunc =  secondsToStringWithUnitTotal4;
//
//}
//
//void BasicEnvelopeModule::processSampleFrame(double * left, double * right)
//{
//	double out;
//
//	if (makeBipolar)
//	{
//		out = basicEnvelope.getSample() * 2 - 1;
//		EnvOutNeg = -out;
//	}
//	else
//	{
//		out = basicEnvelope.getSample();
//		EnvOutNeg = 1-out;
//	}
//
//	EnvOutPos = out;
//
//	*left = *right = out * gain;
//}
//
//void BasicEnvelopeModule::processBlock(double **inOutBuffer, int numChannels, int numSamples)
//{
//	jassert(numChannels == 2);
//
//	//bool doApplyModulations = modulationManager.getNumConnections() != 0;
//
//	bool doParamSmoothing = paramManager.getNumParamsToSmooth() != 0;
//
//	double left, right;
//	for (int n = 0; n < numSamples; n++)
//	{
//		//if (doApplyModulations) 
//		//modulationManager.applyModulations();
//
//		processSampleFrame(&left, &right);
//
//		left = clamp(left, -1.0, 1.0);
//
//		if (inOutBuffer[0]) inOutBuffer[0][n] = left;
//		if (inOutBuffer[1]) inOutBuffer[1][n] = left;
//
//		if (doParamSmoothing)
//			paramManager.doSmoothing();
//	}
//
//	paramManager.cleanup();
//}
