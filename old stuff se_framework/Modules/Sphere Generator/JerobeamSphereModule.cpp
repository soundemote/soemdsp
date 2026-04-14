#include "JerobeamSphereModule.h"

JerobeamSphereModule::JerobeamSphereModule(CriticalSection *lockToUse, 
	jura::MetaParameterManager* metaManagerToUse)
	: BasicMidiPlugin(lockToUse, metaManagerToUse), jbSphereCore(this)
{
  ScopedLock scopedLock(*lock);

	// DO NOT CHANGE ORDERING
	setupModulesAndModulation(metaManagerToUse);
  createParameters();
	paramManager.instantiateParameters(this);
	setupCallbacks();
	setInitPatchValues();
	setHelpTextAndLabels();
	//loadDefaultPreset();
}

void JerobeamSphereModule::setupModulesAndModulation(jura::MetaParameterManager * metaManagerToUse)
{
	addMidiSlave(&jbSphereCore, MidiMaster::AddFor::NoteChange);
	addMidiSlave(&jbSphereCore, MidiMaster::AddFor::MonoNoteCallbacks);
	addMidiSlave(&jbSphereCore, MidiMaster::AddFor::MasterRateChange);
	addMidiSlave(&jbSphereCore, MidiMaster::AddFor::HostBPMChange);

	modulationManager.registerModulationSource(&modFeedback);
	modulationManager.registerModulationSource(&ampEnvOut);

	int i;

	i = 1; for (auto & obj : BasicEnvelopeModuleArray)
	{
		obj = new BasicEnvelopeModule(lock, metaManagerToUse, &modulationManager, this);

		MidiMaster::addMidiSlave(obj, MidiMaster::AddFor::MonoNoteCallbacks);

		obj->setModuleName("ADSR" + String(i));
		obj->setModulationSourceName("ADSR " + String(i));
		obj->setNoiseSeed(i);

		obj->resetParametersToDefaultValues();

		addChildAudioModule(obj);
		modulationManager.registerModulationSource(obj);

		++i;
	}

	i = 1; for (auto & obj : BasicOscillatorModuleArray)
	{
		obj = new BasicOscillatorModule(lock, metaManagerToUse, &modulationManager, this);

		MidiMaster::addMidiSlave(&obj->basicOscillatorCore, MidiMaster::AddFor::MonoNoteCallbacks);
		MidiMaster::addMidiSlave(&obj->basicOscillatorCore, MidiMaster::AddFor::MasterRateChange);
		MidiMaster::addMidiSlave(&obj->basicOscillatorCore, MidiMaster::AddFor::HostBPMChange);

		obj->setModuleName("LFO" + String(i));
		obj->setModulationSourceName("LFO " + String(i));
		obj->setNoiseSeed(i);

		obj->resetParametersToDefaultValues();

		addChildAudioModule(obj);
		modulationManager.registerModulationSource(obj);

		++i;
	}

	i = 1; for (auto & obj : MidiModSourceModuleArray)
	{
		obj = new MidiModSourceModule(lock, metaManagerToUse, &modulationManager, this);

		MidiMaster::addMidiSlave(obj, MidiMaster::AddFor::NoteChange);
		MidiMaster::addMidiSlave(obj, MidiMaster::AddFor::VelocityChange);

		obj->setModuleName("MIDI" + String(i));
		obj->setModulationSourceName("MIDI " + String(i));

		obj->resetParametersToDefaultValues();

		addChildAudioModule(obj);
		modulationManager.registerModulationSource(obj);

		++i;
	}

	Breakpoint1Module = new jura::BreakpointModulatorAudioModule(lock);
	Breakpoint1Module->setModuleName("Breakpoint_1");
	Breakpoint1Module->setModulationSourceName("Breakpoint 1");
	addChildAudioModule(Breakpoint1Module);
	modulationManager.registerModulationSource(Breakpoint1Module);

	Breakpoint2Module = new jura::BreakpointModulatorAudioModule(lock);
	Breakpoint2Module->setModuleName("Breakpoint_2");
	Breakpoint2Module->setModulationSourceName("Breakpoint 2");
	addChildAudioModule(Breakpoint2Module);
	modulationManager.registerModulationSource(Breakpoint2Module);
}

