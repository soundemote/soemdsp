#include "JerobeamSpiralModule.h"

#undef RELATIVE
#undef ABSOLUTE

JerobeamSpiralModule::JerobeamSpiralModule(CriticalSection *lockToUse, 
	jura::MetaParameterManager* metaManagerToUse)
	: BasicModule(lockToUse, metaManagerToUse)
{
  ScopedLock scopedLock(*lock);

	setModuleLink("https://www.soundemote.com/products/Spiral-generator");
	setModuleNameString("Spiral Generator", "SpiralGenerator");
	PluginFileManager::setModuleVersionString("1.4.0");
	setActiveDirectory(presetPath);
	AudioModule::patchFormatIndex = 2;

	BasicModule::setupParametersAndModules();
	setModulationManager(&modulationManager);
	modulationManager.setMetaParameterManager(metaParamManager);

	loadFile(File(initPresetPath));
}

void JerobeamSpiralModule::setupModulesAndModulation()
{
	MIDIMASTER.addMidiSlave(this, MidiMaster::AddFor::MonoNoteCallbacks);

	MIDIMASTER.addMidiSlave(&jbSpiralCore, MidiMaster::AddFor::NoteChange);
	MIDIMASTER.addMidiSlave(&jbSpiralCore, MidiMaster::AddFor::MonoNoteCallbacks);
	MIDIMASTER.addMidiSlave(&jbSpiralCore, MidiMaster::AddFor::MasterRateChange);
	MIDIMASTER.addMidiSlave(&jbSpiralCore, MidiMaster::AddFor::HostBPMChange);

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

	jbSpiralCore.bandpassModule = new BandpassModule(lock, metaParamManager, &modulationManager);
	jbSpiralCore.bandpassModule->setModuleName("Filter");
	addChildAudioModule(jbSpiralCore.bandpassModule);

	delayModule = new DelayModule(lock, metaParamManager, &modulationManager);
	delayModule->setModuleName("Delay");
	addChildAudioModule(delayModule);

	oscilloscopeModule = new OscilloscopeModule(lock, metaParamManager, &modulationManager);
	oscilloscopeModule->setModuleName("Oscilloscope");
	addChildAudioModule(oscilloscopeModule);
}

void JerobeamSpiralModule::createParameters()
{
	vector<String> EnvShapeMenuItems ={ "EXP", "EXP2", "LOG", "S-CRV" };

	/* Common Controls */
	parChannelMode.initCombobox("Channel Mode", 0, MonoSynth::ChannelModeMenuItems);
	parStereoRotate.initSlider("Stereo Rotate", -1, 1, 0);

	parClippingMode.initCombobox("Clip Mode", 1, JerobeamSpiralCore::ClippingModeMenuItems);
	parGain.initSlider("Gain", -60, 40, 0);
	parClippingLevel.initSlider("Clip Level", 1.e-3, 1.5, 1);

	parSmoothing.initSlider("Parameter Smoothing", 0.001, 1, .01);
	parOversampling.initSlider("Oversampling", 1, 8, 1);

	parDC.initSlider("DC Offset", -1, +1, 0);
	parX.initSlider("X Offset", -1, +1, 0);
	parY.initSlider("Y Offset", -1, +1, 0);

	parBP1Reset.initCombobox("BP1ResetMode", 2, { "Always", "On-note", "Legato" });
	parBP2Reset.initCombobox("BP2ResetMode", 2, { "Always", "On-note", "Legato" });

	parResetMode.initCombobox("Reset Mode", JerobeamSpiralCore::ResetMode::when_not_legato, JerobeamSpiralCore::ResetModeMenuItems);
	parNoteChangeMode.initCombobox("Note Change Mode", MidiMaster::NoteChangeMode::Always, MidiMaster::NoteChangeModeMenuItems);
	parResetWhenBeatChanges.initButton("Reset on tempo change", 0);
	parGlideMode.initCombobox("Glide Mode", 0, MidiMaster::GlideModeMenuItems);
	parFreqMode.initCombobox("Frequency Mode", 0, JerobeamSpiralCore::KeytrackingMenuItems);
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
	parFeedbackSrc.initCombobox("Feedback Source", 0, JerobeamSpiralCore::FeedbackSourceMenuItems);
	parFeedbackAmp.initSlider("Feedback Amount", -5, +5, 0);

	/* Amp Env Controls */
	parEnvResetMode.initCombobox("EnvResetMode", 2, JerobeamSpiralCore::EnvResetModeMenuItems);
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

	/* Spiral Controls */
	parCoreGain.initSlider("Spiral Gain", -1, 1, 1);
	parPhase.initSlider("Phase", -1, 1, 0);
	parDensity.initSlider("Density", 0.1, 1000, 1.0);
	parQuantizeDensity.initButton("QuantizeDensity", 0);
	parPosition.initSlider("Sub Phase", -1, +1, 0);
	parPositionSpeed.initSlider("Sub Phase Speed", -20, +20, 0);
	parSize.initSlider("Size", 0.1, 1, 0.5);
	parSharp.initSlider("Sharp", -1, 1, 0);
	parSharpCurve.initSlider("SharpCurve", 0, 5, 0);
	parSharpCrvMult.initSlider("CurveMultiply", 0, 1, 1.0);
	parMorph.initSlider("Morph", 0, 1, 0.5);
	parMorphSpeed.initSlider("Morph Speed", -20, +20, 0);
	parRotX.initSlider("RotX", -1, +1, 0);
	parRotXSpeed.initSlider("RotX Speed", -20, +20, 0);
	parRotY.initSlider("RotY", -1, +1, 0);
	parRotYSpeed.initSlider("RotY Speed", -20, +20, 0);
	parZDepth.initSlider("ZDepth", 0, 5, 0);
	parZDarkness.initSlider("ZDarkness", 0, 20, 0);
	parZDarknessLPF.initSlider("ZDarkLPF", .01, 24000, 24000);
	parZDarknessHPF.initSlider("ZDarkHPF", .01, 24000, .01);

	paramManager.instantiateParameters(this);
}

