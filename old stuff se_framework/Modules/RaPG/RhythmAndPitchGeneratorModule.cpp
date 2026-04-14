#include "RhythmAndPitchGeneratorModule.h"

void RaPG_EnvelopeModule::createParameters()
{
  ModulatableParameter2 * p;

	p = new ModulatableParameter2("attack", 0.001, 10, 0.003, Parameter::EXPONENTIAL);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { envelope.setAttack(v); });
	p->setDefaultModParameters
	(
		0,
		100,
		-p->getMaxValue(),
		+p->getMaxValue(),
		ModulationConnection::modModes::ABSOLUTE,
		1
	);

	p = new ModulatableParameter2("decay", 0.001, 10, .25, Parameter::EXPONENTIAL);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { envelope.setDecay(v); });
	p->setDefaultModParameters
	(
		0,
		100,
		-p->getMaxValue(),
		+p->getMaxValue(),
		ModulationConnection::modModes::ABSOLUTE,
		1
	);

	p = new ModulatableParameter2("sustain", 0, 1, 0, Parameter::LINEAR);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { envelope.setSustainLevel(v); });
	p->setDefaultModParameters
	(
		0,
		1,
		-1,
		1,
		ModulationConnection::modModes::ABSOLUTE,
		1
	);

	p = new ModulatableParameter2("release", 0.001, 10, .1, Parameter::EXPONENTIAL);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { envelope.setRelease(v); });
	p->setDefaultModParameters
	(
		0,
		100,
		-p->getMaxValue(),
		+p->getMaxValue(),
		ModulationConnection::modModes::ABSOLUTE,
		1
	);

	p = new ModulatableParameter2("attackCurve", -10, 10, 1, Parameter::LINEAR);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { envelope.setAttackCurve(v); });
	p->setDefaultModParameters
	(
		-p->getMaxValue(),
		+p->getMaxValue(),
		-p->getMaxValue(),
		+p->getMaxValue(),
		ModulationConnection::modModes::ABSOLUTE,
		1
	);

	p = new ModulatableParameter2("decayCurve", -10, 10, 1, Parameter::LINEAR);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { envelope.setDecayCurve(v); });
	p->setDefaultModParameters
	(
		-p->getMaxValue(),
		+p->getMaxValue(),
		-p->getMaxValue(),
		+p->getMaxValue(),
		ModulationConnection::modModes::ABSOLUTE,
		1
	);

	p = new ModulatableParameter2("releaseCurve", -10, 10, 0, Parameter::LINEAR);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { envelope.setReleaseCurve(v); });
	p->setDefaultModParameters
	(
		-p->getMaxValue(),
		+p->getMaxValue(),
		-p->getMaxValue(),
		+p->getMaxValue(),
		ModulationConnection::modModes::ABSOLUTE,
		1
	);

	p = new ModulatableParameter2("velocityMod", 0, 1, 1.0/3.0, Parameter::LINEAR);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { ghostAmplitude = 1-v; });
	p->setDefaultModParameters
	(
		0,
		1,
		-1,
		+1,
		ModulationConnection::modModes::ABSOLUTE,
		1
	);

	p = new ModulatableParameter2("triggerOnNote", 0, 1, 0, Parameter::BOOLEAN);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { triggerOnNote = v > 0.0; });
	p->setDefaultModParameters
	(
		0,
		1,
		-2,
		+2,
		ModulationConnection::modModes::ABSOLUTE,
		1
	);

	p = new ModulatableParameter2("resetOnNote", 0, 1, 0, Parameter::BOOLEAN);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { resetOnNote = v > 0.0; });
	p->setDefaultModParameters
	(
		0,
		1,
		-2,
		+2,
		ModulationConnection::modModes::ABSOLUTE,
		1
	);

	p = new ModulatableParameter2("triggerOnClick", 0, 1, 1, Parameter::BOOLEAN);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { triggerOnClick = v > 0.0; });
	p->setDefaultModParameters
	(
		0,
		1,
		-2,
		+2,
		ModulationConnection::modModes::ABSOLUTE,
		1
	);

	p = new ModulatableParameter2("resetOnClick", 0, 1, 0, Parameter::BOOLEAN);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { resetOnClick = v > 0.0; });
	p->setDefaultModParameters
	(
		0,
		1,
		-2,
		+2,
		ModulationConnection::modModes::ABSOLUTE,
		1
	);

	p = new ModulatableParameter2("triggerOnMasterReset", 0, 1, 0, Parameter::BOOLEAN);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { triggerOnMasterReset = v > 0.0; });
	p->setDefaultModParameters
	(
		0,
		1,
		-2,
		+2,
		ModulationConnection::modModes::ABSOLUTE,
		1
	);

	p = new ModulatableParameter2("resetOnMasterReset", 0, 1, 0, Parameter::BOOLEAN);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { resetOnMasterReset = v > 0.0; });
	p->setDefaultModParameters
	(
		0,
		1,
		-2,
		+2,
		ModulationConnection::modModes::ABSOLUTE,
		1
	);
}