void JerobeamSphereModule::createParameters()
{
	vector<String> EnvShapeMenuItems ={ "EXP", "EXP2", "LOG", "S-CRV" };

	/* Common Controls */
	parChannelMode.initCombobox("Channel Mode", 0, MonoSynth::ChannelModeMenuItems);
	parStereoRotate.initSlider("Stereo Rotate", -1, 1, 0);

	parClippingMode.initCombobox("Clip Mode", 1, JerobeamSphereCore::ClippingModeMenuItems);
	parGain.initSlider("Gain", -60, 40, 3);
	parClippingLevel.initSlider("Clip Level", 1.e-3, 1.5, 1);

	parSmoothing.initSlider("Parameter Smoothing", 0.001, 1, .01);
	parOversampling.initSlider("Oversampling", 1, 8, 1);

	parDC.initSlider("DC Offset", -1, +1, 0);
	parX.initSlider("X Offset", -1, +1, 0);
	parY.initSlider("Y Offset", -1, +1, 0);
	parHPF.initSlider("Highpass", .01, 24000, .01);
	parLPF.initSlider("Lowpass", .01, 24000, 24000);

	parBP1Reset.initCombobox("BP1ResetMode", 2, { "Always", "On-note", "Legato" });
	parBP2Reset.initCombobox("BP2ResetMode", 2, { "Always", "On-note", "Legato" });

	parResetMode.initCombobox("Reset Mode", JerobeamSphereCore::ResetMode::when_not_legato, JerobeamSphereCore::ResetModeMenuItems);
	parNoteChangeMode.initCombobox("Note Change Mode", MidiMaster::NoteChangeMode::Always, MidiMaster::NoteChangeModeMenuItems);
	parResetWhenBeatChanges.initButton("Reset on tempo change", 0);
	parGlideMode.initCombobox("Glide Mode", 0, MidiMaster::GlideModeMenuItems);
	parFreqMode.initCombobox("Frequency Mode", 0, JerobeamSphereCore::KeytrackingMenuItems);
	parBeatFreq.initSlider("Frequency", -1000, +1000, 0);
	parBeatTempo.initSlider("Tempo", 0, 25, 0);
	parBeatMult.initSlider("Tempo Multiply", -12, +12, 0);
	parTune.initSlider("Tune", -72, 72, 0);
	parOctave.initSlider("Octave", -12, 12, 0);
	parFreqMult.initSlider("Frequency Multiply", -8, 8, 1);
	parHarmonicMult.initSlider("Harmonic Multiply", 1, 32, 1);

	parGlideAmt.initSlider("Glide Amount", 1.e-5, 1, 1.e-3);
	parHarmOctGlideAmt.initSlider("HarmOct Glide Amount", 1.e-5, 1, .01);

	/* Feedback Controls */
	parFeedbackHPF.initSlider("HP Feedback", .01, 24000, .01);
	parFeedbackLPF.initSlider("LP Feedback", .01, 24000, 24000);
	parFeedbackSrc.initCombobox("Feedback Source", 0, JerobeamSphereCore::FeedbackSourceMenuItems);
	parFeedbackAmp.initSlider("Feedback Amount", -5, +5, 0);

	/* Amp Env Controls */
	parEnvResetMode.initCombobox("EnvResetMode", 2, JerobeamSphereCore::EnvResetModeMenuItems);
	parEnvGlobalFB.initSlider("Speed", 100, 20000, 150);
	parEnvAttackFB.initSlider("Attack", 1.e-6, 3, 0.9);
	parEnvDecayFB.initSlider("Decay", 1.e-6, 3, 1.4);
	parEnvSustain.initSlider("Sustain", 0, 1, 1);
	parEnvReleaseFB.initSlider("Release", 1.e-6, 3, 1);
	parEnvGlobalTime.initSlider("Linear Time", .001, 20000, 100);
	parEnvAttackShape.initCombobox("Atk Shape", 2, EnvShapeMenuItems);
	parEnvDecayShape.initCombobox("Dec Shape", 0, EnvShapeMenuItems);
	parEnvReleaseShape.initCombobox("Rel Shape", 0, EnvShapeMenuItems);
	parEnvVelInfluence.initSlider("Vel Scale", 0, 1, 0);

	parCoreGain.initSlider("SphereGain", -1, 1, 1);
	parPhase.initSlider("Phase", -1, 1, 1);
	/* Sphere Controls */
	parDensity.initSlider("Density", 0, 100, 0);
	parSharpness.initSlider("Sharpness", -1, 1, 0);
	parRotX.initSlider("RotX", -180, 180, 0);
	parRotY.initSlider("RotY", -180, 180, 0);
	parZDepth.initSlider("ZDepth", 0, 1, 0);
	parZAmount.initSlider("ZAmount", 0, 1, 0);
	parEnds.initSlider("Ends", -10, 10, 0);
	parJump.initSlider("Jump", 0, 1, 0);
	parDirection.initSlider("Direction", 0, 1, 0);
	parBoing.initSlider("Boing", 0, 1, 0);
	parShape.initSlider("Shape", 0, 1, 0);
	parVolume.initSlider("Volume", 0, 2, 1);
	parVolumePreJump.initButton("VolumePreJump", 0);
}