void JerobeamSpiralModule::setupCallbacks()
{
	parChannelMode.setCallback([this](double v) { jbSpiralCore.setChannelMode((int)v); });
	parClippingMode.setCallback([this](double v) { jbSpiralCore.setClipMode((int)v); });
	parClippingLevel.setCallback([this](double v) { jbSpiralCore.setClipLevel(v); });
	parStereoRotate.setCallback([this](double v) { jbSpiralCore.setRotation(v); });
	parGain.setCallback([this](double v) { jbSpiralCore.setOutputAmplitude(dbToAmp(v)); });

	parDC.setCallback([this](double v) { jbSpiralCore.setDCOffset(v); });
	parX.setCallback([this](double v) { jbSpiralCore.setXOffset(v); });
	parY.setCallback([this](double v) { jbSpiralCore.setYOffset(v); });

	parFreqMode.setCallback([this](double v) { jbSpiralCore.setKeytrackingMode((JerobeamSpiralCore::KeytrackingMode)(int)(v)); });
	parNoteChangeMode.setCallback([this](double v) { MIDIMASTER.setNoteChangeMode((int)v); });

	parResetWhenBeatChanges.setCallback([this](double v) { jbSpiralCore.resetWhenBeatTempoOrMultChanes = v >= 0.5; });

	parBeatFreq.setCallback([this](double v) { jbSpiralCore.setFrequencyOffset(v); });
	parBeatTempo.setCallback([this](double v) { jbSpiralCore.setTempoSelection((int)v); });
	parBeatMult.setCallback([this](double v) { jbSpiralCore.setTempoMultiplier(v); });

	parTune.setCallback([this](double v) { jbSpiralCore.setSemitoneOffset(v); });
	parOctave.setCallback([this](double v) { jbSpiralCore.setOctaveOffset(v); });

	parFreqMult.setCallback([this](double v)
	{
		MIDIMASTER.setMasterRate(v);

		jbSpiralCore.jbSpiral.setMorphSpeed(v * parMorphSpeed);
		jbSpiralCore.jbSpiral.setPositionSpeed(v * parPositionSpeed);
		jbSpiralCore.jbSpiral.setRotXSpeed(v * parRotXSpeed);
		jbSpiralCore.jbSpiral.setRotYSpeed(v * parRotYSpeed);
	});

	parHarmonicMult.setCallback([this](double v) { jbSpiralCore.setHarmonicMultiplier(v); });
	parHarmOctGlideAmt.setCallback([this](double v) { jbSpiralCore.setHarmAndOctGlideAmt(v); });

	parGlideMode.setCallback([this](double v) { MIDIMASTER.setGlideMode((int)v); });
	parGlideAmt.setCallback([this](double v) { MIDIMASTER.setGlideSeconds(v); });

	parOversampling.setCallback([this](double v) { jbSpiralCore.setOversampling((int)v); });

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

	parFeedbackHPF.setCallback([this](double v) { jbSpiralCore.setFeedbackHPCutoff(v); });
	parFeedbackLPF.setCallback([this](double v) { jbSpiralCore.setFeedbackLPCutoff(v); });

	parFeedbackSrc.setCallback([this](double v) { jbSpiralCore.setFeedbackSource((int)v); });
	parFeedbackAmp.setCallback([this](double v) { jbSpiralCore.setFeedbackAmp(v); });

	/* AMPLITUDE ENVELOPE */
	parEnvVelInfluence.setCallback([this](double v) { jbSpiralCore.ampEnv.setVelocityInfluence(v); });
	parEnvResetMode.setCallback([this](double v) { jbSpiralCore.setAmpEnvResetMode((JerobeamSpiralCore::EnvResetMode)(int)v); });
	parEnvGlobalTime.setCallback([this](double v) { jbSpiralCore.ampEnv.setGlobalTime(v); });
	parEnvAttackFB.setCallback([this](double v) { jbSpiralCore.ampEnv.setAttackFeedback(v); });
	parEnvDecayFB.setCallback([this](double v) { jbSpiralCore.ampEnv.setDecayFeedback(v); });
	parEnvReleaseFB.setCallback([this](double v) { jbSpiralCore.ampEnv.setReleaseFeedback(v); });
	parEnvSustain.setCallback([this](double v) { jbSpiralCore.ampEnv.setSustainAmplitude(v); });
	parEnvGlobalFB.setCallback([this](double v) { jbSpiralCore.ampEnv.setGlobalFeedback(v); });
	parEnvAttackShape.setCallback([this](double v) { jbSpiralCore.ampEnv.setAttackShape((FeedbackADSR::Shape)(int)v); });
	parEnvDecayShape.setCallback([this](double v) { jbSpiralCore.ampEnv.setDecayShape((FeedbackADSR::Shape)(int)v); });
	parEnvReleaseShape.setCallback([this](double v) { jbSpiralCore.ampEnv.setReleaseShape((FeedbackADSR::Shape)(int)v); });
	
	/* Spiral Callbacks */
	parResetMode.setCallback([this](double v) { jbSpiralCore.setResetMode((int)v); });
	parCoreGain.setCallback([this](double v) { jbSpiralCore.setInternalGain(v); });
	parPhase.setCallback([this](double v) { jbSpiralCore.jbSpiral.phasor.setPhaseOffset(v); });
	parPosition.setCallback([this](double v) { jbSpiralCore.jbSpiral.setPosition(v); });
	parPositionSpeed.setCallback([this](double v) { jbSpiralCore.jbSpiral.setPositionSpeed(v * MIDIMASTER.getMasterRate()); });
	parDensity.setCallback([this](double v) { jbSpiralCore.jbSpiral.setDensity(parQuantizeDensity ? floor(v) : v); });
	parQuantizeDensity.setCallback([this](double v) { jbSpiralCore.jbSpiral.setDensity(parQuantizeDensity ? floor(parDensity) : parDensity); });
	parSharp.setCallback([this](double v) { jbSpiralCore.jbSpiral.setSharp(bipolarToUnipolar(v)); });
	parSharpCurve.setCallback([this](double v) { jbSpiralCore.jbSpiral.setSharpCurve(v); });
	parSharpCrvMult.setCallback([this](double v) { jbSpiralCore.jbSpiral.setSharpCrvMult(v); });
	parMorph.setCallback([this](double v) { jbSpiralCore.jbSpiral.setMorph(v); });
	parMorphSpeed.setCallback([this](double v) { jbSpiralCore.jbSpiral.setMorphSpeed(v * MIDIMASTER.getMasterRate()); });
	parSize.setCallback([this](double v) { jbSpiralCore.jbSpiral.setSize(v); });
	parRotX.setCallback([this](double v) { jbSpiralCore.jbSpiral.setRotX(v); });
	parRotXSpeed.setCallback([this](double v) { jbSpiralCore.jbSpiral.setRotXSpeed(v * MIDIMASTER.getMasterRate()); });
	parRotY.setCallback([this](double v) { jbSpiralCore.jbSpiral.setRotY(v); });
	parRotYSpeed.setCallback([this](double v) { jbSpiralCore.jbSpiral.setRotYSpeed(v * MIDIMASTER.getMasterRate()); });
	parZDepth.setCallback([this](double v) { jbSpiralCore.jbSpiral.setZDepth(v); });
	parZDarkness.setCallback([this](double v) { jbSpiralCore.jbSpiral.setZAmount(v); });
	parZDarknessLPF.setCallback([this](double v) { jbSpiralCore.jbSpiral.bpfilter.setLowpassCutoff(v); });
	parZDarknessHPF.setCallback([this](double v) { jbSpiralCore.jbSpiral.bpfilter.setHighpassCutoff(v); });
}

