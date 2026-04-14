#include "JerobeamMushroomModule.h"

#undef RELATIVE
#undef ABSOLUTE

JerobeamMushroomModule::JerobeamMushroomModule(CriticalSection *lockToUse,
	jura::MetaParameterManager* metaManagerToUse)
	: BasicModule(lockToUse, metaManagerToUse)	
{
  ScopedLock scopedLock(*lock);

	setModuleLink("https://www.soundemote.com/products/Mushroom-generator");
	setModuleNameString("Mushroom Generator", "MushroomGenerator");
	AudioModule::patchFormatIndex = 2;
	PluginFileManager::setModuleVersionString("1.4.0");
	setActiveDirectory(presetPath);

	BasicModule::setupParametersAndModules();

	setModulationManager(&modulationManager);
	modulationManager.setMetaParameterManager(metaParamManager);

	loadFile(File(initPresetPath));
}

void JerobeamMushroomModule::setupModulesAndModulation()
{
	MIDIMASTER.addMidiSlave(this, MidiMaster::AddFor::MonoNoteCallbacks);

	MIDIMASTER.addMidiSlave(&jbMushroomCore, MidiMaster::AddFor::NoteChange);
	MIDIMASTER.addMidiSlave(&jbMushroomCore, MidiMaster::AddFor::MonoNoteCallbacks);
	MIDIMASTER.addMidiSlave(&jbMushroomCore, MidiMaster::AddFor::MasterRateChange);
	MIDIMASTER.addMidiSlave(&jbMushroomCore, MidiMaster::AddFor::HostBPMChange);

	modulationManager.registerModulationSource(&modFeedback);
	modulationManager.registerModulationSource(&ampEnvOut);

	int i;

	i = 1; for (auto & obj : BasicEnvelopeModuleArray)
	{
		obj = new BasicEnvelopeModule(lock, metaParamManager, &modulationManager, &MIDIMASTER);

		MIDIMASTER.addMidiSlave(obj, MidiMaster::AddFor::MonoNoteCallbacks);

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
		obj = new BasicOscillatorModule(lock, metaParamManager, &modulationManager, &MIDIMASTER);

		MIDIMASTER.addMidiSlave(&obj->basicOscillatorCore, MidiMaster::AddFor::MonoNoteCallbacks);
		MIDIMASTER.addMidiSlave(&obj->basicOscillatorCore, MidiMaster::AddFor::MasterRateChange);
		MIDIMASTER.addMidiSlave(&obj->basicOscillatorCore, MidiMaster::AddFor::HostBPMChange);

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
		obj = new MidiModSourceModule(lock, metaParamManager, &modulationManager, &MIDIMASTER);

		MIDIMASTER.addMidiSlave(obj, MidiMaster::AddFor::NoteChange);
		MIDIMASTER.addMidiSlave(obj, MidiMaster::AddFor::VelocityChange);
		MIDIMASTER.addMidiSlave(obj, MidiMaster::AddFor::MidiInterfaceCallbacks);

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
	modulationManager.registerModulationSource(Breakpoint1Module);
	addChildAudioModule(Breakpoint1Module);

	Breakpoint2Module = new jura::BreakpointModulatorAudioModule(lock);
	Breakpoint2Module->setModuleName("Breakpoint_2");
	Breakpoint2Module->setModulationSourceName("Breakpoint 2");
	modulationManager.registerModulationSource(Breakpoint2Module);
	addChildAudioModule(Breakpoint2Module);

	equalizerModule = new jura::EqualizerAudioModule(lock, &equalizer);
	equalizerModule->setModuleName("Equalizer");
	equalizerModule->getParameterByName("Bypass")->setValue(1, true, true);
	addChildAudioModule(equalizerModule);

	limiterModule = new ElanLimiterModule(lock, metaParamManager, &modulationManager);
	limiterModule->setModuleName("Limiter");
	addChildAudioModule(limiterModule);

	jbMushroomCore.bandpassModule = new BandpassModule(lock, metaParamManager, &modulationManager);
	jbMushroomCore.bandpassModule->setModuleName("Filter");
	addChildAudioModule(jbMushroomCore.bandpassModule);

	delayModule = new DelayModule(lock, metaParamManager, &modulationManager);
	delayModule->setModuleName("Delay");
	addChildAudioModule(delayModule);

	oscilloscopeModule = new OscilloscopeModule(lock, metaParamManager, &modulationManager);
	oscilloscopeModule->setModuleName("Oscilloscope");
	addChildAudioModule(oscilloscopeModule);
}

void JerobeamMushroomModule::createParameters()
{
	vector<String> EnvShapeMenuItems ={ "EXP", "EXP2", "LOG", "S-CRV" };

	/* Common Controls */
	parChannelMode.initCombobox("Channel Mode", 0, MonoSynth::ChannelModeMenuItems);
	parStereoRotate.initSlider("Stereo Rotate", -1, 1, 0);

	parClippingMode.initCombobox("Clip Mode", 1, JerobeamMushroomCore::ClippingModeMenuItems);
	parGain.initSlider("Gain", -60, 40, -6);
	parClippingLevel.initSlider("Clip Level", 1.e-3, 1.5, 1);

	parSmoothing.initSlider("Parameter Smoothing", 0.001, 1, .01);
	parOversampling.initSlider("Oversampling", 1, 8, 1);

	parDC.initSlider("DC Offset", -1, +1, 0);
	parX.initSlider("X Offset", -1, +1, 0);
	parY.initSlider("Y Offset", -1, +1, 0);

	parBP1Reset.initCombobox("BP1ResetMode", 2, { "Always", "On-note", "Legato" });
	parBP2Reset.initCombobox("BP2ResetMode", 2, { "Always", "On-note", "Legato" });

	parResetMode.initCombobox("Reset Mode", JerobeamMushroomCore::ResetMode::when_not_legato, JerobeamMushroomCore::ResetModeMenuItems);
	parNoteChangeMode.initCombobox("Note Change Mode", MidiMaster::NoteChangeMode::Always, MidiMaster::NoteChangeModeMenuItems);
	parResetWhenBeatChanges.initButton("Reset on tempo change", 0);
	parGlideMode.initCombobox("Glide Mode", 0, MidiMaster::GlideModeMenuItems);
	parFreqMode.initCombobox("Frequency Mode", 0, JerobeamMushroomCore::KeytrackingMenuItems);
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
	parFeedbackHPF.initSlider("HP Feedback", .01, 24000, 30);
	parFeedbackLPF.initSlider("LP Feedback", .01, 24000, 10000);
	parFeedbackSrc.initCombobox("Feedback Source", 0, JerobeamMushroomCore::FeedbackSourceMenuItems);
	parFeedbackAmp.initSlider("Feedback Amount", -5, +5, 0);

	/* Amp Env Controls */
	parEnvResetMode.initCombobox("EnvResetMode", 2, JerobeamMushroomCore::EnvResetModeMenuItems);
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

	/* Mushroom Controls */
	parPhaseOffset.initSlider("Phase", -1, 1, 0);
	parCoreGain.initSlider("ShroomGain", -1, 1, 1);
	parGrow.initSlider("Grow", 0, 1, 1);
	parWidth.initSlider("Width", -2, 2, 1);
	parMirror.initSlider("Mirror", -1, 1, 0);
	parDensity.initSlider("Density", .1, 500, 3);
	parSharp.initSlider("Sharp", -1, 1, 0);
	parCapStemRatio.initSlider("CapStem Ratio", 0, 1, 0.5);
	parCapStemTransition.initSlider("CapStem Transition", -10, 10, 0);
	parStemWidth.initSlider("Stem Width", 0, 1, 0);
	parStemTwist.initSlider("Stem Twist", 0, 1, 0);
	parNumMushrooms.initSlider("Number of Shrooms", -31, 31, 1);
	parClusterSpread.initSlider("Cluster Spread", -1, 1, 0);

	parCapStemRotation.initSlider("CapStem Rotation", -1, +1, 0);
	parClusterRotation.initSlider("Cluster Rotation", -1, +1, 0);
	parClusterRotSpd.initSlider("Cluster Rotation Spd", -20, +20, 0);
	parCapStemRotSpd.initSlider("CapStem Rotation Spd", -10, +10, 0);

	parNumMushrooms.mapper = new ParameterMapperLinearBipolarSkipZero(parNumMushrooms.mini, parNumMushrooms.maxi);

	parBeatTempo.stringConvertFunc = elan::tempoMulToString;
	parBeatMult.stringConvertFunc = elan::beatMulToString;
	parGlideAmt.stringConvertFunc = elan::secondsToStringWithUnitTotal4;
	parHarmOctGlideAmt.stringConvertFunc = elan::secondsToStringWithUnitTotal4;
	parSmoothing.stringConvertFunc = elan::secondsToStringWithUnitTotal4;
	parNumMushrooms.stringConvertFunc = elan::numShroomsToString;

	paramManager.instantiateParameters(this);
}

void JerobeamMushroomModule::setupCallbacks()
{
	parChannelMode.setCallback([this](double v) { jbMushroomCore.setChannelMode((int)v); });
	parClippingMode.setCallback([this](double v) { jbMushroomCore.setClipMode((int)v); });
	parClippingLevel.setCallback([this](double v) { jbMushroomCore.setClipLevel(v); });
	parStereoRotate.setCallback([this](double v) { jbMushroomCore.setRotation(v); });
	parGain.setCallback([this](double v) { jbMushroomCore.setOutputAmplitude(elan::dbToAmp(v)); });

	parDC.setCallback([this](double v) { jbMushroomCore.setDCOffset(v); });
	parX.setCallback([this](double v) { jbMushroomCore.setXOffset(v); });
	parY.setCallback([this](double v) { jbMushroomCore.setYOffset(v); });

	parFreqMode.setCallback([this](double v) { jbMushroomCore.setKeytrackingMode((JerobeamMushroomCore::KeytrackingMode)(int)(v)); });
	parNoteChangeMode.setCallback([this](double v) { MIDIMASTER.setNoteChangeMode((int)v); });

	parResetWhenBeatChanges.setCallback([this](double v) { jbMushroomCore.resetWhenBeatTempoOrMultChanes = v >= 0.5; });

	parBeatFreq.setCallback([this](double v) { jbMushroomCore.setFrequencyOffset(v); });
	parBeatTempo.setCallback([this](double v) { jbMushroomCore.setTempoSelection((int)v); });
	parBeatMult.setCallback([this](double v) { jbMushroomCore.setTempoMultiplier(v); });

	parTune.setCallback([this](double v) { jbMushroomCore.setSemitoneOffset(v); });
	parOctave.setCallback([this](double v) { jbMushroomCore.setOctaveOffset(v); });

	parFreqMult.setCallback([this](double v)
	{
		MIDIMASTER.setMasterRate(v);

		jbMushroomCore.jbMushroom.capstemRotator.setFrequency(parCapStemRotSpd * v);
		jbMushroomCore.jbMushroom.clusterRotator.setFrequency(parClusterRotSpd * v);
	});

	parHarmonicMult.setCallback([this](double v) { jbMushroomCore.setHarmonicMultiplier(v); });
	parHarmOctGlideAmt.setCallback([this](double v) { jbMushroomCore.setHarmAndOctGlideAmt(v); });

	parGlideMode.setCallback([this](double v) { MIDIMASTER.setGlideMode((int)v); });
	parGlideAmt.setCallback([this](double v) { MIDIMASTER.setGlideSeconds(v); });

	parOversampling.setCallback([this](double v) { jbMushroomCore.setOversampling((int)v); });

	parSmoothing.setCallback([this](double v)
	{
		setGlobalSmoothingSpeed(v);

		for (auto & obj : BasicEnvelopeModuleArray)
			obj->setGlobalSmoothingSpeed(v);

		for (auto & obj : BasicOscillatorModuleArray)
			obj->setGlobalSmoothingSpeed(v);

		limiterModule->setGlobalSmoothingSpeed(v);
		delayModule->setGlobalSmoothingSpeed(v);
	});

	parFeedbackHPF.setCallback([this](double v) { jbMushroomCore.setFeedbackHPCutoff(v); });
	parFeedbackLPF.setCallback([this](double v) { jbMushroomCore.setFeedbackLPCutoff(v); });

	parFeedbackSrc.setCallback([this](double v) { jbMushroomCore.setFeedbackSource((int)v); });
	parFeedbackAmp.setCallback([this](double v) { jbMushroomCore.setFeedbackAmp(v); });

	/* AMPLITUDE ENVELOPE */
	parEnvVelInfluence.setCallback([this](double v) { jbMushroomCore.ampEnv.setVelocityInfluence(v); });
	parEnvResetMode.setCallback([this](double v) { jbMushroomCore.setAmpEnvResetMode((JerobeamMushroomCore::EnvResetMode)(int)v); });
	parEnvGlobalTime.setCallback([this](double v) { jbMushroomCore.ampEnv.setGlobalTime(v); });
	parEnvAttackFB.setCallback([this](double v) { jbMushroomCore.ampEnv.setAttackFeedback(v); });
	parEnvDecayFB.setCallback([this](double v) { jbMushroomCore.ampEnv.setDecayFeedback(v); });
	parEnvReleaseFB.setCallback([this](double v) { jbMushroomCore.ampEnv.setReleaseFeedback(v); });
	parEnvSustain.setCallback([this](double v) { jbMushroomCore.ampEnv.setSustainAmplitude(v); });
	parEnvGlobalFB.setCallback([this](double v) { jbMushroomCore.ampEnv.setGlobalFeedback(v); });
	parEnvAttackShape.setCallback([this](double v) { jbMushroomCore.ampEnv.setAttackShape((FeedbackADSR::Shape)(int)v); });
	parEnvDecayShape.setCallback([this](double v) { jbMushroomCore.ampEnv.setDecayShape((FeedbackADSR::Shape)(int)v); });
	parEnvReleaseShape.setCallback([this](double v) { jbMushroomCore.ampEnv.setReleaseShape((FeedbackADSR::Shape)(int)v); });

	/* Mushroom Callbacks */
	parCoreGain.setCallback([this](double v) { jbMushroomCore.setInternalGain(v); });
	parGrow.setCallback([this](double v) { jbMushroomCore.jbMushroom.setGrow(v); });
	parDensity.setCallback([this](double v) { jbMushroomCore.jbMushroom.setDensity(v); });
	parCapStemRatio.setCallback([this](double v) { jbMushroomCore.jbMushroom.setHead(v); });
	parCapStemTransition.setCallback([this](double v) { jbMushroomCore.jbMushroom.setCapStemTransition(v); });
	parClusterSpread.setCallback([this](double v) { jbMushroomCore.jbMushroom.setSpread(v); });
	parStemTwist.setCallback([this](double v) { jbMushroomCore.jbMushroom.setWobble(v); });
	parSharp.setCallback([this](double v) { jbMushroomCore.jbMushroom.setSharp(v); });
	parNumMushrooms.setCallback([this](double v) { jbMushroomCore.jbMushroom.setNumMushrooms((int)v); });
	parWidth.setCallback([this](double v) { jbMushroomCore.jbMushroom.setWidth(v); });
	parStemWidth.setCallback([this](double v) { jbMushroomCore.jbMushroom.setStem(v); });
	parMirror.setCallback([this](double v) { jbMushroomCore.jbMushroom.setApart(v); });
	parClusterRotation.setCallback([this](double v) { jbMushroomCore.jbMushroom.setClusterRotation(v); });
	parCapStemRotation.setCallback([this](double v) { jbMushroomCore.jbMushroom.setCapRotation(v); });
	parCapStemRotSpd.setCallback([this](double v) { jbMushroomCore.jbMushroom.setStemRotationSpd(v * MIDIMASTER.getMasterRate()); });
	parClusterRotSpd.setCallback([this](double v) { jbMushroomCore.jbMushroom.setClusterRotationSpd(v * MIDIMASTER.getMasterRate()); });
}

void JerobeamMushroomModule::setInitPatchValues()
{
	parFeedbackAmp.setValue(1);

	for (auto & obj : BasicOscillatorModuleArray)
		obj->parFrequency.setValue(4);

	delayModule->parBypass.setValue(1);

	jbMushroomCore.ampEnv.setDelayTime(0);
	jbMushroomCore.ampEnv.setAttackTime(0);
	jbMushroomCore.ampEnv.setDecayTime(0);
	jbMushroomCore.ampEnv.setReleaseTime(0);
}

void JerobeamMushroomModule::setHelpTextAndLabels()
{
	parFreqMult.text = "Master Rate";
	parClusterRotSpd.text = "^- Rotation Speed";
	parCapStemRotSpd.text = "^- Rotation Speed";
	parResetMode.text = "Reset";
	parEnvResetMode.text = "Reset";
	parCoreGain.text = "Shroom Gain";
	parEnvGlobalTime.text = "<- Linear / Curved ->";
	parStemTwist.text = "Twist";
	parNumMushrooms.text = "Number of Shrooms";

	parHarmonicMult.text = "Harmonic";

	parX.text = "X";
	parY.text = "Y";

	parSmoothing.text = "Smoothing";

	parStereoRotate.text = "Rotate";

	parChannelMode.helpText = "Options for summing output to mono.";
	parStereoRotate.helpText = "Rotates stereo image similar to panning. Will cause mono to become stereo.";

	parClippingMode.helpText = "Hard or Soft clip. Soft will affect signal before limit is reached. Set Clip Level to 1.5 for more headroom";
	parGain.helpText = "Gain for the entire synth. Signal will be clipped at Clip Level amplitude.";
	parClippingLevel.helpText = "Sets the point of clipping. Soft clipping will affect signal before limit is reached.";

	parSmoothing.helpText = "Amount of smoothing applied to sliders / controls, as well as host automation.";
	parOversampling.helpText = "Applies antialiasing, increases CPU usage, and distorts image if viewed on oscilloscope. Set to x2 if not recording video.";

	parDC.helpText = "Monophonic DC offset (affecting left/right channels equally). Is affected by rotation and clipping.";
	parX.helpText = "DC offset of left channel. Is affected by rotation and clipping.";
	parY.helpText = "DC offset of right channel. Is affected by rotation and clipping.";

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

	/* Mushroom Controls */
	parCoreGain.helpText = "Gain of mushroom oscillator. Set to 0 to turn off. Useful for when you want to hear modulators through DC Offset modultion.";
	parGrow.helpText = "Height of mushroom. \"Grow, my little mushrooms, GROOOWW!!!\" -last words of Master Shroomy";
	parWidth.helpText = "Controls width of cap and stem. Stem must have some 'Stem Width'. It must.";
	parMirror.helpText = "Magically doubles the number of mushrooms. Use to layer a sub-octave oscillation for organic mushroom sounds.";
	parDensity.helpText = "Spiral density of mushroom cap. Use whole numbers to avoid clicks when using 'CapStem' rotation.";
	parSharp.helpText = "Sharpness of triangle oscillator. Use to add harmonics. Triangle controls the movement of phase through the mushroom oscillation.";
	parCapStemRatio.helpText = "Rotates cap and stem. Set 'Density' to a whole number to avoid clicks.";
	parCapStemTransition.helpText = "Soften the transition between cap and stem to reduce click/buzz/";
	parStemWidth.helpText = "Stem spiral width. Is affected by 'Width' control as well.";
	parStemTwist.helpText = "Causes mushrooms to twist together, forming protective barrier against all magical attacks.";
	parNumMushrooms.helpText = "Increases the total # of shrooms in your garden. Be careful of the anti-shrooms, they look very similar. ";
	parClusterSpread.helpText = "Sometimes mushrooms like to hide behind one another. Add spread before consuming.";

	parCapStemRotation.helpText = "Phase of CapStep rotator.";
	parClusterRotation.helpText = "Spins the entire mushroom colony. \"Spin, my little mushrooms, SPEEENN!!!\" -second to last words of Master Shroomy";
	parClusterRotSpd.helpText = "Phase of ClusterRotator";
	parCapStemRotSpd.helpText = "Rotates cap and step. Use whole numbers of 'Density' to avoid clicks.";

	loadFile(File(initPresetPath));
}

void JerobeamMushroomModule::setSampleRate(double v)
{
	sampleRate = v;

	jbMushroomCore.setSampleRate(v);

	for (auto & obj : BasicEnvelopeModuleArray)
		obj->setSampleRate(v);

	for (auto & obj : BasicOscillatorModuleArray)
		obj->setSampleRate(v);

	for (auto & obj : MidiModSourceModuleArray)
		obj->setSampleRate(v);

	Breakpoint1Module->setSampleRate(v);
	Breakpoint2Module->setSampleRate(v);
	equalizerModule->setSampleRate(v);
	limiterModule->jura::AudioModule::setSampleRate(v);
	delayModule->jura::AudioModule::setSampleRate(v);
	oscilloscopeModule->jura::AudioModule::setSampleRate(v);
}

void JerobeamMushroomModule::triggerAllNotesOff()
{
	/* if statements don't work, we need a feature "start from current time" */
	//if (parBP1Reset < (double)MonoSynth::EnvResetMode::env_never)
	Breakpoint1Module->noteOff(getMidiState().note);
	//if (parBP2Reset < (double)MonoSynth::EnvResetMode::env_never)
	Breakpoint2Module->noteOff(getMidiState().note);
}

void JerobeamMushroomModule::triggerAttack()
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

	Breakpoint1Module->noteOn(parBP1Reset == (double)JerobeamMushroomCore::EnvResetMode::env_never, getMidiState().note, int(getMidiState().velocity) * 127);
	Breakpoint2Module->noteOn(parBP2Reset == (double)JerobeamMushroomCore::EnvResetMode::env_never, getMidiState().note, int(getMidiState().velocity) * 127);
}