void JerobeamSphereModule::setupCallbacks()
{
	parChannelMode.setCallback([this](double v) { jbSphereCore.setChannelMode((int)v); });
	parClippingMode.setCallback([this](double v) { jbSphereCore.setClipMode((int)v); });
	parClippingLevel.setCallback([this](double v) { jbSphereCore.setClipLevel(v); });
	parStereoRotate.setCallback([this](double v) { jbSphereCore.setRotation(v); });
	parGain.setCallback([this](double v) { jbSphereCore.setOutputAmplitude(rosic::dB2amp(v)); });

	parDC.setCallback([this](double v) { jbSphereCore.setDCOffset(v); });
	parX.setCallback([this](double v) { jbSphereCore.setXOffset(v); });
	parY.setCallback([this](double v) { jbSphereCore.setYOffset(v); });
	parHPF.setCallback([this](double v) { jbSphereCore.bpFilter.setHighpassCutoff(v); });
	parLPF.setCallback([this](double v) { jbSphereCore.bpFilter.setLowpassCutoff(v); });

	parFreqMode.setCallback([this](double v) { jbSphereCore.setKeytrackingMode((JerobeamSphereCore::KeytrackingMode)(int)(v)); });
	parNoteChangeMode.setCallback([this](double v) { MidiMaster::setNoteChangeMode((int)v); });

	parResetWhenBeatChanges.setCallback([this](double v) { jbSphereCore.resetWhenBeatTempoOrMultChanes = v >= 0.5; });

	parBeatFreq.setCallback([this](double v) { jbSphereCore.setFrequencyOffset(v); });
	parBeatTempo.setCallback([this](double v) { jbSphereCore.setTempoSelection((int)v); });
	parBeatMult.setCallback([this](double v) { jbSphereCore.setTempoMultiplier(v); });

	parTune.setCallback([this](double v) { jbSphereCore.setSemitoneOffset(v); });
	parOctave.setCallback([this](double v) { jbSphereCore.setOctaveOffset(v); });

	parFreqMult.setCallback([this](double v)
	{
		MidiMaster::setMasterRate(v);

		//jbSphereCore.jbSphere.phasorWander.setFrequency(parWander * v);
		//jbSphereCore.jbSphere.phasorDarkAngle.setFrequency(parDarkAngleSpd * v);
		//jbSphereCore.jbSphere.phasorX.setFrequency(parRotXSpd * v);
		//jbSphereCore.jbSphere.phasorY.setFrequency(parRotYSpd * v);
		//jbSphereCore.jbSphere.phasorZ.setFrequency(parRotZSpd * v);
	});

	parHarmonicMult.setCallback([this](double v) { jbSphereCore.setHarmonicMultiplier(v); });
	parHarmOctGlideAmt.setCallback([this](double v) { jbSphereCore.setHarmAndOctGlideAmt(v); });

	parGlideMode.setCallback([this](double v) { MidiMaster::setGlideMode((int)v); });
	parGlideAmt.setCallback([this](double v) { MidiMaster::setGlideSeconds(v); });

	parOversampling.setCallback([this](double v) { jbSphereCore.setOversampling((int)v); });

	parSmoothing.setCallback([this](double v)
	{
		setGlobalSmoothingSpeed(v);

		for (auto & obj : BasicEnvelopeModuleArray)
			obj->setGlobalSmoothingSpeed(v);

		for (auto & obj : BasicOscillatorModuleArray)
			obj->setGlobalSmoothingSpeed(v);
	});

	parFeedbackHPF.setCallback([this](double v) { jbSphereCore.setFeedbackHPCutoff(v); });
	parFeedbackLPF.setCallback([this](double v) { jbSphereCore.setFeedbackLPCutoff(v); });

	parFeedbackSrc.setCallback([this](double v) { jbSphereCore.setFeedbackSource((int)v); });
	parFeedbackAmp.setCallback([this](double v) { jbSphereCore.setFeedbackAmp(v); });

	/* AMPLITUDE ENVELOPE */
	parEnvVelInfluence.setCallback([this](double v) { jbSphereCore.ampEnv.setVelocityInfluence(v); });
	parEnvResetMode.setCallback([this](double v) { jbSphereCore.setAmpEnvResetMode((JerobeamSphereCore::EnvResetMode)(int)v); });
	parEnvGlobalTime.setCallback([this](double v) { jbSphereCore.ampEnv.setGlobalTime(v); });
	parEnvAttackFB.setCallback([this](double v) { jbSphereCore.ampEnv.setAttackFeedback(v); });
	parEnvDecayFB.setCallback([this](double v) { jbSphereCore.ampEnv.setDecayFeedback(v); });
	parEnvReleaseFB.setCallback([this](double v) { jbSphereCore.ampEnv.setReleaseFeedback(v); });
	parEnvSustain.setCallback([this](double v) { jbSphereCore.ampEnv.setSustainAmplitude(v); });
	parEnvGlobalFB.setCallback([this](double v) { jbSphereCore.ampEnv.setGlobalFeedback(v); });
	parEnvAttackShape.setCallback([this](double v) { jbSphereCore.ampEnv.setAttackShape((FeedbackADSR::Shape)(int)v); });
	parEnvDecayShape.setCallback([this](double v) { jbSphereCore.ampEnv.setDecayShape((FeedbackADSR::Shape)(int)v); });
	parEnvReleaseShape.setCallback([this](double v) { jbSphereCore.ampEnv.setReleaseShape((FeedbackADSR::Shape)(int)v); });

	parPhase.setCallback([this](double v) { jbSphereCore.jbSphere.phasor.setPhaseOffset(v); });
	parResetMode.setCallback([this](double v) { jbSphereCore.setResetMode((int)v); });
	/* Sphere Callbacks*/
	parDensity.setCallback([this](double v) { jbSphereCore.jbSphere.setDensity(v); });
	parSharpness.setCallback([this](double v) { jbSphereCore.jbSphere.setSharpness(v); });
	parRotX.setCallback([this](double v) { jbSphereCore.jbSphere.setRotX(v); });
	parRotY.setCallback([this](double v) { jbSphereCore.jbSphere.setRotY(v); });
	parZDepth.setCallback([this](double v) { jbSphereCore.jbSphere.setZDepth(v); });
	parZAmount.setCallback([this](double v) { jbSphereCore.jbSphere.setZAmount(v); });
	parEnds.setCallback([this](double v) { jbSphereCore.jbSphere.setEnds(v); });
	parBoing.setCallback([this](double v) { jbSphereCore.jbSphere.setBoing(v); });
	parJump.setCallback([this](double v) { jbSphereCore.jbSphere.setBoingStrength(v); });
	parDirection.setCallback([this](double v) { jbSphereCore.jbSphere.setDir(v); });
	parShape.setCallback([this](double v) { jbSphereCore.jbSphere.setShape(v); });
	parVolume.setCallback([this](double v) { jbSphereCore.jbSphere.setVolume(v); });
	parVolumePreJump.setCallback([this](double v) { jbSphereCore.jbSphere.setVolumePreJump(v); });
}