void JerobeamSpiralModule::setInitPatchValues()
{
	parFeedbackAmp.setValue(1);

	for (auto & obj : BasicOscillatorModuleArray)
		obj->parFrequency.setValue(4);

	delayModule->parBypass.setValue(1);

	jbSpiralCore.ampEnv.setDelayTime(0);
	jbSpiralCore.ampEnv.setAttackTime(0);
	jbSpiralCore.ampEnv.setDecayTime(0);
	jbSpiralCore.ampEnv.setReleaseTime(0);

	parDensity.ptr->setDefaultModParameters(-1.e+100, 1.e+100, -8, +8, jura::ModulationConnection::modModes::EXPONENTIAL, 1);
}

void JerobeamSpiralModule::setHelpTextAndLabels()
{
	parFreqMult.text = "Master Rate";
	parResetMode.text = "Reset";
	parEnvResetMode.text = "Reset";
	parEnvGlobalTime.text = "<- Linear / Curved ->";

	parHarmonicMult.text = "Harmonic";

	parQuantizeDensity.text = "Qtz";
	parPositionSpeed.text = "^- Rotation Speed";
	parMorphSpeed.text = "^- Rotation Speed";
	parRotXSpeed.text = "^- Rotation Speed";
	parRotYSpeed.text = "^- Rotation Speed";

	parZDarkness.text = "Z-Darkness";
	parZDarknessHPF.text = "Z-Darkness HPF";
	parZDarknessLPF.text = "Z-Darkness LPF";

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

	/* Spiral Controls */
	parCoreGain.helpText =  "Gain of Spiral oscillator. Set to 0 to turn off. Useful for when you want to hear modulators through DC Offset modultion.";
	parPhase.helpText = "Offset of Spiral oscillator phase.";
	parPosition.helpText = "Position of sub spiral along the main axis. Quantize density to avoid clicks when modulating this parameter.";
	parSize.helpText = "Radius or width of the inner circles. Increases brightness of the sound.";
	parQuantizeDensity.helpText = "Rounds the density variable down to an integer. Integer density is needed for clean sub phase modulation.";
	parSharp.helpText = "Sharpness of triangle oscillator. Use to add harmonics. Triangle controls the movement of phase through the spiral oscillator.";
	parSharpCurve.helpText = "Sharpness curvature of triangle oscillator. Edges of spiral darken and sound is more aggressive. Generally, use 0 to 1 range.";
	parSharpCrvMult.helpText = "Only functions if sharp curve is not 0. Try setting SharpCurve to 1 and sweep CurveMultiply from 0 to 1.";
	parMorph.helpText = "Zooms infinitely in or out the fractal spiral. Modulate using rotation speed or a sawtooth with 1 amplitude.";
	parRotX.helpText = "Rotates the spiral on the X axis. This will affect pitch if ZAmount is more than 0.";
	parRotY.helpText = "Rotates the spiral on the Y axis. This will affect pitch if ZAmount is more than 0.";
	parZDepth.helpText = "Causes the spiral to fray in all directions. This will affect pitch if ZAmount is more than 0.";
	parZDarkness.helpText = "Creates an illusion of depth by darkening the back of the spiral with a feedback signal. Will cause keytracking issues. Oversampling helps.";
	parZDarknessHPF.helpText = "Applies highpass filtering to feedback signal used for Z-Darkness.";
	parZDarknessLPF.helpText = "Applies lowpass filtering to feedback signal used for Z-Darkness.";
}

