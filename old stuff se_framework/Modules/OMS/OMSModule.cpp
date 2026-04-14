#include "OMSModule.h"

JerobeamRadarModule::JerobeamRadarModule(CriticalSection* lockToUse
	,	MetaParameterManager* metaManagerToUse
	,	ModulationManager* modManagerToUse
	, MidiMaster * midiMasterPtr_)
	: ModulatableAudioModule(lockToUse, metaManagerToUse, modManagerToUse)
{
	midiMasterPtr = midiMasterPtr_;

	ModulatableParameter2 * p;

	addObservedParameter(p = new ModulatableParameter2("Bypass", 0, 1, 0));
	p->setValueChangeCallback([this](double v) { setIsBypassed(v > 0.5); });
	p->setDefaultModParameters(
		0,
		1,
		-1,
		+1,
		jura::ModulationConnection::modModes::ABSOLUTE,
		1
	);

	addObservedParameter(p = new ModulatableParameter2("RadarGain", -1, 1, 1));
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { setGain(v); });
	p->setDefaultModParameters(
		-1.e+100,
		+1.e+100,
		0,
		0,
		jura::ModulationConnection::modModes::ABSOLUTE,
		1
	);
	
	addObservedParameter(p = new ModulatableParameter2("Phase", -1, 1, 0));
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { radarCore.phasor.setPhaseOffset(v); });
	p->setDefaultModParameters(
		-1.e+100,
		+1.e+100,
		-10,
		+10,
		jura::ModulationConnection::modModes::ABSOLUTE,
		1
	);

	addObservedParameter(p = new ModulatableParameter2("Density", 0, 400, 1));
	parametersForSmoothing.push_back(p);
	p->setMapper(new rsParameterMapperSinhUni(p->getMinValue(), p->getMaxValue(), 7));
	p->setValueChangeCallback([this](double v) { radarCore.setDensity(v); });
	p->setDefaultModParameters(
		-1.e+100,
		+1.e+100,
		-8,
		+8,
		jura::ModulationConnection::modModes::EXPONENTIAL,
		1
	);

	addObservedParameter(p = new ModulatableParameter2("Sharp", -1, 1, 0));
	parametersForSmoothing.push_back(p);
	p->setMapper(new jura::rsParameterMapperTanh(p->getMinValue(), p->getMaxValue(), 2));
	p->setValueChangeCallback([this](double v) { radarCore.setSharp(v); });
	p->setDefaultModParameters(
		-1,
		+1,
		-1,
		+1,
		jura::ModulationConnection::modModes::ABSOLUTE,
		.5
	);

	addObservedParameter(p = new ModulatableParameter2("Sharp_Curve", 0, 35, 1));
	parametersForSmoothing.push_back(p);
	p->setMapper(new rsParameterMapperSinhUni(p->getMinValue(), p->getMaxValue(), 7));
	p->setValueChangeCallback([this](double v) { radarCore.setFade(v); });
	p->setDefaultModParameters(
		-1.e+100,
		+1.e+100,
		-p->getMaxValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		1
	);

	addObservedParameter(p = new ModulatableParameter2("SubPhase", -1, +1, 0));
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { radarCore.setRotation(v); });
	p->setDefaultModParameters(
		-1.e+100,
		+1.e+100,
		-p->getMaxValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		.1
	);

	addObservedParameter(p = new ModulatableParameter2("SubPhaseRotation", -20, +20, 0));
	parametersForSmoothing.push_back(p);
	p->setMapper(new rsParameterMapperSinh(p->getMinValue(), p->getMaxValue(), 4));
	p->setValueChangeCallback([this](double v) { radarCore.rotator.setFrequency(v * getMidiMasterPtr()->getMasterRate()); });
	p->setDefaultModParameters(
		-1.e+100,
		+1.e+100,
		-100,
		+100,
		jura::ModulationConnection::modModes::ABSOLUTE,
		20
	);

	addObservedParameter(p = new ModulatableParameter2("Direction", 0, 1, 1));
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { radarCore.setDirection(v); });
	p->setDefaultModParameters(
		-1.e+100,
		+1.e+100,
		-p->getMaxValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		.1
	);

	addObservedParameter(p = new ModulatableParameter2("Shade", 0, 80, 1));
	parametersForSmoothing.push_back(p);
	p->setMapper(new rsParameterMapperSinhUni(p->getMinValue(), p->getMaxValue(), 8));
	p->setValueChangeCallback([this](double v) { radarCore.setShade(v); });
	p->setDefaultModParameters(
		-1.e+100,
		+1.e+100,
		-p->getMaxValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		1
	);

	addObservedParameter(p = new ModulatableParameter2("Lap", -1, 1, 0));
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { radarCore.setLap(v); });
	p->setDefaultModParameters(
		-1.e+100,
		+1.e+100,
		-p->getMaxValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		.1
	);

	addObservedParameter(p = new ModulatableParameter2("RingCut", 0, 1, 0, Parameter::BOOLEAN));
	p->setValueChangeCallback([this](double v) { radarCore.setRingCut(v >= 0.5); });
	p->setDefaultModParameters(
		0,
		1,
		-1,
		+1,
		jura::ModulationConnection::modModes::ABSOLUTE,
		1
	);

	addObservedParameter(p = new ModulatableParameter2("Curve_Start", 0, 1, 0, Parameter::BOOLEAN));
	p->setValueChangeCallback([this](double v) { radarCore.setPow1Up(v >= 0.5); });
	p->setDefaultModParameters(
		0,
		1,
		-1,
		+1,
		jura::ModulationConnection::modModes::ABSOLUTE,
		1
	);

	addObservedParameter(p = new ModulatableParameter2("Curve_End", 0, 1, 0, Parameter::BOOLEAN));
	p->setValueChangeCallback([this](double v) { radarCore.setPow1Down(v >= 0.5); });
	p->setDefaultModParameters(
		0,
		1,
		-1,
		+1,
		jura::ModulationConnection::modModes::ABSOLUTE,
		1
	);

	addObservedParameter(p = new ModulatableParameter2("Bend", 0, 1, 0, Parameter::BOOLEAN));
	p->setValueChangeCallback([this](double v) { radarCore.setPow2Bend(v >= 0.5); });
	p->setDefaultModParameters(
		-1.e+100,
		+1.e+100,
		0,
		0,
		jura::ModulationConnection::modModes::ABSOLUTE,
		0
	);
	
	addObservedParameter(p = new ModulatableParameter2("PhaseInv", 0, 1, 0, Parameter::BOOLEAN));
	p->setValueChangeCallback([this](double v) { radarCore.setPhaseInv(v >= 0.5); });
	p->setDefaultModParameters(
		0,
		1,
		-1,
		+1,
		jura::ModulationConnection::modModes::ABSOLUTE,
		1
	);

	addObservedParameter(p = new ModulatableParameter2("TunnelInv", 0, 1, 0, Parameter::BOOLEAN));
	p->setValueChangeCallback([this](double v) { radarCore.setTunnelInv(v >= 0.5); });
	p->setDefaultModParameters(
		-1.e+100,
		+1.e+100,
		0,
		0,
		jura::ModulationConnection::modModes::ABSOLUTE,
		0
	);

	addObservedParameter(p = new ModulatableParameter2("SpiralReturn", 0, 1, 0, Parameter::BOOLEAN));
	p->setValueChangeCallback([this](double v) { radarCore.setSpiralReturn(v >= 0.5); });
	p->setDefaultModParameters(
		0,
		1,
		-1,
		+1,
		jura::ModulationConnection::modModes::ABSOLUTE,
		1
	);

	addObservedParameter(p = new ModulatableParameter2("Length", 0.0001, 1, 1));
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { radarCore.setLength(v); });
	p->setDefaultModParameters(
		-1.e+100,
		+1.e+100,
		-p->getMaxValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		.1
	);

	addObservedParameter(p = new ModulatableParameter2("Ratio", 0, 1, 1));
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { radarCore.setRatio(v); });
	p->setDefaultModParameters(
		-1.e+100,
		+1.e+100,
		0,
		0,
		jura::ModulationConnection::modModes::ABSOLUTE,
		0
	);

	addObservedParameter(p = new ModulatableParameter2("FrontRing", 0, 1, 0));
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { radarCore.setFrontRing(v); });
	p->setDefaultModParameters(
		-1.e+100,
		+1.e+100,
		-p->getMaxValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		.1
	);

	addObservedParameter(p = new ModulatableParameter2("Morph", 0, 1, 1));
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { radarCore.setZoom(v); });
	p->setDefaultModParameters(
		-1.e+100,
		+1.e+100,
		-p->getMaxValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		.1
	);

	addObservedParameter(p = new ModulatableParameter2("ZDepth", 0, 1, 0));
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { radarCore.setZDepth(v); });
	p->setDefaultModParameters(
		-1.e+100,
		+1.e+100,
		-p->getMaxValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		.1
	);

	addObservedParameter(p = new ModulatableParameter2("Inner", 0, 1, 0));
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { radarCore.setInner(v); });
	p->setDefaultModParameters(
		-1.e+100,
		+1.e+100,
		-p->getMaxValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		.1
	);

	addObservedParameter(p = new ModulatableParameter2("Protrude_X", -1, 1, 0));
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { radarCore.setX(v); });
	p->setDefaultModParameters(
		-1.e+100,
		+1.e+100,
		-p->getMaxValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		.1
	);

	addObservedParameter(p = new ModulatableParameter2("Protrude_Y", -1, 1, 0));
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { radarCore.setY(v); });
	p->setDefaultModParameters(
		-1.e+100,
		+1.e+100,
		-p->getMaxValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		.1
	);
}

