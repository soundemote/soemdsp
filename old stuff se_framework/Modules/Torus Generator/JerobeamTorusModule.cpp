#include "JerobeamTorusModule.h"

JerobeamTorusModule::JerobeamTorusModule(CriticalSection *lockToUse, 
	jura::MetaParameterManager* metaManagerToUse)
	: BasicModule(lockToUse, metaManagerToUse)
{
  ScopedLock scopedLock(*lock);

	AudioModule::patchFormatIndex = 2;

	//loadFile(File(initPresetPath));

	setModuleTypeName("TorusGenerator");

	MIDIMASTER.addMidiSlave(this, MidiMaster::AddFor::MonoNoteCallbacks);

	MIDIMASTER.addMidiSlave(&jbTorusCore, MidiMaster::AddFor::NoteChange);
	MIDIMASTER.addMidiSlave(&jbTorusCore, MidiMaster::AddFor::MonoNoteCallbacks);
	MIDIMASTER.addMidiSlave(&jbTorusCore, MidiMaster::AddFor::MasterRateChange);
	MIDIMASTER.addMidiSlave(&jbTorusCore, MidiMaster::AddFor::HostBPMChange);

	modulationManager.registerModulationSource(&modFeedback);
	modulationManager.registerModulationSource(&ampEnvOut);

	jbTorusCore.bandpassModule = new BandpassModule(lock, metaParamManager, &modulationManager);
	jbTorusCore.bandpassModule->setModuleName("Filter");
	addChildAudioModule(jbTorusCore.bandpassModule);

	ModulatableParameter2* p = nullptr;

	addObservedParameter(p = new ModulatableParameter2("TorusGain", -1, 1, 1));
	//parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { jbTorusCore.setInternalGain(v); });
	p->setDefaultModParameters(-1.e+100, +1.e+100, +8, +8, jura::ModulationConnection::modModes::EXPONENTIAL, 1);

	addObservedParameter(p = new ModulatableParameter2("Phase", -1, 1, 0));
	//parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { jbTorusCore.jbTorus.phasor.setPhaseOffset(v); });
	p->setDefaultModParameters(-1.e+100, +1.e+100, -10, +10, jura::ModulationConnection::modModes::EXPONENTIAL, 1);

	addObservedParameter(p = new ModulatableParameter2("Density", 1, 1000, 3));
	//parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { jbTorusCore.jbTorus.setDensity(v); });
	p->setDefaultModParameters(-1.e+100, +1.e+100, +8, +8, jura::ModulationConnection::modModes::EXPONENTIAL, 1);

	addObservedParameter(p = new ModulatableParameter2("SDensity", -2, +2, 1));
	//parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { jbTorusCore.jbTorus.setSDensity(v); });
	p->setDefaultModParameters(-1.e+100, +1.e+100, +8, +8, jura::ModulationConnection::modModes::EXPONENTIAL, 1);

	addObservedParameter(p = new ModulatableParameter2("SubDensityQuantize", 0, 1, 0));
	//parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { jbTorusCore.jbTorus.setQuantizeSubDensity(v >= 0.5); });
	p->setDefaultModParameters(-1.e+100, +1.e+100, +8, +8, jura::ModulationConnection::modModes::EXPONENTIAL, 1);

	addObservedParameter(p = new ModulatableParameter2("DensityQuantize", 0, 1, 0));
	//parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { jbTorusCore.jbTorus.setQuantizeDensity(v >= 0.5); });
	p->setDefaultModParameters(-1.e+100, +1.e+100, +8, +8, jura::ModulationConnection::modModes::EXPONENTIAL, 1);

	addObservedParameter(p = new ModulatableParameter2("Balance", -10, 10, 0));
	//parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { jbTorusCore.jbTorus.setBalance(v); });
	p->setDefaultModParameters(-1.e+100, +1.e+100, -p->getMaxValue(), +p->getMaxValue(), jura::ModulationConnection::modModes::ABSOLUTE, 1);

	addObservedParameter(p = new ModulatableParameter2("SubPhase", -1, 1, 0));
	//parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { jbTorusCore.jbTorus.phasorWander.setPhaseOffset(v); });
	p->setDefaultModParameters(-1.e+100, +1.e+100, -10, +10, jura::ModulationConnection::modModes::EXPONENTIAL, 1);

	addObservedParameter(p = new ModulatableParameter2("Wander", -1, 1, 0));
	//parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { jbTorusCore.jbTorus.phasorWander.setPhaseOffset(v); });
	p->setDefaultModParameters(-1.e+100, +1.e+100, -p->getMaxValue(), +p->getMaxValue(), jura::ModulationConnection::modModes::ABSOLUTE, .1);

	addObservedParameter(p = new ModulatableParameter2("WanderSpeed", -20, 20, 0));
	//parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { jbTorusCore.jbTorus.setWander(v * MIDIMASTER.getMasterRate()); });
	p->setDefaultModParameters(-1.e+100, +1.e+100, -100, +100, jura::ModulationConnection::modModes::EXPONENTIAL, 20);
	p->setMapper(new jura::rsParameterMapperSinh(p->getMinValue(), p->getMaxValue(), 4));

	addObservedParameter(p = new ModulatableParameter2("Size", 0.1, 1, 0.5, jura::Parameter::scalings::EXPONENTIAL));
	//parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { jbTorusCore.jbTorus.setSize(v); });
	p->setDefaultModParameters(-1.e+100, +1.e+100, -p->getMaxValue(), p->getMaxValue(), jura::ModulationConnection::modModes::ABSOLUTE, 1);

	addObservedParameter(p = new ModulatableParameter2("Length", 0, 1, 1));
	//parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { jbTorusCore.jbTorus.setLength(v); });
	p->setDefaultModParameters(-1.e+100, +1.e+100, -p->getMaxValue(), +p->getMaxValue(), jura::ModulationConnection::modModes::ABSOLUTE, .1);

	addObservedParameter(p = new ModulatableParameter2("Sharp", -1, +1, 0));
	//parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { jbTorusCore.jbTorus.setSharp(bipolarToUnipolar(v)); });
	p->setDefaultModParameters(-1.e+100, +1.e+100, -p->getMaxValue(), +p->getMaxValue(), jura::ModulationConnection::modModes::ABSOLUTE, .1);

	addObservedParameter(p = new ModulatableParameter2("DarkAngle", -1, +1, 0, jura::Parameter::scalings::LINEAR_BIPOLAR));
	//parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { jbTorusCore.jbTorus.setDarkAngle(v); });
	p->setDefaultModParameters(-1.e+100, +1.e+100, -p->getMaxValue(), +p->getMaxValue(), jura::ModulationConnection::modModes::ABSOLUTE, .1);

	addObservedParameter(p = new ModulatableParameter2("DarkAngleSpeed", -20, +20, 0));
	//parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { jbTorusCore.jbTorus.phasorDarkAngle.setFrequency(v * MIDIMASTER.getMasterRate()); });
	p->setDefaultModParameters(-1.e+100, +1.e+100, -100, +100, jura::ModulationConnection::modModes::EXPONENTIAL, 20);
	p->setMapper(new jura::rsParameterMapperSinh(p->getMinValue(), p->getMaxValue(), 4));

	addObservedParameter(p = new ModulatableParameter2("DarkIntensity", 0, 100, 0));
	p->setValueChangeCallback([this](double v) { jbTorusCore.jbTorus.setDarkIntensity((int)v); });
	p->setDefaultModParameters(-1.e+100, +1.e+100, -p->getMaxValue(), +p->getMaxValue(), jura::ModulationConnection::modModes::EXPONENTIAL, 1);

	addObservedParameter(p = new ModulatableParameter2("RotX", -1, +1, 0, jura::Parameter::scalings::LINEAR_BIPOLAR));
	//parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { jbTorusCore.jbTorus.setRotX(v); });
	p->setDefaultModParameters(-1.e+100, +1.e+100, -p->getMaxValue(), +p->getMaxValue(), jura::ModulationConnection::modModes::ABSOLUTE, .1);

	addObservedParameter(p = new ModulatableParameter2("RotY", -1, +1, 0, jura::Parameter::scalings::LINEAR_BIPOLAR));
	//parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { jbTorusCore.jbTorus.setRotY(v); });
	p->setDefaultModParameters(-1.e+100, +1.e+100, -p->getMaxValue(), +p->getMaxValue(), jura::ModulationConnection::modModes::ABSOLUTE, .1);

	addObservedParameter(p = new ModulatableParameter2("RotZ", -1, +1, 0, jura::Parameter::scalings::LINEAR_BIPOLAR));
	//parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { jbTorusCore.jbTorus.setRotZ(v); });
	p->setDefaultModParameters(-1.e+100, +1.e+100, -p->getMaxValue(), +p->getMaxValue(), jura::ModulationConnection::modModes::ABSOLUTE, .1);

	addObservedParameter(p = new ModulatableParameter2("RotXSpeed", -20, +20, 0));
	//parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { jbTorusCore.jbTorus.phasorX.setFrequency(v * MIDIMASTER.getMasterRate()); });
	p->setDefaultModParameters(-1.e+100, +1.e+100, -100, +100, jura::ModulationConnection::modModes::EXPONENTIAL, 20);
	p->setMapper(new jura::rsParameterMapperSinh(p->getMinValue(), p->getMaxValue(), 4));

	addObservedParameter(p = new ModulatableParameter2("RotYSpeed", -20, +20, 0));
	//parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { jbTorusCore.jbTorus.phasorY.setFrequency(v * MIDIMASTER.getMasterRate()); });
	p->setDefaultModParameters(-1.e+100, +1.e+100, -100, +100, jura::ModulationConnection::modModes::EXPONENTIAL, 20);
	p->setMapper(new jura::rsParameterMapperSinh(p->getMinValue(), p->getMaxValue(), 4));

	addObservedParameter(p = new ModulatableParameter2("RotZSpeed", -20, +20, 0));
	//parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { jbTorusCore.jbTorus.phasorZ.setFrequency(v * MIDIMASTER.getMasterRate()); });
	p->setDefaultModParameters(-1.e+100, +1.e+100, -100, +100, jura::ModulationConnection::modModes::EXPONENTIAL, 20);
	p->setMapper(new jura::rsParameterMapperSinh(p->getMinValue(), p->getMaxValue(), 4));

	addObservedParameter(p = new ModulatableParameter2("ZAspectX", 0, 1, 0));
	//parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { jbTorusCore.jbTorus.setZAngleX(v); });
	p->setDefaultModParameters(-1.e+100, +1.e+100, -p->getMaxValue(), +p->getMaxValue(), jura::ModulationConnection::modModes::ABSOLUTE, .1);

	addObservedParameter(p = new ModulatableParameter2("ZAspectY", 0, 1, 0));
	//parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { jbTorusCore.jbTorus.setZAngleY(v); });
	p->setDefaultModParameters(-1.e+100, +1.e+100, -p->getMaxValue(), +p->getMaxValue(), jura::ModulationConnection::modModes::ABSOLUTE, .1);

	addObservedParameter(p = new ModulatableParameter2("ZDepth", 0, 5, 0));
	//parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { jbTorusCore.jbTorus.setZDepth(v); });
	p->setDefaultModParameters(-1.e+100, +1.e+100, -p->getMaxValue(), +p->getMaxValue(), jura::ModulationConnection::modModes::ABSOLUTE, .1);
}

