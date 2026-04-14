#include "JerobeamRadarModule.h"

JerobeamRadarModule::JerobeamRadarModule(CriticalSection *lockToUse, jura::MetaParameterManager* metaManagerToUse)
	: BasicModule(lockToUse, metaManagerToUse)
{
  ScopedLock scopedLock(*lock);

	setModulationManager(&modulationManager);
	modulationManager.setMetaParameterManager(metaParamManager);

	PluginFileManager::setModuleWebsite("https://www.soundemote.com/products/radar-generator");
	setModuleNameString("Radar Generator", "RadarGenerator");
	PluginFileManager::setModuleVersionString("1.4.0");
	setActiveDirectory(PluginFileManager::getFactoryPresetFolder());
	AudioModule::patchFormatIndex = 2;

	BasicModule::setupParametersAndModules();

	jassert(File(PluginFileManager::getInitPresetPath()).exists());
	loadFile(File(PluginFileManager::getInitPresetPath()));
}

void JerobeamRadarModule::setupModulesAndModulation()
{
	MIDIMASTER.addMidiSlave(this, MidiMaster::AddFor::MonoNoteCallbacks);

	MIDIMASTER.addMidiSlave(&jbRadarCore, MidiMaster::AddFor::NoteChange);
	MIDIMASTER.addMidiSlave(&jbRadarCore, MidiMaster::AddFor::MonoNoteCallbacks);
	MIDIMASTER.addMidiSlave(&jbRadarCore, MidiMaster::AddFor::MasterRateChange);
	MIDIMASTER.addMidiSlave(&jbRadarCore, MidiMaster::AddFor::HostBPMChange);

	modFeedback.setValuePtr(&jbRadarCore.feedbackValue);
	ampEnvOut.setValuePtr(&jbRadarCore.ampEnvVal);

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

	jbRadarCore.bandpassModule = new BandpassModule(lock, metaParamManager, &modulationManager);
	jbRadarCore.bandpassModule->setModuleName("Filter");
	addChildAudioModule(jbRadarCore.bandpassModule);

	delayModule = new DelayModule(lock, metaParamManager, &modulationManager);
	delayModule->setModuleName("Delay");
	addChildAudioModule(delayModule);

	oscilloscopeModule = new OscilloscopeModule(lock, metaParamManager, &modulationManager);
	oscilloscopeModule->setModuleName("Oscilloscope");
	addChildAudioModule(oscilloscopeModule);
}

void JerobeamRadarModule::createParameters()
{
	vector<String> EnvShapeMenuItems ={ "EXP", "EXP2", "LOG", "S-CRV" };

	/* Common Controls */
	parChannelMode.initCombobox("Channel Mode", 0, MonoSynth::ChannelModeMenuItems);
	parStereoRotate.initSlider("Stereo Rotate", -1, 1, 0);

	parClippingMode.initCombobox("Clip Mode", 1, JerobeamRadarCore::ClippingModeMenuItems);
	parGain.initSlider("Gain", -60, 40, 0);
	parClippingLevel.initSlider("Clip Level", 1.e-3, 1.5, 1);

	parSmoothing.initSlider("Parameter Smoothing", 0.001, 1, .01);
	parOversampling.initSlider("Oversampling", 1, 8, 1);

	parDC.initSlider("DC Offset", -1, +1, 0);
	parX.initSlider("X Offset", -1, +1, 0);
	parY.initSlider("Y Offset", -1, +1, 0);

	parBP1Reset.initCombobox("BP1ResetMode", 2, { "Always", "On-note", "Legato" });
	parBP2Reset.initCombobox("BP2ResetMode", 2, { "Always", "On-note", "Legato" });

	parResetMode.initCombobox("Reset Mode", JerobeamRadarCore::ResetMode::when_not_legato, JerobeamRadarCore::ResetModeMenuItems);
	parNoteChangeMode.initCombobox("Note Change Mode", MIDIMASTER.NoteChangeMode::Always, MIDIMASTER.NoteChangeModeMenuItems);
	parResetWhenBeatChanges.initButton("Reset on tempo change", 0);
	parGlideMode.initCombobox("Glide Mode", 0, MIDIMASTER.GlideModeMenuItems);
	parFreqMode.initCombobox("Frequency Mode", 0, JerobeamRadarCore::KeytrackingMenuItems);
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
	parFeedbackSrc.initCombobox("Feedback Source", 0, JerobeamRadarCore::FeedbackSourceMenuItems);
	parFeedbackAmp.initSlider("Feedback Amount", -5, +5, 0);

	/* Amp Env Controls */
	parEnvResetMode.initCombobox("EnvResetMode", 2, JerobeamRadarCore::EnvResetModeMenuItems);
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

	parCoreGain.initSlider("RadarGain", -1, 1, 1);
	parPhase.initSlider("Phase", -1, 1, 0);
	/* Radar Controls */
	parSubPhase.initSlider("SubPhase", -1, +1, 0);
	parSubPhaseRotation.initSlider("SubPhaseRotation", -20, +20, 0);
	parDensity.initSlider("Density", 0, 400, 1);
	parSharp.initSlider("Sharp", -1, 1, 0);
	parSharpCurve.initSlider("Sharp Curve", 0, 35, 1);	
	parDirection.initSlider("Direction", 0, 1, 1);
	parShade.initSlider("Shade", 0, 80, 1);
	parLap.initSlider("Lap", -1, 1, 0);
	parRingCut.initButton("RingCut", 0);
	parCurveStart.initButton("Curve Start", 0);
	parCurveEnd.initButton("Curve End", 0);
	parPow2Bend.initButton("Bend", 0);
	parPhaseInv.initButton("PhaseInv", 0);
	parTunnelInv.initButton("TunnelInv", 0);
	parSpiralReturn.initButton("SpiralReturn", 0);
	parLength.initSlider("Length", 0.0001, 1, 1);
	parRatio.initSlider("Ratio", 0, 1, 1);
	parFrontRing.initSlider("FrontRing", 0, 1, 0);
	parZDepth.initSlider("ZDepth", 0, 1, 0);
	parInner.initSlider("Inner", 0, 1, 0);
	parMorph.initSlider("Morph", 0, 1, 1);
	parProtrudeX.initSlider("Protrude X", -1, 1, 0);
	parProtrudeY.initSlider("Protrude Y", -1, 1, 0);

	paramManager.instantiateParameters(this);
}

