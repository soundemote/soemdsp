#include "ChaosflyModule.h"

using namespace elan;

ChaosflyModule::ChaosflyModule(CriticalSection * lockToUse
	, jura::MetaParameterManager * metaManagerToUse)
	: BasicModule(lockToUse, metaManagerToUse),
	modulationManager(lockToUse)
{
	ScopedLock scopedLock(*lock);

	setModuleTypeName("Chaosfly");

	smoothingManager = &smootyMan; // solve nullptr crashes;

	BasicModule::setupParametersAndModules();

	jura::Parameter::setStoreDefaultValuesToXml(true);

	MIDIMASTER.setNoteChangeMode(MidiMaster::NoteChangeMode::Legato);
}

void ChaosflyModule::setupModulesAndModulation()
{
	MIDIMASTER.addMidiSlave(this);

	inOscOutput.setValuePtr(&chaosfly.inputSignal);
	modulationManager.registerModulationSource(&inOscOutput);

	modEnvModule = new jura::BreakpointModulatorAudioModule(lock, &chaosfly.modEnv);
	modEnvModule->setModuleName("ModEnv");
	modEnvModule->setModulationSourceName("ModEnv");
	addChildAudioModule(modEnvModule);
	modulationManager.registerModulationSource(modEnvModule);

	ampEnvModule = new jura::BreakpointModulatorAudioModule(lock, &chaosfly.ampEnv);
	ampEnvModule->setModuleName("AmpEnv");
	ampEnvModule->setModulationSourceName("AmpEnv");
	addChildAudioModule(ampEnvModule);
	modulationManager.registerModulationSource(ampEnvModule);

	for (size_t i = 0; i < BreakpointModulatorAudioModuleArray.size(); ++i)
	{
		BreakpointModulatorArray[i] = new rosic::BreakpointModulator();

		BreakpointModulatorAudioModuleArray[i] = new jura::BreakpointModulatorAudioModule(lock, BreakpointModulatorArray[i]);
		BreakpointModulatorAudioModuleArray[i]->setModuleName("Curve"+String(i+1));
		BreakpointModulatorAudioModuleArray[i]->setModulationSourceName("Curve "+String(i+1));
		addChildAudioModule(BreakpointModulatorAudioModuleArray[i]);
		modulationManager.registerModulationSource(BreakpointModulatorAudioModuleArray[i]);
	}
}