OMSModule::OMSModule(CriticalSection* lockToUse
	, MetaParameterManager* metaManagerToUse
	, ModulationManager * modulationManagerToUse)
	: BasicModule(lockToUse, metaManagerToUse, modulationManagerToUse)
{
	ScopedLock scopedLock(*lock);

	setModuleTypeName("OMSModule");

	AudioModule::patchFormatIndex = 3;

	Parameter::setStoreDefaultValuesToXml(true);

	vector<String> EnvShapeMenuItems = { "EXP", "EXP2", "LOG", "S-CRV" };

	radarModule = new JerobeamRadarModule(lock, metaParamManager, &modulationManager, &MIDIMASTER);
	parametersForSmoothing.insert(std::end(parametersForSmoothing), std::begin(radarModule->parametersForSmoothing), std::end(radarModule->parametersForSmoothing));
	MIDIMASTER.addMidiSlave(radarModule, MidiMaster::AddFor::MasterRateChange);
	addChildAudioModule(radarModule);
	radarModule->setModuleName("JerobeamRadar");
	//radarModule.setModulationSourceName("JerobeamRadar");
	//modulationManager.registerModulationSource(&radarModule);
	parSubPhaseRotation = radarModule->getParameterByName("SubPhaseRotation");

	omsCore = new OMSCoreOversampled(radarModule);
	
	modFeedback.setValuePtr(&omsCore->feedbackValue);
	ampEnvOut.setValuePtr(&omsCore->ampEnvVal);

	setModulationManager(&modulationManager);
	modulationManager.setMetaParameterManager(metaParamManager);
	modulationManager.registerModulationSource(&modFeedback);
	modulationManager.registerModulationSource(&ampEnvOut);

	MIDIMASTER.addMidiSlave(this, MidiMaster::AddFor::MonoNoteCallbacks);

	MIDIMASTER.addMidiSlave(omsCore, MidiMaster::AddFor::MonoNoteCallbacks);
	MIDIMASTER.addMidiSlave(omsCore, MidiMaster::AddFor::MasterRateChange);

	addChildAudioModule(equalizerModule = new jura::EqualizerAudioModule(lock, &equalizer));
	equalizerModule->setModuleName("Equalizer");
	equalizerModule->getParameterByName("Bypass")->setValue(1, true, true);

	addChildAudioModule(limiterModule = new ElanLimiterModule(lock, metaParamManager, &modulationManager));
	limiterModule->setModuleName("Limiter");

	addChildAudioModule(omsCore->bandpassModule = new BandpassModule(lock, metaParamManager, &modulationManager));
	omsCore->bandpassModule->setModuleName("Filter");

	addChildAudioModule(delayModule = new DelayModule(lock, metaParamManager, &modulationManager));
	delayModule->setModuleName("Delay");

	addChildAudioModule(oscilloscopeModule = new OscilloscopeModule(lock, metaParamManager, &modulationManager));
	oscilloscopeModule->setModuleName("Oscilloscope");

	addChildAudioModule(psmod = new PrettyScopeModule(lock, metaParamManager));
	psmod->setModuleName("PSMod");

	ModulatableParameter2 * p;

	addObservedParameter(p = new ModulatableParameter2("Reset_Mode", 0,
		(double)OMSCore::ResetModeMenuItems.size() - 1,
		OMSCore::ResetMode::when_not_legato,
		Parameter::STRING)
	);
	for (const auto & s : OMSCore::ResetModeMenuItems)
		p->addStringValue(s);
	p->setValueChangeCallback([this](double v) { omsCore->setResetMode((int)v); });
	p->setDefaultModParameters(
		p->getMinValue(),
		p->getMaxValue(),
		-p->getMaxValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		1
	);

	addObservedParameter(p = new ModulatableParameter2("Reset_on_tempo_change", 0, 1, 0, Parameter::BOOLEAN));
	p->setValueChangeCallback([this](double v) { omsCore->resetWhenBeatTempoOrMultChanes = v >= 0.5; });
	p->setDefaultModParameters(
		0,
		1,
		-1,
		+1,
		jura::ModulationConnection::modModes::ABSOLUTE,
		1
	);

	addObservedParameter(p = new ModulatableParameter2("Channel_Mode", 0,
		(double)MonoSynth::ChannelModeMenuItems.size() - 1,
		0,
		Parameter::STRING)
	);
	for (const auto & s : MonoSynth::ChannelModeMenuItems)
		p->addStringValue(s);
	p->setValueChangeCallback([this](double v) { omsCore->setChannelMode((int)v); });
	p->setDefaultModParameters(
		p->getMinValue(),
		p->getMaxValue(),
		-p->getMaxValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		1
	);

	addObservedParameter(p = new ModulatableParameter2("Clip_Mode", 0,
		(double)OMSCore::ClippingModeMenuItems.size() - 1,
		1,
		Parameter::STRING)
	);
	for (const auto & s : OMSCore::ClippingModeMenuItems)
		p->addStringValue(s);
	p->setValueChangeCallback([this](double v) { omsCore->setClipMode((int)v); });
	p->setDefaultModParameters(
		p->getMinValue(),
		p->getMaxValue(),
		-p->getMaxValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		1
	);

	addObservedParameter(p = new ModulatableParameter2("Clip_Level", 1.e-3, 1.5, 1, jura::Parameter::scalings::EXPONENTIAL));
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { omsCore->setClipLevel(v); });
	p->setDefaultModParameters(
		-1.e-6,
		+2,
		-2,
		+2,
		jura::ModulationConnection::modModes::ABSOLUTE,
		.5
	);
	
	addObservedParameter(p = new ModulatableParameter2("Gain", -60, 40, 0));
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { omsCore->setOutputAmplitude(RAPT::rsDbToAmp(v)); });
	p->setDefaultModParameters(
		-1.e+100,
		+1.e+100,
		-p->getMaxValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		1
	);

	addObservedParameter(p = new ModulatableParameter2("Stereo_Rotate", -1, 1, 0));
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { omsCore->setRotation(v); });
	p->setDefaultModParameters(
		-1.e+100,
		+1.e+100,
		-p->getMaxValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		.1
	);

	addObservedParameter(p = new ModulatableParameter2("DC_Offset", -1, +1, 0));
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { omsCore->setDCOffset(v); });
	p->setDefaultModParameters(
		-1.e+100,
		+1.e+100,
		-p->getMaxValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		1
	);

	addObservedParameter(p = new ModulatableParameter2("X_Offset", -1, +1, 0));
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { omsCore->setXOffset(v); });
	p->setDefaultModParameters(
		-1.e+100,
		+1.e+100,
		-p->getMaxValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		1
	);

	addObservedParameter(p = new ModulatableParameter2("Y_Offset", -1, +1, 0));
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { omsCore->setYOffset(v); });
	p->setDefaultModParameters(
		-1.e+100,
		+1.e+100,
		-p->getMaxValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		1
	);

	addObservedParameter(parBP1Reset = p = new ModulatableParameter2("BP1ResetMode", 0, 2, 2, Parameter::STRING));
	for (const auto & s : { "Always", "On-note", "Legato" })
		p->addStringValue(s);
	p->setValueChangeCallback([this](double v) { MIDIMASTER.setNoteChangeMode((int)v); });
	p->setDefaultModParameters(
		p->getMinValue(),
		p->getMaxValue(),
		-p->getMaxValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		1
	);

	addObservedParameter(parBP2Reset = p = new ModulatableParameter2("BP2ResetMode", 0, 2, 2, Parameter::STRING));
	for (const auto & s : { "Always", "On-note", "Legato" })
		p->addStringValue(s);
	p->setValueChangeCallback([this](double v) { MIDIMASTER.setNoteChangeMode((int)v); });
	p->setDefaultModParameters(
		p->getMinValue(),
		p->getMaxValue(),
		-p->getMaxValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		1
	);

	addObservedParameter(p = new ModulatableParameter2("Note_Change_Mode", 0,
		(double)MIDIMASTER.NoteChangeModeMenuItems.size() - 1,
		MIDIMASTER.NoteChangeMode::Legato,
		Parameter::STRING)
	);
	for (const auto & s : MIDIMASTER.NoteChangeModeMenuItems)
		p->addStringValue(s);
	p->setValueChangeCallback([this](double v) { MIDIMASTER.setNoteChangeMode((int)v); });
	p->setDefaultModParameters(
		p->getMinValue(),
		p->getMaxValue(),
		-p->getMaxValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		1
	);

	addObservedParameter(p = new ModulatableParameter2("Frequency_Mode", 0,
		(double)OMSCore::KeytrackingMenuItems.size() - 1,
		0,
		Parameter::STRING)
	);
	for (const auto & s : OMSCore::KeytrackingMenuItems)
		p->addStringValue(s);
	p->setValueChangeCallback([this](double v) { omsCore->setKeytrackingMode((OMSCore::KeytrackingMode)(int)(v)); });
	p->setDefaultModParameters(
		p->getMinValue(),
		p->getMaxValue(),
		-p->getMaxValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		1
	);

	addObservedParameter(p = new ModulatableParameter2("Glide_Mode", 0,
		(double)MIDIMASTER.GlideModeMenuItems.size() - 1,
		0,
		Parameter::STRING)
	);
	for (const auto & s : MIDIMASTER.GlideModeMenuItems)
		p->addStringValue(s);
	p->setValueChangeCallback([this](double v) { MIDIMASTER.setGlideMode((int)v); });
	p->setDefaultModParameters(
		p->getMinValue(),
		p->getMaxValue(),
		-p->getMaxValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		1
	);

	ParameterProfile::ClipLevel parClip{ &paramManager };

	addObservedParameter(p = new ModulatableParameter2("Frequency", -1000, +1000, 0));
	parametersForSmoothing.push_back(p);
	p->setMapper(new rsParameterMapperSinh(p->getMinValue(), p->getMaxValue(), 4));
	p->setValueChangeCallback([this](double v) { omsCore->setFrequencyOffset(v); });
	p->setDefaultModParameters(
		-1.e+100,
		+1.e+100,
		-8000,
		+8000,
		jura::ModulationConnection::modModes::ABSOLUTE,
		100
	);

	addObservedParameter(p = new ModulatableParameter2("Tempo", 0, 25, 0, Parameter::LINEAR, 1));
	p->setValueChangeCallback([this](double v) { omsCore->setTempoSelection((int)v); });
	p->setDefaultModParameters(
		0,
		25,
		-p->getMaxValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		1
	);

	addObservedParameter(p = new ModulatableParameter2("Tempo_Multiply", -12, +12, 0, Parameter::LINEAR, 1));
	p->setValueChangeCallback([this](double v) { omsCore->setTempoMultiplier(v); });
	p->setDefaultModParameters(
		-20,
		+20,
		-p->getMaxValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		1
	);

	addObservedParameter(p = new ModulatableParameter2("Tune", -72, 72, 0));
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { omsCore->setSemitoneOffset(v); });
	p->setDefaultModParameters(
		-1.e+100,
		+1.e+100,
		-72,
		+72,
		jura::ModulationConnection::modModes::ABSOLUTE,
		0
	);

	addObservedParameter(p = new ModulatableParameter2("Octave", -12, 12, 0, Parameter::LINEAR, 1));
	p->setValueChangeCallback([this](double v) { omsCore->setOctaveOffset(v); });
	p->setDefaultModParameters(
		-1.e+100,
		+1.e+100,
		-8,
		+8,
		jura::ModulationConnection::modModes::ABSOLUTE,
		2
	);

	addObservedParameter(p = new ModulatableParameter2("Frequency_Multiply", -8, +8, 1));
	parametersForSmoothing.push_back(p);
	p->setMapper(new jura::rsParameterMapperSinh(p->getMinValue(), p->getMaxValue(), 4));
	p->setValueChangeCallback([this](double v)
	{
		MIDIMASTER.setMasterRate(v);
	});
	p->setDefaultModParameters(
		-1.e+100,
		+1.e+100,
		-10,
		+10,
		jura::ModulationConnection::modModes::ABSOLUTE,
		1
	);

	addObservedParameter(p = new ModulatableParameter2("Harmonic_Multiply", 1, 32, 1, Parameter::LINEAR, 1));
	p->setValueChangeCallback([this](double v) { omsCore->setHarmonicMultiplier(v); });
	p->setDefaultModParameters(
		-1.e+100,
		+1.e+100,
		0,
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		4
	);

	addObservedParameter(p = new ModulatableParameter2("Glide_Amount", 1.e-5, 1, 1.e-3, Parameter::EXPONENTIAL));
	p->setValueChangeCallback([this](double v) { MIDIMASTER.setGlideSeconds(v); });
	p->setDefaultModParameters(
		p->getMinValue(),
		p->getMaxValue(),
		-p->getMaxValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		.1
	);

	addObservedParameter(p = new ModulatableParameter2("HarmOct_Glide_Amount", 1.e-5, 1, .01, Parameter::EXPONENTIAL));
	p->setValueChangeCallback([this](double v) { omsCore->setHarmAndOctGlideAmt(v); });
	p->setDefaultModParameters(
		p->getMinValue(),
		p->getMaxValue(),
		-p->getMaxValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		.1
	);

	addObservedParameter(p = new ModulatableParameter2("Oversampling", 1, 8, 1, Parameter::LINEAR, 1));
	p->setValueChangeCallback([this](double v) { omsCore->setOversampling((int)v); });
	p->setDefaultModParameters(
		p->getMinValue(),
		p->getMaxValue(),
		-p->getMaxValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		0
	);

	addObservedParameter(p = new ModulatableParameter2("LP_Feedback", .01, 24000, 10000, Parameter::EXPONENTIAL));
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { omsCore->setFeedbackLPCutoff(v); });
	p->setDefaultModParameters(
		0,
		+1.e+100,
		-20,
		+20,
		jura::ModulationConnection::modModes::EXPONENTIAL,
		14
	);

	addObservedParameter(p = new ModulatableParameter2("HP_Feedback", .01, 24000, 30, Parameter::EXPONENTIAL));
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { omsCore->setFeedbackHPCutoff(v); });
	p->setDefaultModParameters(
		0,
		+1.e+100,
		-20,
		+20,
		jura::ModulationConnection::modModes::EXPONENTIAL,
		14
	);

	addObservedParameter(p = new ModulatableParameter2("Parameter_Smoothing", 0.001, 1, .01, Parameter::EXPONENTIAL));
	p->setValueChangeCallback([this](double v)
	{
		limiterModule->setGlobalSmoothingSpeed(v);
		delayModule->setGlobalSmoothingSpeed(v);

		for (auto & param : parametersForSmoothing)
			param->setSmoothingTime(v * 1000);

		if (smoothingManager != nullptr)
			for (auto & obj : smoothingManager->usedSmoothers)
				obj->setTimeConstantAndSampleRate(v * 1000, sampleRate);
	});
	p->setDefaultModParameters(
		p->getMinValue(),
		p->getMaxValue(),
		-p->getMaxValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		.1
	);

	addObservedParameter(p = new ModulatableParameter2("Feedback_Source", 0,
		(double)OMSCore::FeedbackSourceMenuItems.size() - 1,
		0,
		Parameter::STRING)
	);
	for (const auto & s : OMSCore::FeedbackSourceMenuItems)
		p->addStringValue(s);
	p->setValueChangeCallback([this](double v) { omsCore->setFeedbackSource((int)v); });
	p->setDefaultModParameters(
		p->getMinValue(),
		p->getMaxValue(),
		-p->getMaxValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		1
	);

	addObservedParameter(p = new ModulatableParameter2("Feedback_Amount", -5, +5, 0));
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { omsCore->setFeedbackAmp(v); });
	p->setValue(1, true, true);
	p->setDefaultModParameters(
		-1.e+10,
		+1.e+10,
		-8,
		+8,
		jura::ModulationConnection::modModes::ABSOLUTE,
		1
	);

	addObservedParameter(p = new ModulatableParameter2("EnvResetMode", 0,
		(double)OMSCore::EnvResetModeMenuItems.size() - 1,
		2,
		Parameter::STRING)
	);
	for (const auto & s : OMSCore::EnvResetModeMenuItems)
		p->addStringValue(s);
	p->setValueChangeCallback([this](double v) { omsCore->setAmpEnvResetMode((OMSCore::EnvResetMode)(int)v); });
	p->setDefaultModParameters(
		p->getMinValue(),
		p->getMaxValue(),
		-p->getMaxValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		1
	);

	addObservedParameter(p = new ModulatableParameter2("Linear_Time", .001, 20000, 100, Parameter::EXPONENTIAL));
	p->setValueChangeCallback([this](double v) { omsCore->ampEnv.setGlobalTime(v); });
	p->setDefaultModParameters(
		p->getMinValue(),
		p->getMaxValue(),
		-p->getMaxValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		1
	);

	addObservedParameter(p = new ModulatableParameter2("Sustain", 0, 1, 1));
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { omsCore->ampEnv.setSustainAmplitude(v); });
	p->setDefaultModParameters(
		-1.e+100,
		+1.e+100,
		-p->getMaxValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		.5
	);

	addObservedParameter(p = new ModulatableParameter2("Attack", 1.e-6, 3, 0.9));
	p->setValueChangeCallback([this](double v) { omsCore->ampEnv.setAttackFeedback(v); });
	p->setDefaultModParameters(
		-1.e+100,
		+1.e+100,
		-p->getMaxValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		.1
	);

	addObservedParameter(p = new ModulatableParameter2("Decay", 1.e-6, 3, 1.4));
	p->setValueChangeCallback([this](double v) { omsCore->ampEnv.setDecayFeedback(v); });
	p->setDefaultModParameters(
		-1.e+100,
		+1.e+100,
		-p->getMaxValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		.1
	);

	addObservedParameter(p = new ModulatableParameter2("Release", 1.e-6, 3, 1));
	p->setValueChangeCallback([this](double v) { omsCore->ampEnv.setReleaseFeedback(v); });
	p->setDefaultModParameters(
		-1.e+100,
		+1.e+100,
		-p->getMaxValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		.1
	);

	addObservedParameter(p = new ModulatableParameter2("Speed", 100, 20000, 150, Parameter::EXPONENTIAL));
	p->setValueChangeCallback([this](double v) { omsCore->ampEnv.setGlobalFeedback(v); });
	p->setDefaultModParameters(
		-1.e+100,
		+1.e+100,
		p->getMinValue(),
		p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		0
	);

	addObservedParameter(p = new ModulatableParameter2("Atk_Shape", 0,
		(double)EnvShapeMenuItems.size() - 1,
		2,
		Parameter::STRING)
	);
	for (const auto & s : EnvShapeMenuItems)
		p->addStringValue(s);
	p->setValueChangeCallback([this](double v) { omsCore->ampEnv.setAttackShape((FeedbackADSR::Shape)(int)v); });
	p->setDefaultModParameters(
		p->getMinValue(),
		p->getMaxValue(),
		-p->getMaxValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		1
	);

	addObservedParameter(p = new ModulatableParameter2("Dec_Shape", 0,
		(double)EnvShapeMenuItems.size() - 1,
		0,
		Parameter::STRING)
	);
	for (const auto & s : EnvShapeMenuItems)
		p->addStringValue(s);
	p->setValueChangeCallback([this](double v) { omsCore->ampEnv.setDecayShape((FeedbackADSR::Shape)(int)v); });
	p->setDefaultModParameters(
		p->getMinValue(),
		p->getMaxValue(),
		-p->getMaxValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		1
	);

	addObservedParameter(p = new ModulatableParameter2("Rel_Shape", 0,
		(double)EnvShapeMenuItems.size() - 1,
		0,
		Parameter::STRING)
	);
	for (const auto & s : EnvShapeMenuItems)
		p->addStringValue(s);
	p->setValueChangeCallback([this](double v) { omsCore->ampEnv.setReleaseShape((FeedbackADSR::Shape)(int)v); });
	p->setDefaultModParameters(
		p->getMinValue(),
		p->getMaxValue(),
		-p->getMaxValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		1
	);

	addObservedParameter(p = new ModulatableParameter2("Vel_Scale", 0, 1, 0));
	p->setValueChangeCallback([this](double v) { omsCore->ampEnv.setVelocityInfluence(v); });
	p->setDefaultModParameters(
		-1.e+100,
		+1.e+100,
		-p->getMaxValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		.1
	);

	addObservedParameter(p = new ModulatableParameter2("MidiPitchBend", -1, +1, 0));
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v)
	{
		//const double centerDetent = 0.1;
		//if (v < -centerDetent)
		//	v = jmap<double>(v, -1.0, -centerDetent, -1.0, 0.0);
		//else if (v > +centerDetent)
		//	v = jmap<double>(v, +centerDetent, +1.0, 0.0, 1.0);
		//else
		//	v = 0;

		MIDIMASTER.setPitchBend(int(v * 8192), 1);
	});
	p->setDefaultModParameters(
		-1,
		+1,
		p->getMinValue(),
		p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		.1
	);

	addObservedParameter(p = new ModulatableParameter2("MidiModwheel", 0, 1, 0));
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v)
	{
		MIDIMASTER.setMidiController(1, int(v * 127), 1);
	});
	p->setDefaultModParameters(
		0,
		1,
		p->getMinValue(),
		p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		.1
	);

	delayModule->parBypass.setValue(1);

	//loadFile(File(programFolder.initPresetPath));
}