void JerobeamRadarModule::setupCallbacks()
{
	parDensity.ptr->setMapper(new rsParameterMapperSinhUni(parDensity.mini, parDensity.maxi, 7));
	parSharpCurve.ptr->setMapper(new rsParameterMapperSinhUni(parSharpCurve.mini, parSharpCurve.maxi, 7));
	parShade.ptr->setMapper(new rsParameterMapperSinhUni(parShade.mini, parShade.maxi, 8));
	parSharp.ptr->setMapper(new jura::rsParameterMapperTanh(parSharp.mini, parSharp.maxi, 2));

	parChannelMode.setCallback([this](double v) { jbRadarCore.setChannelMode((int)v); });
	parClippingMode.setCallback([this](double v) { jbRadarCore.setClipMode((int)v); });
	parClippingLevel.setCallback([this](double v) { jbRadarCore.setClipLevel(v); });
	parStereoRotate.setCallback([this](double v) { jbRadarCore.setRotation(v); });
	parGain.setCallback([this](double v) { jbRadarCore.setOutputAmplitude(RAPT::rsDbToAmp(v)); });

	parDC.setCallback([this](double v) { jbRadarCore.setDCOffset(v); });
	parX.setCallback([this](double v) { jbRadarCore.setXOffset(v); });
	parY.setCallback([this](double v) { jbRadarCore.setYOffset(v); });

	parFreqMode.setCallback([this](double v) { jbRadarCore.setKeytrackingMode((JerobeamRadarCore::KeytrackingMode)(int)(v)); });
	parNoteChangeMode.setCallback([this](double v) { MIDIMASTER.setNoteChangeMode((int)v); });

	parResetWhenBeatChanges.setCallback([this](double v) { jbRadarCore.resetWhenBeatTempoOrMultChanes = v >= 0.5; });

	parBeatFreq.setCallback([this](double v) { jbRadarCore.setFrequencyOffset(v); });
	parBeatTempo.setCallback([this](double v) { jbRadarCore.setTempoSelection((int)v); });
	parBeatMult.setCallback([this](double v) { jbRadarCore.setTempoMultiplier(v); });

	parTune.setCallback([this](double v) { jbRadarCore.setSemitoneOffset(v); });
	parOctave.setCallback([this](double v) { jbRadarCore.setOctaveOffset(v); });

	parFreqMult.setCallback([this](double v)
	{
		MIDIMASTER.setMasterRate(v);

		jbRadarCore.jbRadar.rotator.setFrequency(parSubPhaseRotation * v);
	});

	parHarmonicMult.setCallback([this](double v) { jbRadarCore.setHarmonicMultiplier(v); });
	parHarmOctGlideAmt.setCallback([this](double v) { jbRadarCore.setHarmAndOctGlideAmt(v); });

	parGlideMode.setCallback([this](double v) { MIDIMASTER.setGlideMode((int)v); });
	parGlideAmt.setCallback([this](double v) { MIDIMASTER.setGlideSeconds(v); });

	parOversampling.setCallback([this](double v) { jbRadarCore.setOversampling((int)v); });

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

	parFeedbackHPF.setCallback([this](double v) { jbRadarCore.setFeedbackHPCutoff(v); });
	parFeedbackLPF.setCallback([this](double v) { jbRadarCore.setFeedbackLPCutoff(v); });

	parFeedbackSrc.setCallback([this](double v) { jbRadarCore.setFeedbackSource((int)v); });
	parFeedbackAmp.setCallback([this](double v) { jbRadarCore.setFeedbackAmp(v); });

	/* AMPLITUDE ENVELOPE */
	parEnvVelInfluence.setCallback([this](double v) { jbRadarCore.ampEnv.setVelocityInfluence(v); });
	parEnvResetMode.setCallback([this](double v) { jbRadarCore.setAmpEnvResetMode((JerobeamRadarCore::EnvResetMode)(int)v); });
	parEnvGlobalTime.setCallback([this](double v) { jbRadarCore.ampEnv.setGlobalTime(v); });
	parEnvAttackFB.setCallback([this](double v) { jbRadarCore.ampEnv.setAttackFeedback(v); });
	parEnvDecayFB.setCallback([this](double v) { jbRadarCore.ampEnv.setDecayFeedback(v); });
	parEnvReleaseFB.setCallback([this](double v) { jbRadarCore.ampEnv.setReleaseFeedback(v); });
	parEnvSustain.setCallback([this](double v) { jbRadarCore.ampEnv.setSustainAmplitude(v); });
	parEnvGlobalFB.setCallback([this](double v) { jbRadarCore.ampEnv.setGlobalFeedback(v); });
	parEnvAttackShape.setCallback([this](double v) { jbRadarCore.ampEnv.setAttackShape((FeedbackADSR::Shape)(int)v); });
	parEnvDecayShape.setCallback([this](double v) { jbRadarCore.ampEnv.setDecayShape((FeedbackADSR::Shape)(int)v); });
	parEnvReleaseShape.setCallback([this](double v) { jbRadarCore.ampEnv.setReleaseShape((FeedbackADSR::Shape)(int)v); });

	parResetMode.setCallback([this](double v) { jbRadarCore.setResetMode((int)v); });
	parPhase.setCallback([this](double v) { jbRadarCore.jbRadar.phasor.setPhaseOffset(v); });
	parSubPhase.setCallback([this](double v) { jbRadarCore.jbRadar.setRotation(v); });
	parCoreGain.setCallback([this](double v) { jbRadarCore.setInternalGain(v); });
	/* Radar Callbacks*/
	parSubPhaseRotation.setCallback([this](double v) { jbRadarCore.jbRadar.rotator.setFrequency(v * MIDIMASTER.getMasterRate()); });
	parDensity.setCallback([this](double v) { jbRadarCore.jbRadar.setDensity(v); });
	parSharp.setCallback([this](double v) { jbRadarCore.jbRadar.setSharp(v); });
	parSharpCurve.setCallback([this](double v) { jbRadarCore.jbRadar.setFade(v); });
	parDirection.setCallback([this](double v) { jbRadarCore.jbRadar.setDirection(v); });
	parShade.setCallback([this](double v) { jbRadarCore.jbRadar.setShade(v); });
	parLap.setCallback([this](double v) { jbRadarCore.jbRadar.setLap(v); });
	parRingCut.setCallback([this](double v) { jbRadarCore.jbRadar.setRingCut(v >= 0.5); });
	parCurveStart.setCallback([this](double v) { jbRadarCore.jbRadar.setPow1Up(v >= 0.5); });
	parCurveEnd.setCallback([this](double v) { jbRadarCore.jbRadar.setPow1Down(v >= 0.5); });
	parPow2Bend.setCallback([this](double v) { jbRadarCore.jbRadar.setPow2Bend(v >= 0.5); });
	parPhaseInv.setCallback([this](double v) { jbRadarCore.jbRadar.setPhaseInv(v >= 0.5); });
	parTunnelInv.setCallback([this](double v) { jbRadarCore.jbRadar.setTunnelInv(v >= 0.5); });
	parSpiralReturn.setCallback([this](double v) { jbRadarCore.jbRadar.setSpiralReturn(v >= 0.5); });
	parLength.setCallback([this](double v) { jbRadarCore.jbRadar.setLength(v); });
	parRatio.setCallback([this](double v) { jbRadarCore.jbRadar.setRatio(v); });
	parFrontRing.setCallback([this](double v) { jbRadarCore.jbRadar.setFrontRing(v); });
	parMorph.setCallback([this](double v) { jbRadarCore.jbRadar.setZoom(v); });
	parZDepth.setCallback([this](double v) { jbRadarCore.jbRadar.setZDepth(v); });
	parInner.setCallback([this](double v) { jbRadarCore.jbRadar.setInner(v); });
	parProtrudeX.setCallback([this](double v) { jbRadarCore.jbRadar.setX(v); });
	parProtrudeY.setCallback([this](double v) { jbRadarCore.jbRadar.setY(v); });
}