void JerobeamMushroomModule::triggerNoteOnLegato()
{
	if (parBP1Reset < (double)JerobeamMushroomCore::EnvResetMode::env_when_not_legato)
		Breakpoint1Module->noteOn(false, MIDIMASTER.getState().note, int(MIDIMASTER.getState().velocity) * 127);
	if (parBP2Reset < (double)JerobeamMushroomCore::EnvResetMode::env_when_not_legato)
		Breakpoint2Module->noteOn(false, MIDIMASTER.getState().note, int(MIDIMASTER.getState().velocity) * 127);
}

void JerobeamMushroomModule::triggerNoteOffLegato()
{
	if (parBP1Reset < (double)JerobeamMushroomCore::EnvResetMode::env_on_note)
		Breakpoint1Module->noteOn(false, MIDIMASTER.getState().note, int(MIDIMASTER.getState().velocity) * 127);
	if (parBP2Reset < (double)JerobeamMushroomCore::EnvResetMode::env_on_note)
		Breakpoint2Module->noteOn(false, MIDIMASTER.getState().note, int(MIDIMASTER.getState().velocity) * 127);
}

void JerobeamMushroomModule::triggerRelease()
{
	/* if statements don't work, we need a feature "start from current time" */
	//if (parBP1Reset < (double)MonoSynth::EnvResetMode::env_never)
	Breakpoint1Module->noteOff(MIDIMASTER.getState().note);
	//if (parBP2Reset < (double)MonoSynth::EnvResetMode::env_never)
	Breakpoint2Module->noteOff(MIDIMASTER.getState().note);
}