//void JerobeamTorusModule::setupCallbacks()
//{
	//parFreqMult.setCallback([this](double v)
	//{
	//	MIDIMASTER.setMasterRate(v);

	//	jbTorusCore.jbTorus.phasorWander.setFrequency(parWander * v);
	//	jbTorusCore.jbTorus.phasorDarkAngle.setFrequency(parDarkAngleSpd * v);
	//	jbTorusCore.jbTorus.phasorX.setFrequency(parRotXSpd * v);
	//	jbTorusCore.jbTorus.phasorY.setFrequency(parRotYSpd * v);
	//	jbTorusCore.jbTorus.phasorZ.setFrequency(parRotZSpd * v);
	//});
//}

//void JerobeamTorusModule::setHelpTextAndLabels()
//{
	///* Common Controls */
	//parCoreGain.helpText =  "Gain of torus oscillator. Set to 0 to turn off. Useful for when you want to hear modulators through DC Offset modultion.";
	//parPhase.helpText = "Offset of torus phase. Use 0.5 phase to avoid clicks on attack.";

	///* Torus Controls */
	//parSubPhase.helpText = "Position of sub spiral along the main axis.";
	//parWanderSpd.helpText = "Rotation speed of sub spiral phase.";
	//
	//parSDensity.helpText = "Number of sub spiral rotations; increases harmonic content. Balance must be more than 0 to hear sub spirals.";
	//parDensityQuantize.helpText = "Rounds density down to an integer value. Use with sharp value of 1 for cleanest sound.";
	//parSubDensityQuantize.helpText = "Rounds sub density down to an integer value. Use with sharp value of 1 for cleanest sound.";
	//parBalance.helpText = "Increases amplitude of sub spirals.";
	//parSize.helpText = "Radius or width of the spiral rotation or brightness of the sound.";
	//parLength.helpText = "Length of torus along the main axis. Use 1 for cleanest sound. If not 1, sharp should also not be 1 or -1 for cleanest sound.";
	//parSharp.helpText = "Sharpness of the \"triangle\" movement of phase through the spiral oscillator. Avoid clicks by setting sharp to 1 with quantized density.";
	//parDarkAngle.helpText = "Where along the main axis to darken the spiral.";
	//parDarkAngleSpd.helpText = "Speed of rotation of the angle of darkening.";
	//parDarkIntensity.helpText = "Intensity of darkening of the Z-axis. Y Rotation must be a value other than +0.25 or -0.25 to hear the effect.";
	//parRotX.helpText = "Rotation of torus along its main axis.";
	//parRotXSpd.helpText = "Speed of rotation of the main axis.";
	//parRotYSpd.helpText = "Speed of rotation of Y rotation.";
	//parRotZSpd.helpText = "Speed of rotation of Z rotation.";
	//parRotY.helpText = "Rotation of torus along the Y axis.";
	//parRotZ.helpText = "Rotation of torus along the Z axis.";
	//parZAspectX.helpText = "Extrusion of spirals on the X axis. Z-Depth must be more than 0 to hear the effect.";
	//parZAspectY.helpText = "Extrusion of spirals on the Y axis. Z-Depth must be more than 0 to hear the effect.";
	//parZDepth.helpText = "Intensity of extrusion of spirals along the X or Y axis. Z-Aspect parameters must be more than 0 to hear the effect.";