void JerobeamSphereModule::setInitPatchValues()
{

}

void JerobeamSphereModule::setHelpTextAndLabels()
{
	parFreqMult.text = "Master Rate";
	parResetMode.text = "Reset";
	parEnvResetMode.text = "Reset";
	parEnvGlobalTime.text = "<- Linear / Curved ->";

	parHarmonicMult.text = "Harmonic";

	parChannelMode.helpText = "Options for summing output to mono.";
	parStereoRotate.helpText = "Rotates stereo image similar to panning. Will cause mono to become stereo.";

	parClippingMode.helpText = "Hard or Soft clip. Soft will affect signal before limit is reached. Set Clip Level to 1.5 for more headroom";
	parGain.helpText = "Gain for the entire synth. Signal will be clipped at Clip Level amplitude.";
	parClippingLevel.helpText = "Sets the point of clipping. Soft clipping will affect signal before limit is reached.";

	parSmoothing.helpText = "Amount of smoothing applied for changes to GUI sliders.";
	parOversampling.helpText = "Applies antialiasing, increases CPU usage, and distorts image if viewed on oscilloscope. Set to x2 if not recording video.";

	parDC.helpText = "DC offset of left/right channels. Is affected by rotation & clipping. Modulate with LFO to use LFO use as audio source.";
	parX.helpText = "DC offset of left channel. Is affected by rotation and clipping.";
	parY.helpText = "DC offset of right channel. Is affected by rotation and clipping.";
	parHPF.helpText = "Applies highpass filtering to final output. Be careful when using DC with no HP filtering.";
	parLPF.helpText = "Applies lowpass filtering to final output.";

	parBP1Reset.helpText = "Always: Reset on note on/off except for final off. / On-note: only for note on. / Legato: when note is not tied.";
	parBP1Reset.helpText = "Always: Reset on note on/off except for final off. / On-note: only for note on. / Legato: when note is not tied.";

	parResetMode.helpText = "Always: Resets phase every note / Legato: when note is not tied  / One-shot: Trigger release after one oscillation. ";
	parResetWhenBeatChanges.helpText = "Help keep things on tempo by resetting phase when tempo settings change.";
	parGlideMode.helpText = "Choose when to apply pitch glide.";
	parNoteChangeMode.helpText = "Always: Note changed as expected for legato playing / On Note: Only when key is pressed, good for percussion.";
	parFreqMode.helpText = "If off, midi notes do not change pitch. Use 'Off' for tempo-synced oscillations.";
	parBeatFreq.helpText = "Offsets pitch linearly. Allows for off-pitch beating. Modulate with LFO for classic \"in-tune\" FM, or with feedback for growls.";
	parBeatTempo.helpText = "Choose a beat multiplier. Will offset pitch unless set to 0/0. x1 = whole / x4 = quarter / x5.333 = quater dotted.";
	parBeatMult.helpText = "Doubles or halves the tempo setting. This does not affect LFOs/ADSRs/Breakpoints.";
	parTune.helpText = "Pitch offset in semitones. Modulate this for exponential FM. This will not stay in tune, but will sounds great for fx.";
	parOctave.helpText = "Pitch offset in octaves (quantized)";
	parFreqMult.helpText = "Master frequency of LFOs and synth. Modulating this with an LFO will cause frequency feedback. Try it!";
	parHarmonicMult.helpText = "Multiplies frequency by number of harmonics. Modulate with LFO to create fun little melodies. Add feedback for even more fun.";

	parGlideAmt.helpText = "Speed of transition between pitches.";
	parHarmOctGlideAmt.helpText = "Speed of transition when setting octave of harmonic";

	/* Feedback Controls */
	parFeedbackHPF.helpText = "Highpass filter applied to feedback source. Important for when modulating frequency to remain pitch centered.";
	parFeedbackLPF.helpText = "Lowpass filter applied to feedback source. Use to reduce harmonic content.";
	parFeedbackSrc.helpText = "Ramp: upwards sawtooth derived from oscillator timer / Triangle: Same as ramp except affected by Sharp control.";
	parFeedbackAmp.helpText = "Feedback gain. To use feedback: right-click on slider -> modulation setup -> add Feedback Source.";

	/* Amp Env Controls */
	parEnvResetMode.helpText = "Always: Reset on note on/off except for final off. / On-note: only for note on. / Legato: when not tied. / Never: Do not reset.";
	parEnvGlobalFB.helpText = "Increases speed of the entire envelope. Use this control after finding a good envelope shape.";
	parEnvAttackFB.helpText = "Increases attack time.";
	parEnvDecayFB.helpText = "Increases decay time.";
	parEnvSustain.helpText = "Sets level of sustain.";
	parEnvReleaseFB.helpText = "Increases release time.";
	parEnvGlobalTime.helpText = "Sets envelope stages to be more or less linearly-shaped. Also affects envlope speed. Compensate for this using other parameters.";
	parEnvAttackShape.helpText = "Choose shape of envelope stage. Generally LOG for attack. Try S-CURVE for blown-instrument-style envelope and to tame clicks.";
	parEnvDecayShape.helpText = "Choose shape of envelope stage. Generally EXP for attack. Try EXP2 for extreme pluck sound and vactrol emulation.";
	parEnvReleaseShape.helpText = "Choose shape of envelope stage. Generally EXP for release. Try S-CURVE for blown-instrument-style envelope and to tame clicks.";
	parEnvVelInfluence.helpText = "Sets how much influence midi velocity has on envelop. 0 means no infleunce.";

	parCoreGain.helpText = "Gain of radar oscillator. Set to 0 to turn off. Useful for when you want to hear modulators through DC Offset modultion.";
	parPhase.helpText = "Offset of radar oscillator phase.";
	/* Sphere Controls */
	parDensity.helpText = "";
	parSharpness.helpText = "";
	parRotX.helpText = "";
	parRotY.helpText = "";
	parZDepth.helpText = "";
	parZAmount.helpText = "";
	parEnds.helpText = "";
	parBoing.helpText = "";
	parJump.helpText = "";
	parDirection.helpText = "";
	parShape.helpText = "";
	parVolume.helpText = "";
	parVolumePreJump.helpText = "";
}

