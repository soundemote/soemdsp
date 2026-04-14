#include "ChaoticaModule.h"

ChaoticaModule::ChaoticaModule(CriticalSection * lockToUse, jura::MetaParameterManager * metaManagerToUse)
	: BasicMidiPlugin(lockToUse, metaManagerToUse)
{

	modulationManager.registerModulationSource(&sawMod);

  createParameters();
}

void ChaoticaModule::createParameters() //Chaotica
{
	parOversampling.interval = 1;

	// fill paramManager, MUST HE CALLED HERE
	paramManager.addParameters(
	{
		// common
		parGain.initSlider("Gain", 0, 1, 1),
		parOversampling.initSlider("Oversampling", 1, 128, 1),

		// oscillator
		parStepSize.initSlider("StepSize", 1.e-6, 4.e+4, 4.e+4, jura::Parameter::EXPONENTIAL),
		parRate.initSlider("Rate", 0, 100, 0),
		parChaoticaGain.initSlider("Chaotica Gain", 0, 100, 0),
		parNLDrive.initSlider("NLDrive", 0, 100, 0),
		par2D.initSlider("_2D", 0, 10, 0),
		parDamping.initSlider("Damping", 0, 10, 0),
		parOffset.initSlider("Offset", 0, 10, 0),
		parQ.initSlider("Q", 0, 10, 0),
		parXRotation.initSlider("XRotation", -1, 1, 0),
		parYRotation.initSlider("YRotation", -1, 1, 0),

		parXFilter.initSlider("X filter", 1.e-6, 2.e+4, 2.e+4, jura::Parameter::EXPONENTIAL),
		parYFilter.initSlider("Y filter", 1.e-6, 2.e+4, 2.e+4, jura::Parameter::EXPONENTIAL),
		parZFilter.initSlider("Z filter", 1.e-6, 2.e+4, 2.e+4, jura::Parameter::EXPONENTIAL),

		parSawFreq.initSlider("SawFreq", 1, 16000, 1, jura::Parameter::EXPONENTIAL),

		parSmoothing.initSlider("Parameter Smoothing", 1.e-6, 1, 0.01, jura::Parameter::EXPONENTIAL),
	});


	// instantiate ptr, MUST BE CALLED HERE
	for (myparams * p : paramManager)
	{
		p->managerPtr = &paramManager;
		p->instantiateParameter();
		addObservedParameter(p->ptr);
	}

	parGain.setCallback([this](double v) { chaoticaCore.setOutputAmplitude(v); });
	parOversampling.setCallback([this](double v) { chaoticaCore.setOversampling(v); });

	parStepSize.setCallback([this](double v) { chaoticaCore.osc.setStepSize(parStepSize.getSmoothedVal()); });
	parRate.setCallback([this](double v) { chaoticaCore.osc.setRate(parRate.getSmoothedVal()); });
	parChaoticaGain.setCallback([this](double v) { chaoticaCore.osc.setGain(parChaoticaGain.getSmoothedVal()); });
	parNLDrive.setCallback([this](double v) { chaoticaCore.osc.setNLDrive(parNLDrive.getSmoothedVal()); });
	par2D.setCallback([this](double v) { chaoticaCore.osc.set2D(par2D.getSmoothedVal()); });
	parDamping.setCallback([this](double v) { chaoticaCore.osc.setDamping(parDamping.getSmoothedVal()); });
	parOffset.setCallback([this](double v) { chaoticaCore.osc.setOffset(parOffset.getSmoothedVal()); });
	parQ.setCallback([this](double v) { chaoticaCore.osc.setQ(parQ.getSmoothedVal()); });
	parXRotation.setCallback([this](double v) { chaoticaCore.osc.setXRotation(parXRotation.getSmoothedVal()); });
	parYRotation.setCallback([this](double v) { chaoticaCore.osc.setYRotation(parYRotation.getSmoothedVal()); });

	parXFilter.setCallback([this](double v) { chaoticaCore.osc.x_filter.setCutoff(parXFilter.getSmoothedVal()); });
	parYFilter.setCallback([this](double v) { chaoticaCore.osc.y_filter.setCutoff(parYFilter.getSmoothedVal()); });
	parZFilter.setCallback([this](double v) { chaoticaCore.osc.z_filter.setCutoff(parZFilter.getSmoothedVal()); });

	parSawFreq.setCallback([this](double v) { chaoticaCore.saw.setFrequency(parSawFreq.getSmoothedVal()); });

	chaoticaCore.saw.setWaveform(BasicOscillator::WaveformMode::SAW);

	parSmoothing.setCallback([this](double v) { paramManager.setGlobalSmoothingAmount(v); });
}

void ChaoticaModule::setSampleRate(double v)
{
	chaoticaCore.setSampleRate(v);
}


void ChaoticaModule::setBeatsPerMinute(double newBpm)
{
	chaoticaCore.setBPM(newBpm);
}

void ChaoticaModule::processBlock(double **inOutBuffer, int numChannels, int numSamples)
{
	jassert(numChannels == 2);

	bool doApplyModulations = modulationManager.getNumConnections() != 0;

	const bool doParamSmoothing = paramManager.getNumParamsToSmooth() != 0;

	double left, right;
	for (int n = 0; n < numSamples; n++)
	{
		if (doApplyModulations) 
			modulationManager.applyModulations();

		chaoticaCore.processSampleFrame(&left, &right);

		if (inOutBuffer[0]) inOutBuffer[0][n] = left;
		if (inOutBuffer[1]) inOutBuffer[1][n] = right;

		if (doParamSmoothing)
			paramManager.doSmoothing();
	}

	paramManager.cleanup();
}