void ChaosflyModule::createParameters()
{
	jura::ModulatableParameter2 * p;
	
	p = new jura::ModulatableParameter2("FMD_Freq", 0, 1, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);

	p = new jura::ModulatableParameter2("FMD_Res", 0, 1, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);

	p = new jura::ModulatableParameter2("FMD_Chaos", 0, 1, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);

	p = new jura::ModulatableParameter2("FMD_Input_Clip", 0, 10, 10, jura::Parameter::LINEAR);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { chaosfly.setInputClipLevel(v); });

	p = new jura::ModulatableParameter2("FMD_Out", 0, 10, 1, jura::Parameter::LINEAR);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { outputGain = v; });

	p = new jura::ModulatableParameter2("FMD_Out_Adjust", 0, 50, 1, jura::Parameter::LINEAR);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { outputGain2 = v; });

	p = new jura::ModulatableParameter2("FMD_Overdrive", 0, 1, 0, jura::Parameter::LINEAR);
	parametersForSmoothing.push_back(p);
	addObservedParameter(p);

	p = new jura::ModulatableParameter2("Parameter_Smoothing", 0.001, 1, .01, jura::Parameter::EXPONENTIAL);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v)
	{
		for (auto & param : parametersForSmoothing)
			param->setSmoothingTime(v * 1000);

		for (auto & obj : smoothingManager->usedSmoothers)
			obj->setTimeConstantAndSampleRate(v * 1000, hostSampleRate);
	});

	p = new jura::ModulatableParameter2("Oversampling", 1, 8, 1, jura::Parameter::LINEAR, 1);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { chaosfly.setOversampling((int)v); });

	 p = new jura::ModulatableParameter2("Channel_Mode", 0, (int)channelModes.size()-1, 0, jura::Parameter::STRING);
	 addObservedParameter(p);
	 p->setValueChangeCallback([this](double v) { chaosfly.setChannelMode((int)v); });
	 for (size_t i = 0; i < channelModes.size(); ++i)
		 p->addStringValue(channelModes[i]);

	p = new jura::ModulatableParameter2("Gain", -48, 6, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { chaosfly.setOutputGainDecibels(v); });

	p = new jura::ModulatableParameter2("Bias", -3, +3, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { chaosfly.setOutputBias(v); });

	p = new jura::ModulatableParameter2("hpFilterOut", 1.e-6, 20000, 1.e-6, jura::Parameter::EXPONENTIAL);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { chaosfly.hpFilter.setCutoff(v); });

	p = new jura::ModulatableParameter2("OutputClipLevel", 0, 2, 2, jura::Parameter::LINEAR);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { chaosfly.setOutputClipLevel(v); });

	p = new jura::ModulatableParameter2("Osc1Amp", -1, +1, 1, jura::Parameter::LINEAR);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { chaosfly.setOsc1Amplitude(v); });
	p->setMapper(new jura::rsParameterMapperSinh(p->getMinValue(), p->getMaxValue(), 4));

	p = new jura::ModulatableParameter2("Osc2Amp", -1, +1, 1, jura::Parameter::LINEAR);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { chaosfly.setOsc2Amplitude(v); });
	p->setMapper(new jura::rsParameterMapperSinh(p->getMinValue(), p->getMaxValue(), 4));

	p = new jura::ModulatableParameter2("Osc_1_Bypass", 0, 1, 0, jura::Parameter::BOOLEAN);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { chaosfly.setOsc1Bypass(v > 0.5); });

	p = new jura::ModulatableParameter2("Osc_2_Bypass", 0, 1, 0, jura::Parameter::BOOLEAN);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { chaosfly.setOsc2Bypass(v > 0.5); });

	p = new jura::ModulatableParameter2("Feedback_Gain", -20, +20, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { chaosfly.feedbackGain = v; });

	p = new jura::ModulatableParameter2("Output_Osc1_Pre_Filter", 0, 1, 1, jura::Parameter::BOOLEAN);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { chaosfly.setOutputOsc1PreFilter(v > 0.5); });

	p = new jura::ModulatableParameter2("Modulate_With_Osc1_Pre_Filter", 0, 1, 0, jura::Parameter::BOOLEAN);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { chaosfly.setModulateWithOsc1ValuePreFilter(v > 0.5); });

	p = new jura::ModulatableParameter2("Tune", -72, 72, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { chaosfly.setTuneOffset(v); });

	p = new jura::ModulatableParameter2("Octave", -12, +12, 0, jura::Parameter::LINEAR, 1);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { chaosfly.setOctaveOffset(v); });

	p = new jura::ModulatableParameter2("Glide", 0, 500, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { MIDIMASTER.setGlideSeconds(v); });

	p = new jura::ModulatableParameter2("InMix", -8, +8, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { chaosfly.setInputMix(v); });

	p = new jura::ModulatableParameter2("InDetune", -24, 120, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { chaosfly.setInputFilterDetune(v); });

	p = new jura::ModulatableParameter2("Ratio", 1 / 64.0, 64, 1, jura::Parameter::LINEAR);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { chaosfly.setFrequencyRatio(v); });

	p = new jura::ModulatableParameter2("ModDepth", -10, +10, 1, jura::Parameter::LINEAR);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { chaosfly.setModulationDepthScaler(v); });

	p = new jura::ModulatableParameter2("Mod11", -20, +20, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { chaosfly.setModDepth11(v); });

	p = new jura::ModulatableParameter2("Mod22", -20, +20, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { chaosfly.setModDepth22(v); });

	p = new jura::ModulatableParameter2("Mod12", -20, +20, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { chaosfly.setModDepth12(v); });

	p = new jura::ModulatableParameter2("Mod21", -20, +20, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { chaosfly.setModDepth21(v); });

	p = new jura::ModulatableParameter2("FmVsPm11", 0, 1, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { chaosfly.setFreqVsPhaseMod11(v); });

	p = new jura::ModulatableParameter2("FmVsPm22", 0, 1, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { chaosfly.setFreqVsPhaseMod22(v); });

	p = new jura::ModulatableParameter2("FmVsPm12", 0, 1, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { chaosfly.setFreqVsPhaseMod12(v); });

	p = new jura::ModulatableParameter2("FmVsPm21", 0, 1, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { chaosfly.setFreqVsPhaseMod21(v); });

	p = new jura::ModulatableParameter2("Osc1FreqMin", -20000, 20000, -20000, jura::Parameter::LINEAR);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { chaosfly.setOsc1FreqMin(v); });

	p = new jura::ModulatableParameter2("Osc1FreqMax", -20000, 20000, 20000, jura::Parameter::LINEAR);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { chaosfly.setOsc1FreqMax(v); });

	p = new jura::ModulatableParameter2("Osc2FreqMin", -20000, 20000, -20000, jura::Parameter::LINEAR);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { chaosfly.setOsc2FreqMin(v); });

	p = new jura::ModulatableParameter2("Osc2FreqMax", -20000, 20000, 20000, jura::Parameter::LINEAR);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { chaosfly.setOsc2FreqMax(v); });

	p = new jura::ModulatableParameter2("ModEnvDepth", -4, +4, 1, jura::Parameter::LINEAR);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { chaosfly.setModEnvDepth(v); });

	p = new jura::ModulatableParameter2("Filter1CutoffScale", 0.005, 200, 100, jura::Parameter::EXPONENTIAL);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { chaosfly.setFilter1FreqScaler(v); });

	p = new jura::ModulatableParameter2("Filter1Resonance", 0, 1, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { chaosfly.setFilter1Resonance(v); });

	p = new jura::ModulatableParameter2("Filter1Mode", 0, (int)filterModes.size()-1, 1, jura::Parameter::STRING);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { chaosfly.setFilter1Mode(v > 0.5); });
	for (size_t i = 0; i < filterModes.size(); ++i)
		p->addStringValue(filterModes[i]);

	p = new jura::ModulatableParameter2("Filter2CutoffScale", 0.005, 200, 0.01, jura::Parameter::EXPONENTIAL);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { chaosfly.setFilter2FreqScaler(v); });

	p = new jura::ModulatableParameter2("Filter2Resonance", 0, 1, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { chaosfly.setFilter2Resonance(v); });

	p = new jura::ModulatableParameter2("Filter2Mode", 0, (int)filterModes.size()-1, 5, jura::Parameter::STRING);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { chaosfly.setFilter2Mode((int)v); });
	for (size_t i = 0; i < filterModes.size(); ++i)
		p->addStringValue(filterModes[i]);

	p = new jura::ModulatableParameter2("InputToFeedback", -4, 4, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { chaosfly.setInputToFeedback(v); });

	p = new jura::ModulatableParameter2("Filter3Detune", -48, 48, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { chaosfly.setFeedbackDetune(v); });

	p = new jura::ModulatableParameter2("Filter3Gain", -100, +100, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { chaosfly.setFeedbackBoostAmount(v); });

	p = new jura::ModulatableParameter2("Filter3Width", 0.25, 4, 1, jura::Parameter::EXPONENTIAL);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { chaosfly.setFeedbackBoostWidth(v); });

	p = new jura::ModulatableParameter2("OscInOctave", -12, +12, 0, jura::Parameter::LINEAR, 1);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { chaosfly.setOscInOctave(v); });

	p = new jura::ModulatableParameter2("OscInDetune", -84, +84, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { chaosfly.setInputOscDetune(v); });

	p = new jura::ModulatableParameter2("OscInAmplitude", -12, +12, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { chaosfly.setInputOscAmplitude(v); });

	p = new jura::ModulatableParameter2("OscInWave", 0, (int)inputOscSchapes.size()-1, 0, jura::Parameter::STRING);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { chaosfly.setInputOscWaveform((int)v); });
	for (size_t i = 0; i < inputOscSchapes.size(); ++i)
		p->addStringValue(inputOscSchapes[i]);

	p = new jura::ModulatableParameter2("Osc1FreqScale", -64, 64, 1, jura::Parameter::LINEAR);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { chaosfly.setFreqFactor1(v); });

	p = new jura::ModulatableParameter2("Osc1FreqOffset", -2000, +2000, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { chaosfly.setFreqOffset1(v); });

	p = new jura::ModulatableParameter2("Osc1Phase", 0, 360, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { chaosfly.setPhase1Degrees(v); });

	p = new jura::ModulatableParameter2("Osc1ClipLevel", 0, 2, 2, jura::Parameter::LINEAR);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { chaosfly.setClipLevel1(v); });

	p = new jura::ModulatableParameter2("Osc1ClipCenter", -1, 1, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { chaosfly.setClipCenter1(v); });

	p = new jura::ModulatableParameter2("Osc1Wave", 0, (int)oscillatorShapes.size()-1, 0, jura::Parameter::STRING);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { chaosfly.setWaveform1((int)v); });
	for (size_t i = 0; i < oscillatorShapes.size(); ++i)
		p->addStringValue(oscillatorShapes[i]);

	p = new jura::ModulatableParameter2("Osc1EllipseA", -1, +1, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { chaosfly.setEllipseA1(v); });

	p = new jura::ModulatableParameter2("Osc1EllipseB", -1, +1, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { chaosfly.setEllipseB1(v); });

	p = new jura::ModulatableParameter2("Osc1EllipseC", -100, +100, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { chaosfly.setEllipseC1(v); });

	p = new jura::ModulatableParameter2("Osc2FreqScale", -64, 64, 1, jura::Parameter::LINEAR);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { chaosfly.setFreqFactor2(v); });

	p = new jura::ModulatableParameter2("Osc2FreqOffset", -2000, +2000, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { chaosfly.setFreqOffset2(v); });

	p = new jura::ModulatableParameter2("Osc2Phase", 0, 360, 90, jura::Parameter::LINEAR);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { chaosfly.setPhase2Degrees(v); });

	p = new jura::ModulatableParameter2("Osc2ClipLevel", 0, 2, 2, jura::Parameter::LINEAR);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { chaosfly.setClipLevel2(v); });

	p = new jura::ModulatableParameter2("Osc2ClipCenter", -1, 1, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { chaosfly.setClipCenter2(v); });

	p = new jura::ModulatableParameter2("Osc2Wave", 0, (int)oscillatorShapes.size() - 1, 0, jura::Parameter::STRING);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { chaosfly.setWaveform2((int)v); });
	for (size_t i = 0; i < oscillatorShapes.size(); ++i)
		p->addStringValue(oscillatorShapes[i]);

	p = new jura::ModulatableParameter2("Osc2EllipseA", -10, +10, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { chaosfly.setEllipseA2(v); });

	p = new jura::ModulatableParameter2("Osc2EllipseB", -10, +10, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { chaosfly.setEllipseB2(v); });

	p = new jura::ModulatableParameter2("Osc2EllipseC", -10, +10, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { chaosfly.setEllipseC2(v); });

	p = new jura::ModulatableParameter2("SyncThresh", 0.001, 2, 2, jura::Parameter::EXPONENTIAL);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { chaosfly.setSyncThreshold(v); });

	p = new jura::ModulatableParameter2("SyncAmount", 0, 1, 1, jura::Parameter::LINEAR);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { chaosfly.setSyncAmount(v); });

	p = new jura::ModulatableParameter2("SyncFilters", 0, 1, 1, jura::Parameter::BOOLEAN);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { chaosfly.setFilterResetOnSync(v > 0.5); });

	p = new jura::ModulatableParameter2("SyncMode", 0, (int)syncModes.size()-1, 0, jura::Parameter::STRING);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { chaosfly.setSyncMode((int)v); });
	for (size_t i = 0; i < syncModes.size(); ++i)
		p->addStringValue(syncModes[i]);

	p = new jura::ModulatableParameter2("Bypass_Ducking", 0, 1, 0, jura::Parameter::BOOLEAN);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { chaosfly.setDuckBypass(v > 0.5); });

	p = new jura::ModulatableParameter2("DuckRange", 0.001, 4, 4, jura::Parameter::EXPONENTIAL);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { chaosfly.setDuckingRange(v); });

	p = new jura::ModulatableParameter2("DuckCenter", -1, +1, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { chaosfly.setDuckingCenter(v); });

	p = new jura::ModulatableParameter2("DuckFlatness", 0, 1, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { chaosfly.setDuckingFlatness(v); });

	p = new jura::ModulatableParameter2("DuckShape", 0, (int)duckShapes.size()-1, 0, jura::Parameter::STRING);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { chaosfly.setDuckingShape((int)v); });
	for (size_t i = 0; i < duckShapes.size(); ++i)
		p->addStringValue(duckShapes[i]);

	p = new jura::ModulatableParameter2("DuckFill", 0, 2, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { chaosfly.setDuckingFill(v); });

	p = new jura::ModulatableParameter2("Noise_Amp", 0, 10, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { chaosfly.noiseAmp = v; });

	p = new jura::ModulatableParameter2("Noise_LPF", .01, 24000, 24000, jura::Parameter::LINEAR);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { chaosfly.noiseFilter.setLowpassCutoff(v); });

	p = new jura::ModulatableParameter2("Noise_HPF", .01, 24000, .01, jura::Parameter::EXPONENTIAL);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { chaosfly.noiseFilter.setHighpassCutoff(v); });

	p = new jura::ModulatableParameter2("Attack", 1, 1000, 10, jura::Parameter::EXPONENTIAL);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { chaosfly.compressor.setAttackTime(v); });

	p = new jura::ModulatableParameter2("Release", 10, 1000, 100, jura::Parameter::EXPONENTIAL);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { chaosfly.compressor.setReleaseTime(v); });

	p = new jura::ModulatableParameter2("LookAhead", 0, 50, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { chaosfly.compressor.setLookAheadTime(v); });

	p = new jura::ModulatableParameter2("InLevel", -24, 24, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { chaosfly.compressor.setInputGain(v); });

	p = new jura::ModulatableParameter2("OutLevel", -24, 48, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { chaosfly.compressor.setOutputGain(v); });

	p = new jura::ModulatableParameter2("Threshold", -48, 12, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { chaosfly.compressor.setThreshold(v);  });

	p = new jura::ModulatableParameter2("CompRatio", 1, 40, 1, jura::Parameter::EXPONENTIAL);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { chaosfly.compressor.setRatio(v); });

	p = new jura::ModulatableParameter2("KneeWidth", 0, 48, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { chaosfly.compressor.setKneeWidth(v); });

	p = new jura::ModulatableParameter2("Limit", 0, 1, 0, jura::Parameter::BOOLEAN);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { chaosfly.compressor.setLimiterMode(v > 0.5); });

	p = new jura::ModulatableParameter2("AutoGain", 0, 1, 0, jura::Parameter::BOOLEAN);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { chaosfly.compressor.setAutoGain(v > 0.5); });

	p = new jura::ModulatableParameter2("Dry_Wet", 0, 1, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { chaosfly.compressor.setDryWetRatio(v); });

	p = new jura::ModulatableParameter2("osc1ResetMode", 0, int(ResetMode::numModes) - 1, int(ResetMode::Never), jura::Parameter::STRING);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { osc1ResetMode = (ResetMode)(int)v; });
	p->addStringValue("osc1Reset: Never");
	p->addStringValue("osc1Reset: Always");
	p->addStringValue("osc1Reset: Legato");		

	p = new jura::ModulatableParameter2("osc2ResetMode", 0, int(ResetMode::numModes) - 1, int(ResetMode::Never), jura::Parameter::STRING);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { osc2ResetMode = (ResetMode)(int)v; });
	p->addStringValue("osc2Reset: Never");
	p->addStringValue("osc2Reset: Always");
	p->addStringValue("osc2Reset: Legato");

	p = new jura::ModulatableParameter2("ampEnvResetMode", 0, int(ResetMode::numModes) - 1, int(ResetMode::Always), jura::Parameter::STRING);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { ampEnvResetMode = (ResetMode)(int)v; });
	p->addStringValue("ampEnvReset: Never");
	p->addStringValue("ampEnvReset: Always");
	p->addStringValue("ampEnvReset: Legato");

	p = new jura::ModulatableParameter2("modEnvResetMode", 0, int(ResetMode::numModes) - 1, int(ResetMode::Always), jura::Parameter::STRING);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { modEnvResetMode = (ResetMode)(int)v; });
	p->addStringValue("modEnvReset: Never");
	p->addStringValue("modEnvReset: Always");
	p->addStringValue("modEnvReset: Legato");

	p = new jura::ModulatableParameter2("ampEnvTrigMode", 0, int(TrigMode::numModes) - 1, int(TrigMode::Always), jura::Parameter::STRING);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { ampEnvTrigMode = (TrigMode)(int)v; });
	p->addStringValue("ampEnvTrig: Always");
	p->addStringValue("ampEnvTrig: Legato");

	p = new jura::ModulatableParameter2("modEnvTrigMode", 0, int(TrigMode::numModes) - 1, int(TrigMode::Always), jura::Parameter::STRING);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { modEnvTrigMode = (TrigMode)(int)v; });
	p->addStringValue("modEnvTrig: Always");
	p->addStringValue("modEnvTrig: Legato");

	p = new jura::ModulatableParameter2("glideMode", 0, MidiMaster::GlideModeMenuItems.size() - 1, int(MidiMaster::GlideMode::when_doing_legato), jura::Parameter::STRING);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { MIDIMASTER.setGlideMode((MidiMaster::GlideMode)(int)v); });
	p->addStringValue("glideMode: Legato");
	p->addStringValue("glideMode: Always");
}