void RaPG_PitchModule::createParameters()
{
	ModulatableParameter2 * p;

	p = new ModulatableParameter2("enable", 0, 1, 1, Parameter::BOOLEAN);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { pitchQuantizer.setIsEnabled(v > 0.0); });
	p->setDefaultModParameters
	(
		0,
		1,
		-2,
		+2,
		ModulationConnection::modModes::ABSOLUTE,
		1
	);

	p = new ModulatableParameter2("seed", 0, 999, 0, Parameter::LINEAR, 1);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { pitchQuantizer.setSeed((int)v); });
	p->setDefaultModParameters
	(
		0,
		10000,
		-100,
		+100,
		ModulationConnection::modModes::ABSOLUTE,
		3
	);

	p = new ModulatableParameter2("steps", 0, 128, 0, Parameter::LINEAR, 1);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { pitchQuantizer.setSteps((int)v); });
	p->setDefaultModParameters
	(
		p->getMinValue(),
		10000,
		-100,
		+100,
		ModulationConnection::modModes::ABSOLUTE,
		1
	);

	p = new ModulatableParameter2("divide", 1, 128, 4, Parameter::LINEAR, 1);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { pitchQuantizer.setClockDivide((int)v); });
	p->setDefaultModParameters
	(
		p->getMinValue(),
		1280,
		-128,
		+128,
		ModulationConnection::modModes::ABSOLUTE,
		4
	);

	p = new ModulatableParameter2("noteLengthMod", 0, 2, 1, Parameter::LINEAR);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { pitchQuantizer.setNoteLengthMod(v); });
	p->setDefaultModParameters
	(
		p->getMinValue(),
		2,
		-2,
		+2,
		ModulationConnection::modModes::ABSOLUTE,
		1
	);

	p = new ModulatableParameter2("octaveAmp", 0, 8, 1, Parameter::LINEAR, 0);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { pitchQuantizer.setOctaveAmplitude(v); });
	p->setDefaultModParameters
	(
		p->getMinValue(),
		12,
		-24,
		+24,
		ModulationConnection::modModes::ABSOLUTE,
		p->getMaxValue() * 0.5
	);

	p = new ModulatableParameter2("octaveOffset", -8, 8, 0, Parameter::LINEAR, 0);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { pitchQuantizer.setOctaveOffset(v); });
	p->setDefaultModParameters
	(
		-100,
		+100,
		-p->getMaxValue(),
		+p->getMaxValue(),
		ModulationConnection::modModes::ABSOLUTE,
		p->getMaxValue() * 0.5
	);

	p = new ModulatableParameter2("bipolar", 0, 1, 0, Parameter::BOOLEAN);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { pitchQuantizer.setIsUnipolar(v <= 0.0); });
	p->setDefaultModParameters
	(
		0,
		1,
		-2,
		+2,
		ModulationConnection::modModes::ABSOLUTE,
		1
	);
}

RaPG_ClockModule::RaPG_ClockModule(CriticalSection * lockToUse, jura::MetaParameterManager * metaManagerToUse, RaPG_RhythmGenerator * masterRhythmGen)
	: BasicModule(lockToUse, metaManagerToUse)
	, masterRhythmGen(masterRhythmGen)
	, clock(masterRhythmGen)
{
	BasicModule::setupParametersAndModules();
}

void RaPG_ClockModule::setSampleRate(double v)
{
	clock.setSampleRate(v);
}