void JerobeamSphereModule::setSampleRate(double v)
{
	sampleRate = v;
	//LinL.setSampleRate(v);
	//LinR.setSampleRate(v);
	//LinL.setSmoothingSeconds(1 / v * 2);
	//LinR.setSmoothingSeconds(1 / v * 2);

	jbSphereCore.setSampleRate(v);

	for (auto & obj : childModules)
		obj->setSampleRate(v);
}

void JerobeamSphereModule::allNotesOff()
{
	MidiMaster::allNotesOff();

	Breakpoint1Module->noteOff(false);
	Breakpoint2Module->noteOff(false);
}

void JerobeamSphereModule::triggerAttack()
{
	MidiMaster::triggerAttack();

	Breakpoint1Module->noteOn(parBP1Reset == (double)JerobeamSphereCore::EnvResetMode::env_never, MidiMaster::getCurrentNote(), MidiMaster::getCurrentVelocity());
	Breakpoint2Module->noteOn(parBP2Reset == (double)JerobeamSphereCore::EnvResetMode::env_never, MidiMaster::getCurrentNote(), MidiMaster::getCurrentVelocity());
}

void JerobeamSphereModule::triggerNoteOnLegato()
{
	MidiMaster::triggerNoteOnLegato();

	if (parBP1Reset < (double)JerobeamSphereCore::EnvResetMode::env_when_not_legato)
		Breakpoint1Module->noteOn(false, MidiMaster::getCurrentNote(), MidiMaster::getCurrentVelocity());
	if (parBP2Reset < (double)JerobeamSphereCore::EnvResetMode::env_when_not_legato)
		Breakpoint2Module->noteOn(false, MidiMaster::getCurrentNote(), MidiMaster::getCurrentVelocity());
}