void ChaosflyModule::setStateFromXml(const juce::XmlElement & xmlState, const juce::String & stateName, bool markAsClean)
{
	chaosfly.reset();
	BasicModule::setStateFromXml(xmlState, stateName, markAsClean);		
}

void ChaosflyModule::setSampleRate(double v)
{
	hostSampleRate = v;
  chaosfly.setSampleRate(v);

	for (auto & obj : childModules)
		obj->setSampleRate(v);
}

void ChaosflyModule::triggerPitchChange()
{
	chaosfly.setPitch(MIDIMASTER.getState().noteSmoothed);
}

void ChaosflyModule::allNotesOff()
{
	ampEnvModule->noteOff(false);
	modEnvModule->noteOff(false);
}

void ChaosflyModule::triggerAttack()
{
	ampEnvModule->noteOn(ampEnvResetMode == ResetMode::Never, MIDIMASTER.getState().note, int(MIDIMASTER.getState().velocity * 127));
	modEnvModule->noteOn(modEnvResetMode == ResetMode::Never, MIDIMASTER.getState().note, int(MIDIMASTER.getState().velocity * 127));

	if (osc1ResetMode != ResetMode::Never)
		chaosfly.syncOscillators(1);
	if (osc2ResetMode != ResetMode::Never)
		chaosfly.syncOscillators(2);
}