void RaPG_ClockModule::createParameters()
{
	ModulatableParameter2 * p;

	p = new ModulatableParameter2("divide", 1, 128, 1, Parameter::LINEAR, 1);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { clock.setDivision((int)v); });
	p->setDefaultModParameters
	(
		p->getMinValue(),
		1280,
		-128,
		+128,
		ModulationConnection::modModes::ABSOLUTE,
		4
	);

	p = new ModulatableParameter2("pulse1time", 0, 1, 0, Parameter::LINEAR);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { clock.setPrimaryPulseDelayNormalized(v); });
	p->setDefaultModParameters
	(
		0,
		1,
		-2,
		+2,
		ModulationConnection::modModes::ABSOLUTE,
		2
	);


	p = new ModulatableParameter2("pulse2time", 0, 1, 0, Parameter::LINEAR);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { clock.setSecondaryPulseDelayNormalized(v); });
	p->setDefaultModParameters
	(
		0,
		1,
		-2,
		+2,
		ModulationConnection::modModes::ABSOLUTE,
		2
	);

	p = new ModulatableParameter2("enable", 0, 1, 1, Parameter::BOOLEAN);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { clock.setIsEnabled(v > 0.0); });
	p->setDefaultModParameters
	(
		0,
		1,
		-2,
		+2,
		ModulationConnection::modModes::ABSOLUTE,
		1
	);
}

void RaPG_RhythmModule::createParameters()
{
	ModulatableParameter2 * p;

	p = new ModulatableParameter2("enable", 0, 1, 1, Parameter::BOOLEAN);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { rhythmGen.setIsEnabled(v > 0.0); });
	p->setDefaultModParameters
	(
		0,
		1,
		-2,
		+2,
		ModulationConnection::modModes::ABSOLUTE,
		1
	);

	p = new ModulatableParameter2("seed", 0, 999, 0, Parameter::LINEAR, 1);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { rhythmGen.setSeed((int)v); });
	p->setDefaultModParameters
	(
		0,
		10000,
		-100,
		+100,
		ModulationConnection::modModes::ABSOLUTE,
		3
	);

	p = new ModulatableParameter2("steps", 0, 128, 0, Parameter::LINEAR, 1);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { rhythmGen.setSteps((int)v); });
	p->setDefaultModParameters
	(
		p->getMinValue(),
		1280,
		-100,
		+100,
		ModulationConnection::modModes::ABSOLUTE,
		1
	);

	p = new ModulatableParameter2("divide", 1, 128, 4, Parameter::LINEAR, 1);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { rhythmGen.setMasterClockDivision((int)v); });
	p->setDefaultModParameters
	(
		p->getMinValue(),
		1280,
		-128,
		+128,
		ModulationConnection::modModes::ABSOLUTE,
		4
	);

	p = new ModulatableParameter2("manual", -7, 7, 0, Parameter::LINEAR);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { rhythmGen.setOffset(v); });
	p->setDefaultModParameters
	(
		-14,
		+14,
		-14,
		+14,
		ModulationConnection::modModes::ABSOLUTE,
		4
	);

	p = new ModulatableParameter2("enableGhostClicks", 0, 1, 0, Parameter::BOOLEAN);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { rhythmGen.setEnableGhostClicks(v > 0.0); });
	p->setDefaultModParameters
	(
		0,
		1,
		-2,
		+2,
		ModulationConnection::modModes::ABSOLUTE,
		1
	);
}