void JerobeamSphereModule::triggerNoteOffLegato()
{
	MidiMaster::triggerNoteOffLegato();

	if (parBP1Reset < (double)JerobeamSphereCore::EnvResetMode::env_on_note)
		Breakpoint1Module->noteOn(false, MidiMaster::getCurrentNote(), MidiMaster::getCurrentVelocity());
	if (parBP2Reset < (double)JerobeamSphereCore::EnvResetMode::env_on_note)
		Breakpoint2Module->noteOn(false, MidiMaster::getCurrentNote(), MidiMaster::getCurrentVelocity());
}

void JerobeamSphereModule::triggerRelease()
{
	MidiMaster::triggerRelease();

	/* if statements don't work, we need a feature "start from current time" */
	//if (parBP1Reset < (double)MonoSynth::EnvResetMode::env_never)
	Breakpoint1Module->noteOff(MidiMaster::getCurrentNote());
	//if (parBP2Reset < (double)MonoSynth::EnvResetMode::env_never)
	Breakpoint2Module->noteOff(MidiMaster::getCurrentNote());
}

void JerobeamSphereModule::setBeatsPerMinute(double newBpm)
{
	MidiMaster::setBPM(newBpm);

	Breakpoint1Module->setBeatsPerMinute(newBpm*0.25);
	Breakpoint2Module->setBeatsPerMinute(newBpm*0.25);
}