void JerobeamSpiralModule::setSampleRate(double v)
{
	sampleRate = v;

	jbSpiralCore.setSampleRate(v);

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

void JerobeamSpiralModule::triggerAllNotesOff()
{
	/* if statements don't work, we need a feature "start from current time" */
	//if (parBP1Reset < (double)MonoSynth::EnvResetMode::env_never)
	Breakpoint1Module->noteOff(getMidiState().note);
	//if (parBP2Reset < (double)MonoSynth::EnvResetMode::env_never)
	Breakpoint2Module->noteOff(getMidiState().note);
}

void JerobeamSpiralModule::triggerAttack()
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

	Breakpoint1Module->noteOn(parBP1Reset == (double)JerobeamSpiralCore::EnvResetMode::env_never, getMidiState().note, int(getMidiState().velocity) * 127);
	Breakpoint2Module->noteOn(parBP2Reset == (double)JerobeamSpiralCore::EnvResetMode::env_never, getMidiState().note, int(getMidiState().velocity) * 127);
}

void JerobeamSpiralModule::triggerNoteOnLegato()
{
	if (parBP1Reset < (double)JerobeamSpiralCore::EnvResetMode::env_when_not_legato)
		Breakpoint1Module->noteOn(false, MIDIMASTER.getState().note, int(MIDIMASTER.getState().velocity) * 127);
	if (parBP2Reset < (double)JerobeamSpiralCore::EnvResetMode::env_when_not_legato)
		Breakpoint2Module->noteOn(false, MIDIMASTER.getState().note, int(MIDIMASTER.getState().velocity) * 127);
}