void RaPG_OscillatorModule::createParameters()
{
	ModulatableParameter2 * p;

	p = new ModulatableParameter2("phase", -1, 1, 0, Parameter::LINEAR);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { oscillator.setPhaseOffset(v); });
	p->setDefaultModParameters
	(
		-1000,
		+1000,
		-2,
		+2,
		ModulationConnection::modModes::ABSOLUTE,
		0.5
	);

	p = new ModulatableParameter2("frequency", -3000, +3000, 0, Parameter::LINEAR);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { oscillator.setFrequencyMod(v); });
	p->setMapper(new jura::rsParameterMapperSinh(p->getMinValue(), p->getMaxValue(), 4));
	p->setDefaultModParameters
	(
		-20000,
		+20000,
		-5000,
		+5000,
		ModulationConnection::modModes::ABSOLUTE,
		50
	);

	p = new ModulatableParameter2("pitch", -72, 72, 0, Parameter::LINEAR);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v)
	{
		oscillator.setPitchMod(v);
	});
	p->setDefaultModParameters
	(
		-128,
		+128,
		-128,
		+128,
		ModulationConnection::modModes::ABSOLUTE,
		1
	);

	p = new ModulatableParameter2("amplitude", 0, 1, 0.5, Parameter::LINEAR);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { oscillator.setAmplitude(v); });
	p->setDefaultModParameters
	(
		-2,
		+2,
		-2,
		+2,
		ModulationConnection::modModes::ABSOLUTE,
		.5
	);

	p = new ModulatableParameter2("saw", -1, 1, 0, Parameter::LINEAR);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { oscillator.setAsymmetrical(v); });
	p->setMapper(new jura::rsParameterMapperTanh(-1, 1, 2));
	p->setDefaultModParameters
	(
		-1,
		+1,
		-4,
		+4,
		ModulationConnection::modModes::ABSOLUTE,
		.5
	);

	p = new ModulatableParameter2("spike", -1, 1, 0, Parameter::LINEAR);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { oscillator.setBendSymmetrical(v); });
	p->setMapper(new jura::rsParameterMapperTanh(-.99999, .99999, 5));
	p->setDefaultModParameters
	(
		-1,
		+1,
		-4,
		+4,
		ModulationConnection::modModes::ABSOLUTE,
		.5
	);

	p = new ModulatableParameter2("square", -1, 1, 0, Parameter::LINEAR);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { oscillator.setBendOffset(v); });
	p->setMapper(new jura::rsParameterMapperTanh(-.99999, .99999, 5));
	p->setDefaultModParameters
	(
		-1,
		+1,
		-4,
		+4,
		ModulationConnection::modModes::ABSOLUTE,
		.5
	);

	p = new ModulatableParameter2("sine", -1, 1, 1, Parameter::LINEAR);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { oscillator.setSigmoidSymmetrical(v); });
	p->setDefaultModParameters
	(
		-1,
		+1,
		-4,
		+4,
		ModulationConnection::modModes::ABSOLUTE,
		.5
	);

	p = new ModulatableParameter2("knee", -1, 1, 0, Parameter::LINEAR);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { oscillator.setSigmoidOffset(v); });
	p->setDefaultModParameters
	(
		-1,
		+1,
		-4,
		+4,
		ModulationConnection::modModes::ABSOLUTE,
		.5
	);

	p = new ModulatableParameter2("lpCut", .1, 2200, 2200, Parameter::EXPONENTIAL);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { oscillator.setFilterFrequency(v); });
	p->setDefaultModParameters
	(
		0,
		2000,
		-2000,
		+2000,
		ModulationConnection::modModes::ABSOLUTE,
		100
	);

	p = new ModulatableParameter2("ampFilterEnvMod", -1, 1, 0, Parameter::LINEAR);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { ampFilterEnvMod = v; });
	p->setMapper(new jura::rsParameterMapperTanh(-1, 1, 3));
	p->setDefaultModParameters
	(
		-1,
		+1,
		-4,
		+4,
		ModulationConnection::modModes::ABSOLUTE,
		.5
	);

	p = new ModulatableParameter2("resetOnNote", 0, 1, 0, Parameter::BOOLEAN);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { resetOnNote = v > 0.0; });
	p->setDefaultModParameters
	(
		0,
		1,
		-4,
		+4,
		ModulationConnection::modModes::ABSOLUTE,
		1
	);

	p = new ModulatableParameter2("resetOnClick", 0, 1, 0, Parameter::BOOLEAN);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { resetOnClick = v > 0.0; });
	p->setDefaultModParameters
	(
		0,
		1,
		-2,
		+2,
		ModulationConnection::modModes::ABSOLUTE,
		1
	);

	p = new ModulatableParameter2("resetOnMasterReset", 0, 1, 0, Parameter::BOOLEAN);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { resetOnMasterReset = v > 0.0; });
	p->setDefaultModParameters
	(
		0,
		1,
		-2,
		+2,
		ModulationConnection::modModes::ABSOLUTE,
		1
	);

	p = new ModulatableParameter2("glideSpeed", 0.0001, 2, 0.0001, Parameter::EXPONENTIAL);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { oscillator.setGlideSpeed(v); });
	p->setDefaultModParameters
	(
		0,
		+1,
		-2,
		+2,
		ModulationConnection::modModes::ABSOLUTE,
		.5
	);
}

RhythmAndPitchGeneratorModule::RhythmAndPitchGeneratorModule(CriticalSection *lockToUse, jura::MetaParameterManager * metaManagerToUse)
	: BasicModule(lockToUse, metaManagerToUse), modulationManager(lockToUse)
{
  ScopedLock scopedLock(*lock);

	setModuleNameString("RhythmAndPitchGenerator", "RhythmAndPitchGenerator");
	setModuleWebsite("https://www.soundemote.com/labs#rapg");
	PluginFileManager::setModuleVersionString("1.1.0");

	setActiveDirectory(PluginFileManager::getFactoryPresetFolder());

	masterTimer.stopTimer();
	dcFilter.setMode(OnePoleFilter::HIGHPASS_BLT);
	dcFilter.setCutoff(5);

	Parameter::setStoreDefaultValuesToXml(true);

	setupModulesAndModulation();
	createParameters();

	setupCallbacks();
	setInitPatchValues();
	setHelpTextAndLabels();

	callParameterCallbacks(true);

	//loadFile(File(initPresetPath));
}