void JerobeamSphereModule::setPlayheadInfo(AudioPlayHead::CurrentPositionInfo info)
{
	jbSphereCore.setPlayheadInfo(info);
}

void JerobeamSphereModule::setPitchBend(int pitchBendValue)
{
	for (auto & obj : MidiModSourceModuleArray)
		obj->setPitchBend(pitchBendValue);
}

void JerobeamSphereModule::setChannelPressure(int channelPressureValue)
{
	for (auto & obj : MidiModSourceModuleArray)
		obj->setChannelPressure(channelPressureValue);
}

void JerobeamSphereModule::setMidiController(int controllerNumber, int controllerValue)
{
	for (auto & obj : MidiModSourceModuleArray)
		obj->setMidiController(controllerNumber, (float)controllerValue);
}

void JerobeamSphereModule::processBlock(double **inOutBuffer, int numChannels, int numSamples)
{
	//double barPhase = getPhaseBasedOnBarsPerCycle(1);

	const bool doApplyModulations = modulationManager.getNumConnections() != 0;

	const bool doApplySmoothing = smoothingManager->needsSmoothing();

	double left = 0, right = 0;
	for (int n = 0; n < numSamples; n++)
	{
		bool isSilent = jbSphereCore.isSilent();

		if (doApplySmoothing)
			smoothingManager->updateSmoothedValues();

		if (!isSilent && doApplyModulations)
			modulationManager.applyModulations();

		MidiMaster::incrementPitchGlide();

		if (!isSilent)
			jbSphereCore.processSampleFrame(&left, &right);

		//LinL.setTargetValue(left);
		//LinR.setTargetValue(right);

		if (inOutBuffer[0]) inOutBuffer[0][n] = left/*LinL.getSample()*/;
		if (inOutBuffer[1]) inOutBuffer[1][n] = right/*LinR.getSample()*/;
	}
}