void JerobeamSpiralModule::triggerNoteOffLegato()
{
	if (parBP1Reset < (double)JerobeamSpiralCore::EnvResetMode::env_on_note)
		Breakpoint1Module->noteOn(false, MIDIMASTER.getState().note, int(MIDIMASTER.getState().velocity) * 127);
	if (parBP2Reset < (double)JerobeamSpiralCore::EnvResetMode::env_on_note)
		Breakpoint2Module->noteOn(false, MIDIMASTER.getState().note, int(MIDIMASTER.getState().velocity) * 127);
}

void JerobeamSpiralModule::triggerRelease()
{
	/* if statements don't work, we need a feature "start from current time" */
	//if (parBP1Reset < (double)MonoSynth::EnvResetMode::env_never)
	Breakpoint1Module->noteOff(MIDIMASTER.getState().note);
	//if (parBP2Reset < (double)MonoSynth::EnvResetMode::env_never)
	Breakpoint2Module->noteOff(MIDIMASTER.getState().note);
}

void JerobeamSpiralModule::setPlayheadInfo(AudioPlayHead::CurrentPositionInfo info)
{
	jbSpiralCore.setPlayheadInfo(info);
}

void JerobeamSpiralModule::setBeatsPerMinute(double v)
{
	MIDIMASTER.setBPM(v);

	Breakpoint1Module->setBeatsPerMinute(v*0.25);
	Breakpoint2Module->setBeatsPerMinute(v*0.25);

	delayModule->delay.setTempoInBPM(v);
}


void JerobeamSpiralModule::processBlock(double **inOutBuffer, int numChannels, int numSamples)
{
	const bool doApplyModulations = modulationManager.getNumConnections() != 0;

	const bool doApplySmoothing = smoothingManager->needsSmoothing();

	for (int n = 0; n < numSamples; n++)
	{
	  double left = 0, right = 0;

		bool isSilent = jbSpiralCore.isSilent();

		if (doApplySmoothing)
			smoothingManager->updateSmoothedValues();

		if (!isSilent && doApplyModulations)
			modulationManager.applyModulations();

		MIDIMASTER.incrementPitchGlide();

		if (!isSilent)
			jbSpiralCore.processSampleFrame(&left, &right);

		equalizerModule->processStereoFrame(&left, &right);
		limiterModule->processSampleFrame(&left, &right);
		delayModule->processSampleFrame(&left, &right);
		oscilloscopeModule->processStereoFrame(&left, &right);

		if (inOutBuffer[0]) inOutBuffer[0][n] = left;
		if (inOutBuffer[1]) inOutBuffer[1][n] = right;
	}	
}