//}

void JerobeamTorusModule::setSampleRate(double v)
{
	sampleRate = v;

	jbTorusCore.setSampleRate(v);
}

void JerobeamTorusModule::triggerAllNotesOff()
{
}

void JerobeamTorusModule::triggerAttack()
{
	if (MIDIMASTER.getCurrentChannel() == 2)
	{
		if (getMidiState().note == 0)
			MessageManager::callAsync([this]() { loadNextFile(); });
		else if (getMidiState().note == 1)
			MessageManager::callAsync([this]() { loadPreviousFile(); });

		sendChangeMessage();
		return;
	}
}

void JerobeamTorusModule::triggerNoteOnLegato()
{
}

void JerobeamTorusModule::triggerNoteOffLegato()
{
}

void JerobeamTorusModule::triggerRelease()
{
}

void JerobeamTorusModule::setBeatsPerMinute(double v)
{
	MIDIMASTER.setBPM(v);
}

void JerobeamTorusModule::setPlayheadInfo(AudioPlayHead::CurrentPositionInfo info)
{
	jbTorusCore.setPlayheadInfo(info);
}

void JerobeamTorusModule::processBlock(double **inOutBuffer, int numChannels, int numSamples)
{
	//double barPhase = getPhaseBasedOnBarsPerCycle(1);

	const bool doApplyModulations = modulationManager.getNumConnections() != 0;

	const bool doApplySmoothing = smoothingManager->needsSmoothing();

	for (int n = 0; n < numSamples; n++)
	{
	  double left = 0, right = 0;

		bool isSilent = jbTorusCore.isSilent();

		if (doApplySmoothing)
			smoothingManager->updateSmoothedValues();

		if (!isSilent && doApplyModulations)
			modulationManager.applyModulations();

		MIDIMASTER.incrementPitchGlide();

		if (!isSilent)
			jbTorusCore.processSampleFrame(&left, &right);

		if (inOutBuffer[0]) inOutBuffer[0][n] = left;
		if (inOutBuffer[1]) inOutBuffer[1][n] = right;
	}	
}