void JerobeamMushroomModule::setBeatsPerMinute(double v)
{
	MIDIMASTER.setBPM(v);

	Breakpoint1Module->setBeatsPerMinute(v*0.25);
	Breakpoint2Module->setBeatsPerMinute(v*0.25);

	delayModule->delay.setTempoInBPM(v);
}

void JerobeamMushroomModule::setPlayheadInfo(AudioPlayHead::CurrentPositionInfo info)
{
	jbMushroomCore.setPlayheadInfo(info);
}

void JerobeamMushroomModule::processBlock(double **inOutBuffer, int numChannels, int numSamples)
{
	const bool doApplyModulations = modulationManager.getNumConnections() != 0;

	const bool doApplySmoothing = smoothingManager->needsSmoothing();

	for (int n = 0; n < numSamples; n++)
	{
	  double left = 0, right = 0;

		bool isSilent = jbMushroomCore.isSilent();

		if (doApplySmoothing)
			smoothingManager->updateSmoothedValues();

		if (!isSilent && doApplyModulations)
			modulationManager.applyModulations();

		MIDIMASTER.incrementPitchGlide();

		if (!isSilent)
			jbMushroomCore.processSampleFrame(&left, &right);

		equalizerModule->processStereoFrame(&left, &right);
		limiterModule->processSampleFrame(&left, &right);
		delayModule->processSampleFrame(&left, &right);
		oscilloscopeModule->processStereoFrame(&left, &right);

		if (inOutBuffer[0]) inOutBuffer[0][n] = left;
		if (inOutBuffer[1]) inOutBuffer[1][n] = right;
	}	
}