void OMSModule::setSampleRate(double v)
{
	sampleRate = v;

	equalizerModule->setSampleRate(v);
	limiterModule->setSampleRate(v);
	delayModule->setSampleRate(v);
	oscilloscopeModule->setSampleRate(v);
	psmod->setSampleRate(v);
}

void OMSModule::triggerAllNotesOff()
{
}

void OMSModule::triggerAttack()
{
	if (MIDIMASTER.getCurrentChannel() == 16)
	{
		int note = MIDIMASTER.getNoteOn();

		if (note < fileList.size())
			MessageManager::callAsync([this, note]() { loadFile(fileList[note]); });

		return;
	}
}

void OMSModule::triggerNoteOnLegato()
{
}

void OMSModule::triggerNoteOffLegato()
{
}

void OMSModule::triggerRelease()
{
}

void OMSModule::setBeatsPerMinute(double v)
{
	MIDIMASTER.setBPM(v);

	delayModule->delay.setTempoInBPM(v);
}

void OMSModule::setPlayheadInfo(AudioPlayHead::CurrentPositionInfo /*info*/)
{
	//omsCore->setPlayheadInfo(info);
}

void OMSModule::processBlock(double **inOutBuffer, int numChannels, int numSamples)
{
	//double barPhase = getPhaseBasedOnBarsPerCycle(1);

	const bool doApplyModulations = modulationManager.getNumConnections() != 0;

	const bool doApplySmoothing = smoothingManager->needsSmoothing();

	for (int n = 0; n < numSamples; n++)
	{
		double left = 0, right = 0;

		bool isSilent = omsCore->isSilent();

		if (doApplySmoothing)
			smoothingManager->updateSmoothedValues();

		if (!isSilent && doApplyModulations)
				modulationManager.applyModulations();

		MIDIMASTER.incrementPitchGlide();

		if (!isSilent)
				omsCore->processSampleFrame(&left, &right);

		//LinL.setTargetValue(left);
		//LinR.setTargetValue(right);

		equalizerModule->processStereoFrame(&left, &right);
		limiterModule->processSampleFrame(&left, &right);
		delayModule->processSampleFrame(&left, &right);
		//oscilloscopeModule->processStereoFrame(&left, &right);
		psmod->processSampleFrame(&left, &right);

		if (inOutBuffer[0]) inOutBuffer[0][n] = left/*LinL.getSample()*/;
		if (inOutBuffer[1]) inOutBuffer[1][n] = right/*LinR.getSample()*/;
	}

	psmod->audioBuffer.processBlock(inOutBuffer, numChannels, numSamples);
}