void RhythmAndPitchGeneratorModule::setupModulesAndModulation()
{
	MIDIMASTER.addMidiSlave(this, MidiMaster::AddFor::HostBPMChange);
	MIDIMASTER.addMidiSlave(this, MidiMaster::AddFor::MasterRateChange);
	MIDIMASTER.addMidiSlave(this, MidiMaster::AddFor::MonoNoteCallbacks);
	MIDIMASTER.addMidiSlave(this, MidiMaster::AddFor::NoteOnOff);

	setSmoothingManager(&smootyMan);
	setModulationManager(&modulationManager);
	modulationManager.setMetaParameterManager(metaParamManager);

	envelopeModule = new RaPG_EnvelopeModule(lock, metaParamManager);
	envelopeModule->setModulationManager(&modulationManager);
	envelopeModule->setModuleName("Envelope");
	addChildAudioModule(envelopeModule);

	oscillatorModule = new RaPG_OscillatorModule(lock, metaParamManager);
	oscillatorModule->setModulationManager(&modulationManager);
	oscillatorModule->setModuleName("Oscillator");
	addChildAudioModule(oscillatorModule);

	pitchModule = new RaPG_PitchModule(lock, metaParamManager);
	pitchModule->setModulationManager(&modulationManager);
	pitchModule->setModuleName("Pitch");
	addChildAudioModule(pitchModule);

	rhythmModule = new RaPG_RhythmModule(lock, metaParamManager);
	rhythmModule->setModulationManager(&modulationManager);
	rhythmModule->setModuleName("Rhythm");
	addChildAudioModule(rhythmModule);

	oscSource.setValuePtr(&oscillatorModule->oscillator.triSawValue);
	oscFiltered.setValuePtr(&oscillatorModule->oscillator.filteredTriSawValue);
	oscFltAmp.setValuePtr(&oscillatorModule->oscillator.filteredAmplifiedTriSawValue);
	oscFinalOut.setValuePtr(&oscillatorFinalOutValue);
	envSource.setValuePtr(&envelopeOut);
	pitchNoiseSource.setValuePtr(&pitchModule->pitchQuantizer.noiseValue);
	rhythmClockIndex.setValuePtr(&rhythmModule->rhythmGen.clockIndexValueForOutput);
	rhythmNoiseSource.setValuePtr(&rhythmModule->rhythmGen.noiseValue);
	rhythmClickSource.setValuePtr(&envelopeModule->ghostAmplitude);

	registerModulationSource(&oscSource);
	registerModulationSource(&oscFiltered);
	registerModulationSource(&oscFltAmp);
	registerModulationSource(&oscFinalOut);
	registerModulationSource(&envSource);
	registerModulationSource(&pitchNoiseSource);
	registerModulationSource(&rhythmClockIndex);
	registerModulationSource(&rhythmNoiseSource);
	registerModulationSource(&rhythmClickSource);

	parametersForSmoothing.push_back(dynamic_cast<ModulatableParameter2 *>(oscillatorModule->getParameterByName("phase")));
	parametersForSmoothing.push_back(dynamic_cast<ModulatableParameter2 *>(oscillatorModule->getParameterByName("frequency")));
	parametersForSmoothing.push_back(dynamic_cast<ModulatableParameter2 *>(oscillatorModule->getParameterByName("pitch")));
	parametersForSmoothing.push_back(dynamic_cast<ModulatableParameter2 *>(oscillatorModule->getParameterByName("amplitude")));
	parametersForSmoothing.push_back(dynamic_cast<ModulatableParameter2 *>(oscillatorModule->getParameterByName("saw")));
	parametersForSmoothing.push_back(dynamic_cast<ModulatableParameter2 *>(oscillatorModule->getParameterByName("spike")));
	parametersForSmoothing.push_back(dynamic_cast<ModulatableParameter2 *>(oscillatorModule->getParameterByName("square")));
	parametersForSmoothing.push_back(dynamic_cast<ModulatableParameter2 *>(oscillatorModule->getParameterByName("sine")));
	parametersForSmoothing.push_back(dynamic_cast<ModulatableParameter2 *>(oscillatorModule->getParameterByName("knee")));
	parametersForSmoothing.push_back(dynamic_cast<ModulatableParameter2 *>(oscillatorModule->getParameterByName("lpCut")));
	parametersForSmoothing.push_back(dynamic_cast<ModulatableParameter2 *>(pitchModule->getParameterByName("octaveOffset")));

	masterTimer.triggerFunction = [this]()
	{
		rhythmModule->rhythmGen.trigger();
		pitchModule->pitchQuantizer.triggerClock();
	};

	resetBarsTimer.triggerFunction = [this]()
	{
		if (globalResetBars > 0.001)
		{
			masterTimer.reset();
			rhythmModule->reset();
			rhythmModule->trigger();
			pitchModule->reset();
			pitchModule->trigger();

			if (envelopeModule->triggerOnMasterReset)
				envelopeModule->envelope.triggerOn(envelopeModule->resetOnMasterReset);

			if (oscillatorModule->resetOnMasterReset)
				oscillatorModule->reset();

			if (oscillatorModule->resetOnMasterReset)
				oscillatorModule->reset();
		}
	};

	rhythmModule->rhythmGen.triggerFunction = [this]()
	{
		if (envelopeModule->triggerOnClick)
		{
			envelopeModule->envelope.setAmplitudeScale(1.0);
			envelopeModule->envelope.triggerOn(envelopeModule->resetOnClick);
		}

		sendChangeMessage();

		if (oscillatorModule->resetOnClick)
			oscillatorModule->reset();
	};

	rhythmModule->rhythmGen.ghostTriggerFunction = [this]()
	{
		if (envelopeModule->triggerOnClick)
		{
			envelopeModule->envelope.setAmplitudeScale(envelopeModule->ghostAmplitude);
			envelopeModule->envelope.triggerOn();
		}
	};
	pitchModule->pitchQuantizer.newNoteFunction = [this]()
	{
		if (envelopeModule->triggerOnNote)
		{
			if (!envelopeModule->triggerOnClick)
				envelopeModule->envelope.setAmplitudeScale(1.0);
			envelopeModule->envelope.triggerOn(envelopeModule->resetOnNote);
		}

		oscillatorModule->oscillator.setNote(pitchModule->pitchQuantizer.getPitch(), false);

		if (oscillatorModule->resetOnNote)
			oscillatorModule->reset();
	};
	pitchModule->pitchQuantizer.legatoNoteFunction = [this]()
	{
		if (envelopeModule->triggerOnNote)
		{
			if (!envelopeModule->triggerOnClick)
				envelopeModule->envelope.setAmplitudeScale(1.0);
			envelopeModule->envelope.triggerOn(envelopeModule->resetOnNote);
		}

		oscillatorModule->oscillator.setNote(pitchModule->pitchQuantizer.getPitch(), true);
	};
	pitchModule->pitchQuantizer.noteOffFunction = [this]()
	{
		envelopeModule->envelope.triggerOff();
	};
}