void JerobeamRadarModule::setInitPatchValues()
{
	parFeedbackAmp.setValue(1);

	for (auto & obj : BasicOscillatorModuleArray)
		obj->parFrequency.setValue(4);

	delayModule->parBypass.setValue(1);

	jbRadarCore.ampEnv.setDelayTime(0);
	jbRadarCore.ampEnv.setAttackTime(0);
	jbRadarCore.ampEnv.setDecayTime(0);
	jbRadarCore.ampEnv.setReleaseTime(0);

	parDensity.ptr->setDefaultModParameters(-1.e+100, 1.e+100, -8, +8, jura::ModulationConnection::modModes::EXPONENTIAL, 1);
}

void JerobeamRadarModule::setHelpTextAndLabels()
{
	parCurveStart.text = "/";
	parCurveEnd.text = "\\";

	parSubPhaseRotation.text = "^- Rotation Speed";

	parMorph.text = "<- Cylinder / Flat ->";
	parLength.text = "Inner Length";
	parInner.text = "Inner ZDepth";
	parFrontRing.text = "Outer Length";
	parZDepth.text = "Outer ZDepth";

	parFreqMult.text = "Master Rate";
	parResetMode.text = "Reset";
	parEnvResetMode.text = "Reset";
	parEnvGlobalTime.text = "<- Linear / Curved ->";

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

	/* Common Controls */
	parCoreGain.helpText = "Gain of radar oscillator. Set to 0 to turn off. Useful for when you want to hear modulators through DC Offset modultion.";
	parPhase.helpText = "Offset of radar oscillator phase.";

	/* Radar Controls */
	parDensity.helpText = "Length of the spiral.";
	parSharp.helpText = "Sharpness of internal triangle oscillator. Use to add harmonics. Triangle controls the movement of phase through the spiral.";
	parSharpCurve.helpText = "Controls the sharpness curve of triangle oscillator. Note: Curve buttons '/' '\' must be enabled.";
	parSubPhase.helpText = "Position of sub spiral along the main axis.";
	parDirection.helpText = "Affects how spiral arms connect ranging from smooth, sharp, and discontinuous. Enable Pow2Bend to prevent discontinuity.";
	parShade.helpText = "Causes the start of the spiral to be accentuated and an exaggerated radar visual. Makes the sound nasaly.";
	parLap.helpText = "Number of spiral rotations. Creates \"scanning visuals\" when used with Rotation. Non-integer causes discontinuty.";
	parLength.helpText = "Lenght of inner spiral";
	parRatio.helpText = "Flat: increases the protrusion length of X/Y protrusion. Cylinder: controls cylinder width.";
	parFrontRing.helpText = "Lenght of outer spiral";
	parMorph.helpText = "Morphs between two modes. Help text will refer to 'Flat' and 'Cylinder' to describe the effect of parameters in both modes.";
	parZDepth.helpText = "Flat: Sqeezes the otuer spiral arms toward the center. Cylinder: Creates illusion of depth by skewing size and speed of outer spiral.";
	parInner.helpText = "Flat: Changes the end point of the inner spiral. Value of 1 will create a circle. Cylinder: Sharpens the spiral protrusion";
	parProtrudeX.helpText = "Flat: changes the X angle of protrusion. Cylinder: rotates or folds cylinder onto itself on the X axis like a slinky.";
	parProtrudeY.helpText = "Flat: changes the Y angle of protrusion. Cylinder: rotates or folds cylinder into itself on the Y axis like a slinky.";

	parRingCut.helpText = "Enables discreet circles to be drawn instead of spirals. This causes discontinuity.";
	parCurveStart.helpText = "Enables sharpness curve for the first half of triangle phase.";
	parCurveEnd.helpText = "Enables sharpness curve for the second half of triangle phase.";
	parPow2Bend.helpText = "Bends spiral arms to give the appearance of a scan line.";
	parPhaseInv.helpText = "Invert overall phase of radar oscillator";
	parTunnelInv.helpText = "Flips the radar oscillator in a \"mirror image\" kind of wa.y";
	parSpiralReturn.helpText = "Enagles radar to jumps to the next spiral arm instead of retracing. This causes discontinuity";
}