void ChaosflyModule::triggerNoteOnLegato()
{
	if (ampEnvTrigMode != TrigMode::Legato)
		ampEnvModule->noteOn(ampEnvResetMode == ResetMode::Never, MIDIMASTER.getState().note, int(MIDIMASTER.getState().velocity * 127));
	if (modEnvTrigMode != TrigMode::Legato)
		modEnvModule->noteOn(modEnvResetMode == ResetMode::Never, MIDIMASTER.getState().note, int(MIDIMASTER.getState().velocity * 127));

	if (osc1ResetMode == ResetMode::Always)
		chaosfly.syncOscillators(1);
	if (osc2ResetMode == ResetMode::Always)
		chaosfly.syncOscillators(2);
}

void ChaosflyModule::triggerNoteOffLegato()
{
	triggerNoteOnLegato();
}

void ChaosflyModule::triggerRelease()
{
	ampEnvModule->noteOff(MIDIMASTER.getState().note);
	modEnvModule->noteOff(MIDIMASTER.getState().note);
}

void ChaosflyModule::processBlock(double **inOutBuffer, int /*numChannels*/, int numSamples)
{
	const bool doApplyModulations = modulationManager.getNumConnections() != 0;

	const bool doApplySmoothing = smoothingManager->needsSmoothing();

	double left = 0, right = 0;
  for(int n = 0; n < numSamples; n++)
  {
		const bool isSilent = chaosfly.isSilent();

		if (doApplySmoothing)
			smoothingManager->updateSmoothedValues();

		if (!isSilent && doApplyModulations)
			modulationManager.applyModulations();

    if (inOutBuffer[0]) left = inOutBuffer[0][n];
		if (inOutBuffer[1]) right = inOutBuffer[1][n];

		MIDIMASTER.incrementPitchGlide();

		if (!isSilent)
			chaosfly.processSampleFrame(&left, &right, &left, &right);

		if (inOutBuffer[0]) inOutBuffer[0][n] = left * outputGain * outputGain2;
		if (inOutBuffer[1]) inOutBuffer[1][n] = right * outputGain * outputGain2;
  }
}