void RhythmAndPitchGeneratorModule::createParameters()
{
	ModulatableParameter2 * p;

	p = new ModulatableParameter2("division", 0, 27, 21, Parameter::LINEAR, 1);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v)
	{
		bars = elan::indexToTimeSigValue(v);
		recalculateMasterTimerFrequency();
	});
	p->setDefaultModParameters
	(
		0,
		27,
		-27,
		+27,
		ModulationConnection::modModes::ABSOLUTE,
		3
	);

	p = new ModulatableParameter2("globalResetBars", 0.001, 64, 2, Parameter::EXPONENTIAL);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v)
	{
		globalResetBars = v;
		recalculateResetBarTimerFrequency();
	});
	p->setDefaultModParameters
	(
		1.e-6,
		1280,
		-128,
		+128,
		ModulationConnection::modModes::ABSOLUTE,
		2
	);

	p = new ModulatableParameter2("masterRate", .01, 500, 1, Parameter::EXPONENTIAL);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v)
	{
		masterRate = v;
		masterRateNeedsUpdating = true;
	});
	p->setDefaultModParameters
	(
		p->getMinValue(),
		p->getMaxValue(),
		-100,
		+100,
		ModulationConnection::modModes::ABSOLUTE,
		10
	);

	parametersForSmoothing.push_back(dynamic_cast<ModulatableParameter2 *>(getParameterByName("masterRate")));

	p = new ModulatableParameter2("pitchMod", 0, 1, 1, Parameter::BOOLEAN);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v)
	{
		doesMasterRateControlPitch = v > 0.0;
		oscillatorModule->oscillator.informMasterRate(masterRate, doesMasterRateControlPitch);
	});
	p->setDefaultModParameters
	(
		0,
		1,
		-2,
		+2,
		ModulationConnection::modModes::ABSOLUTE,
		1
	);

	p = new ModulatableParameter2("masterVolume", 0, 2, 1, Parameter::LINEAR);
	parametersForSmoothing.push_back(p);
	addObservedParameter(p);
	p->setMapper(new jura::rsParameterMapperRational(p->getMinValue(), p->getMaxValue(), .5));
	p->setValueChangeCallback([this](double v)
	{
		masterVolume = v;
	});
	
	p->setDefaultModParameters
	(
		0,
		+10,
		-4,
		+4,
		ModulationConnection::modModes::ABSOLUTE,
		.5
	);

	p = new ModulatableParameter2("paramSmooth", 0.001, 1, .01, Parameter::EXPONENTIAL);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v)
	{
		for (auto & param : parametersForSmoothing)
			param->setSmoothingTime(v*1000);

		for (auto & obj : smoothingManager->usedSmoothers)
			obj->setTimeConstantAndSampleRate(v*1000, hostSampleRate);				
	});
	p->setDefaultModParameters
	(
		0,
		2,
		-2,
		+2,
		ModulationConnection::modModes::ABSOLUTE,
		.5
	);

	p = new ModulatableParameter2("oversample", 1, 8, 2, Parameter::LINEAR, 1);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { setOversampling((int)v); });
}