void JerobeamRadarModule::setSampleRate(double v)
{
	sampleRate = v;

	jbRadarCore.setSampleRate(v);

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

void JerobeamRadarModule::triggerAllNotesOff()
{
	/* if statements don't work, we need a feature "start from current time" */
	//if (parBP1Reset < (double)MonoSynth::EnvResetMode::env_never)
	Breakpoint1Module->noteOff(getMidiState().note);
	//if (parBP2Reset < (double)MonoSynth::EnvResetMode::env_never)
	Breakpoint2Module->noteOff(getMidiState().note);
}

void JerobeamRadarModule::triggerAttack()
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

	Breakpoint1Module->noteOn(parBP1Reset == (double)JerobeamRadarCore::EnvResetMode::env_never, getMidiState().note, int(getMidiState().velocity) * 127);
	Breakpoint2Module->noteOn(parBP2Reset == (double)JerobeamRadarCore::EnvResetMode::env_never, getMidiState().note, int(getMidiState().velocity) * 127);
}

void JerobeamRadarModule::triggerNoteOnLegato()
{
	if (parBP1Reset < (double)JerobeamRadarCore::EnvResetMode::env_when_not_legato)
		Breakpoint1Module->noteOn(false, MIDIMASTER.getState().note, int(MIDIMASTER.getState().velocity) * 127);
	if (parBP2Reset < (double)JerobeamRadarCore::EnvResetMode::env_when_not_legato)
		Breakpoint2Module->noteOn(false, MIDIMASTER.getState().note, int(MIDIMASTER.getState().velocity) * 127);
}

void JerobeamRadarModule::triggerNoteOffLegato()
{
	if (parBP1Reset < (double)JerobeamRadarCore::EnvResetMode::env_on_note)
		Breakpoint1Module->noteOn(false, MIDIMASTER.getState().note, int(MIDIMASTER.getState().velocity) * 127);
	if (parBP2Reset < (double)JerobeamRadarCore::EnvResetMode::env_on_note)
		Breakpoint2Module->noteOn(false, MIDIMASTER.getState().note, int(MIDIMASTER.getState().velocity) * 127);
}

void JerobeamRadarModule::triggerRelease()
{
	/* if statements don't work, we need a feature "start from current time" */
	//if (parBP1Reset < (double)MonoSynth::EnvResetMode::env_never)
	Breakpoint1Module->noteOff(MIDIMASTER.getState().note);
	//if (parBP2Reset < (double)MonoSynth::EnvResetMode::env_never)
	Breakpoint2Module->noteOff(MIDIMASTER.getState().note);
}

void JerobeamRadarModule::setBeatsPerMinute(double v)
{
	MIDIMASTER.setBPM(v);

	Breakpoint1Module->setBeatsPerMinute(v*0.25);
	Breakpoint2Module->setBeatsPerMinute(v*0.25);

	delayModule->delay.setTempoInBPM(v);
}

void JerobeamRadarModule::setPlayheadInfo(AudioPlayHead::CurrentPositionInfo info)
{
	jbRadarCore.setPlayheadInfo(info);
}

void JerobeamRadarModule::processBlock(double **inOutBuffer, int numChannels, int numSamples)
{
	//double barPhase = getPhaseBasedOnBarsPerCycle(1);

	const bool doApplyModulations = modulationManager.getNumConnections() != 0;

	const bool doApplySmoothing = smoothingManager->needsSmoothing();

	for (int n = 0; n < numSamples; n++)
	{
	  double left = 0, right = 0;

		bool isSilent = jbRadarCore.isSilent();

		if (doApplySmoothing)
			smoothingManager->updateSmoothedValues();

		if (!isSilent && doApplyModulations)
			modulationManager.applyModulations();

		MIDIMASTER.incrementPitchGlide();

		if (!isSilent)
			jbRadarCore.processSampleFrame(&left, &right);

		//LinL.setTargetValue(left);
		//LinR.setTargetValue(right);

		equalizerModule->processStereoFrame(&left, &right);
		limiterModule->processSampleFrame(&left, &right);
		delayModule->processSampleFrame(&left, &right);
		oscilloscopeModule->processStereoFrame(&left, &right);

		if (inOutBuffer[0]) inOutBuffer[0][n] = left/*LinL.getSample()*/;
		if (inOutBuffer[1]) inOutBuffer[1][n] = right/*LinR.getSample()*/;
	}	
}