void RhythmAndPitchGeneratorModule::setStateFromXml(const XmlElement & xmlState, const String & stateName, bool markAsClean)
{
	AudioModuleWithMidiIn::setStateFromXml(xmlState, stateName, markAsClean);

	XmlElement* modXml = xmlState.getChildByName("Modulations");
	if (modXml != nullptr)
		modulationManager.setStateFromXml(*modXml);
	else
		modulationManager.removeAllConnections();
}

XmlElement* RhythmAndPitchGeneratorModule::getStateAsXml(const juce::String & stateName, bool markAsClean)
{
	XmlElement* state = AudioModuleWithMidiIn::getStateAsXml(stateName, markAsClean);

	state->addChildElement(modulationManager.getStateAsXml());

	return state;
}

void RhythmAndPitchGeneratorModule::setSampleRate(double v)
{
	hostSampleRate = v;
	internalSampleRate = oversampling * hostSampleRate;
	for (auto & obj : childModules)
		obj->setSampleRate(internalSampleRate);

	smootyMan.setSampleRate(hostSampleRate);
	smoothingManager->setSmoothingTime(hostSampleRate);
	dcFilter.setSampleRate(hostSampleRate);

	masterTimer.setSampleRate(internalSampleRate);
	resetBarsTimer.setSampleRate(internalSampleRate);
	masterRateRandomGenerator.setSampleRate(internalSampleRate);
}

void RhythmAndPitchGeneratorModule::reset()
{
	pitchModule->reset();
	rhythmModule->reset();
	masterTimer.reset();
}

void RhythmAndPitchGeneratorModule::handleMidiMessage(MidiMessage message)
{
	ScopedLock scopedLock(*lock);

	MIDIMASTER.handleMidiMessage(message);
}

void RhythmAndPitchGeneratorModule::setOversampling(int v)
{
	oversampling = v;
	setSampleRate(hostSampleRate);
	antiAliasFilterL.setSubDivision(oversampling);
	antiAliasFilterR.setSubDivision(oversampling);
}

void RhythmAndPitchGeneratorModule::triggerNoteOn()
{
	if (MIDIMASTER.getCurrentChannel() == 16)
	{
		int note = MIDIMASTER.getNoteOn();

		if (note < fileList.size())
			MessageManager::callAsync([this, note]() { loadFile(fileList[note]); });

		return;
	}

	masterTimer.reset();
	resetBarsTimer.reset(0.0);
	pitchModule->pitchQuantizer.reset();
	rhythmModule->rhythmGen.reset();

	pitchModule->pitchQuantizer.triggerNoteOn(MIDIMASTER.getNoteOn());

	masterTimer.startTimer();
	resetBarsTimer.startTimer();
}

void RhythmAndPitchGeneratorModule::triggerNoteOff()
{
	pitchModule->pitchQuantizer.triggerNoteOff(MIDIMASTER.getNoteOff());
}

void RhythmAndPitchGeneratorModule::triggerRelease()
{
	masterTimer.stopTimer();
	resetBarsTimer.stopTimer();
	envelopeModule->envelope.triggerOff();
}

void RhythmAndPitchGeneratorModule::triggerBPMChange()
{
	recalculateMasterTimerFrequency();
	recalculateResetBarTimerFrequency();
	oscillatorModule->oscillator.informBPM(MIDIMASTER.getBPM());
}

void RhythmAndPitchGeneratorModule::recalculateMasterTimerFrequency()
{
	masterTimerFrequency = barsToFrequency(MIDIMASTER.getBPM(), bars) * masterRateWithNoise;
	masterTimer.setFrequency(masterTimerFrequency);

	for (auto & c : rhythmModule->rhythmGen.AllClocksArray)
		c->informMasterTimerFrequency(masterTimerFrequency);

	pitchModule->pitchQuantizer.informMasterTimerFrequency(masterTimerFrequency);
}

void RhythmAndPitchGeneratorModule::recalculateResetBarTimerFrequency()
{
	resetBarsTimer.setFrequency(barsToFrequency(MIDIMASTER.getBPM(), globalResetBars) * masterRateWithNoise);
}

void RhythmAndPitchGeneratorModule::processSampleFrame(double * left, double * right)
{
	double masterRateNoise = 0;
	if (masterRate > 128)
	{
		masterRateRandomGenerator.setFrequency((masterRate - 128));
		masterRateRandomGenerator.increment();
		masterRateNoise = jmap<double>(masterRate, 128, 500, 0, .5) * masterRateRandomGenerator.getSmoothedNoiseValue();
		masterRateNeedsUpdating = true;
	}

	if (masterRateNeedsUpdating)
	{
		masterRateWithNoise = masterRate + masterRate * masterRateNoise;
		masterRateNeedsUpdating = false;
		oscillatorModule->oscillator.informMasterRate(masterRateWithNoise, doesMasterRateControlPitch);
		recalculateMasterTimerFrequency();
		recalculateResetBarTimerFrequency();
	}

	resetBarsTimer.incrememtIfRunning();
	masterTimer.incrememtIfRunning();
	rhythmModule->getSample();
	pitchModule->getSample();

 	envelopeOut = envelopeModule->getSample();
	double mod = oscillatorModule->ampFilterEnvMod;

	double filterMod = max<double>(mod, 0);
	double ampModAmt = min<double>(mod + 1, 1);

	double bottom = 1 - ampModAmt;
	double top = ampModAmt;
	double amplitudeMod = bottom + top * envelopeOut;

	oscillatorModule->oscillator.setAmplitudeMod(amplitudeMod);
	oscillatorModule->oscillator.setFilterFrequencyOffset(filterMod);
	oscillatorModule->oscillator.setFilterFrequencyMod(envelopeOut);

	*left = *right = oscillatorFinalOutValue = oscillatorModule->getSample();
}

void RhythmAndPitchGeneratorModule::processBlock(double **inOutBuffer, int numChannels, int numSamples)
{
	ScopedLock scopedLock(*lock);

	if (BasicModule::isDemoTimedOut())
	{
		BasicModule::processBlockDemo(inOutBuffer, numChannels, numSamples);
		return;
	}

	const bool doApplyModulations = modulationManager.getNumConnections() != 0;

	const bool doApplySmoothing = smoothingManager->needsSmoothing();

	double x, y;

	for (int n = 0; n < numSamples; n++)
	{
		if (doApplySmoothing)
			smoothingManager->updateSmoothedValues();

		// retrieve input point:
		x = inOutBuffer[0] ? inOutBuffer[0][n] : 0.0;
		y = inOutBuffer[1] ? inOutBuffer[1][n] : 0.0;

		if (MIDIMASTER.areNotesHeld() || !envelopeModule->envelope.isSilent())
		{
			for (int i = 0; i < oversampling; i++)
			{
				if (doApplyModulations)
					modulationManager.applyModulations();

				processSampleFrame(&x, &y);

				if (oversampling > 1)
				{
					x = antiAliasFilterL.getSampleDirect1(x);
					//y = antiAliasFilterR.getSampleDirect1(y);
				}
			}
		}

		x *= masterVolume;
		x = dcFilter.getSample(x);
		x = clip(x, -2.0, +2.0);

		if (inOutBuffer[0]) { inOutBuffer[0][n] = x; }
		if (inOutBuffer[1]) { inOutBuffer[1][n] = x; }
	}
}
