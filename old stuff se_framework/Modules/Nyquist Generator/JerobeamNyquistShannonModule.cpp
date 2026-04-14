#include "JerobeamNyquistShannonModule.h"

#undef RELATIVE
#undef ABSOLUTE

JerobeamNyquistShannonModule::JerobeamNyquistShannonModule(CriticalSection *lockToUse, 
	jura::MetaParameterManager* metaManagerToUse)
	: BasicModule(lockToUse, metaManagerToUse)
{
  ScopedLock scopedLock(*lock);

	setModulationManager(&modulationManager);
	modulationManager.setMetaParameterManager(metaParamManager);

	setModuleLink("https://www.soundemote.com/products/NyquistShannon-generator");
	setModuleNameString("Nyquist Generator", "NyquistGenerator");
	PluginFileManager::setModuleVersionString("1.4.0");
	setActiveDirectory(presetPath);
	AudioModule::patchFormatIndex = 2;

	BasicModule::setupParametersAndModules();

	loadFile(File(initPresetPath));
}

void JerobeamNyquistShannonModule::setupModulesAndModulation()
{
	MIDIMASTER.addMidiSlave(this, MidiMaster::AddFor::MonoNoteCallbacks);

	MIDIMASTER.addMidiSlave(&jbNyquistCore, MidiMaster::AddFor::NoteChange);
	MIDIMASTER.addMidiSlave(&jbNyquistCore, MidiMaster::AddFor::MonoNoteCallbacks);
	MIDIMASTER.addMidiSlave(&jbNyquistCore, MidiMaster::AddFor::MasterRateChange);
	MIDIMASTER.addMidiSlave(&jbNyquistCore, MidiMaster::AddFor::HostBPMChange);

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

	jbNyquistCore.bandpassModule = new BandpassModule(lock, metaParamManager, &modulationManager);
	jbNyquistCore.bandpassModule->setModuleName("Filter");
	addChildAudioModule(jbNyquistCore.bandpassModule);

	delayModule = new DelayModule(lock, metaParamManager, &modulationManager);
	delayModule->setModuleName("Delay");
	addChildAudioModule(delayModule);

	oscilloscopeModule = new OscilloscopeModule(lock, metaParamManager, &modulationManager);
	oscilloscopeModule->setModuleName("Oscilloscope");
	addChildAudioModule(oscilloscopeModule);
}

void JerobeamNyquistShannonModule::createParameters()
{
	vector<String> EnvShapeMenuItems ={ "EXP", "EXP2", "LOG", "S-CRV" };

	/* Common Controls */
	parChannelMode.initCombobox("Channel Mode", 0, MonoSynth::ChannelModeMenuItems);
	parStereoRotate.initSlider("Stereo Rotate", -1, 1, 0);

	parClippingMode.initCombobox("Clip Mode", 1, JerobeamNyquistShannonCore::ClippingModeMenuItems);
	parGain.initSlider("Gain", -60, 40, -6);
	parClippingLevel.initSlider("Clip Level", 1.e-3, 1.5, 1);

	parSmoothing.initSlider("Parameter Smoothing", 0.001, 1, .01);
	parOversampling.initSlider("Oversampling", 1, 8, 1);

	parDC.initSlider("DC Offset", -1, +1, 0);
	parX.initSlider("X Offset", -1, +1, 0);
	parY.initSlider("Y Offset", -1, +1, 0);

	parBP1Reset.initCombobox("BP1ResetMode", 2, { "Always", "On-note", "Legato" });
	parBP2Reset.initCombobox("BP2ResetMode", 2, { "Always", "On-note", "Legato" });

	parResetMode.initCombobox("Reset Mode", JerobeamNyquistShannonCore::ResetMode::when_not_legato, JerobeamNyquistShannonCore::ResetModeMenuItems);
	parNoteChangeMode.initCombobox("Note Change Mode", MidiMaster::NoteChangeMode::Always, MidiMaster::NoteChangeModeMenuItems);
	parResetWhenBeatChanges.initButton("Reset on tempo change", 0);
	parGlideMode.initCombobox("Glide Mode", 0, MidiMaster::GlideModeMenuItems);
	parFreqMode.initCombobox("Frequency Mode", 0, JerobeamNyquistShannonCore::KeytrackingMenuItems);
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
	parFeedbackSrc.initCombobox("Feedback Source", 0, JerobeamNyquistShannonCore::FeedbackSourceMenuItems);
	parFeedbackAmp.initSlider("Feedback Amount", -5, +5, 0);

	/* Amp Env Controls */
	parEnvResetMode.initCombobox("EnvResetMode", 2, JerobeamNyquistShannonCore::EnvResetModeMenuItems);
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

	parCoreGain.initSlider("Nyquist Gain", -1, 1, 1);
	parPhase.initSlider("Phase", -1, 1, 0);
	/* NyquistShannon Controls */
	parPitch.initSlider("Pitch", 0, 20, 1);
	parSubPhase.initSlider("SubPhase", -1, 1, 0);
	parSubPhaseRotation.initSlider("SubPhaseRotation", -20, +20, 0);
	parTone.initSlider("Tone", -100, +100, 0);
	parToneSmoother.initSlider("Tone Smoothing", 1.e-6, 5, 1.e-6);
	parToneModNote.initButton("ToneModNote", 0);
	parToneModPitch.initButton("ToneModPitch", 0);
	parToneModFreq.initButton("ToneModFreq", 0);
	parArtifact.initSlider("Artifact", -1, 1, 0);
	parNumDots.initSlider("NumDots", 1, 300, 1);
	parDotQuantize.initSlider("DotQuantize", 0, 1, .5);

	parPitch.stringConvertFunc = elan::StringFunc3WithX;

	paramManager.instantiateParameters(this);
}

void JerobeamNyquistShannonModule::setupCallbacks()
{
	parChannelMode.setCallback([this](double v) { jbNyquistCore.setChannelMode((int)v); });
	parClippingMode.setCallback([this](double v) { jbNyquistCore.setClipMode((int)v); });
	parClippingLevel.setCallback([this](double v) { jbNyquistCore.setClipLevel(v); });
	parStereoRotate.setCallback([this](double v) { jbNyquistCore.setRotation(v); });
	parGain.setCallback([this](double v) { jbNyquistCore.setOutputAmplitude(dbToAmp(v)); });

	parDC.setCallback([this](double v) { jbNyquistCore.setDCOffset(v); });
	parX.setCallback([this](double v) { jbNyquistCore.setXOffset(v); });
	parY.setCallback([this](double v) { jbNyquistCore.setYOffset(v); });

	parFreqMode.setCallback([this](double v) { jbNyquistCore.setKeytrackingMode((JerobeamNyquistShannonCore::KeytrackingMode)(int)(v)); });
	parNoteChangeMode.setCallback([this](double v) { MIDIMASTER.setNoteChangeMode((int)v); });

	parResetWhenBeatChanges.setCallback([this](double v) { jbNyquistCore.resetWhenBeatTempoOrMultChanes = v >= 0.5; });

	parBeatFreq.setCallback([this](double v) { jbNyquistCore.setFrequencyOffset(v); });
	parBeatTempo.setCallback([this](double v) { jbNyquistCore.setTempoSelection((int)v); });
	parBeatMult.setCallback([this](double v) { jbNyquistCore.setTempoMultiplier(v); });

	parTune.setCallback([this](double v) { jbNyquistCore.setSemitoneOffset(v); });
	parOctave.setCallback([this](double v) { jbNyquistCore.setOctaveOffset(v); });

	parFreqMult.setCallback([this](double v)
	{
		MIDIMASTER.setMasterRate(v);

		jbNyquistCore.jbNyquistShannon.phaseRotator.setFrequency(parSubPhaseRotation * v);
	});

	parHarmonicMult.setCallback([this](double v) { jbNyquistCore.setHarmonicMultiplier(v); });
	parHarmOctGlideAmt.setCallback([this](double v) { jbNyquistCore.setHarmAndOctGlideAmt(v); });

	parGlideMode.setCallback([this](double v) { MIDIMASTER.setGlideMode((int)v); });
	parGlideAmt.setCallback([this](double v) { MIDIMASTER.setGlideSeconds(v); });

	parOversampling.setCallback([this](double v) { jbNyquistCore.setOversampling((int)v); });

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

	parFeedbackHPF.setCallback([this](double v) { jbNyquistCore.setFeedbackHPCutoff(v); });
	parFeedbackLPF.setCallback([this](double v) { jbNyquistCore.setFeedbackLPCutoff(v); });

	parFeedbackSrc.setCallback([this](double v) { jbNyquistCore.setFeedbackSource((int)v); });
	parFeedbackAmp.setCallback([this](double v) { jbNyquistCore.setFeedbackAmp(v); });

	/* AMPLITUDE ENVELOPE */
	parEnvVelInfluence.setCallback([this](double v) { jbNyquistCore.ampEnv.setVelocityInfluence(v); });
	parEnvResetMode.setCallback([this](double v) { jbNyquistCore.setAmpEnvResetMode((JerobeamNyquistShannonCore::EnvResetMode)(int)v); });
	parEnvGlobalTime.setCallback([this](double v) { jbNyquistCore.ampEnv.setGlobalTime(v); });
	parEnvAttackFB.setCallback([this](double v) { jbNyquistCore.ampEnv.setAttackFeedback(v); });
	parEnvDecayFB.setCallback([this](double v) { jbNyquistCore.ampEnv.setDecayFeedback(v); });
	parEnvReleaseFB.setCallback([this](double v) { jbNyquistCore.ampEnv.setReleaseFeedback(v); });
	parEnvSustain.setCallback([this](double v) { jbNyquistCore.ampEnv.setSustainAmplitude(v); });
	parEnvGlobalFB.setCallback([this](double v) { jbNyquistCore.ampEnv.setGlobalFeedback(v); });
	parEnvAttackShape.setCallback([this](double v) { jbNyquistCore.ampEnv.setAttackShape((FeedbackADSR::Shape)(int)v); });
	parEnvDecayShape.setCallback([this](double v) { jbNyquistCore.ampEnv.setDecayShape((FeedbackADSR::Shape)(int)v); });
	parEnvReleaseShape.setCallback([this](double v) { jbNyquistCore.ampEnv.setReleaseShape((FeedbackADSR::Shape)(int)v); });

	parResetMode.setCallback([this](double v) { jbNyquistCore.setResetMode((int)v); });
	parPhase.setCallback([this](double v) { jbNyquistCore.jbNyquistShannon.phasor.setPhaseOffset(v); });
	parSubPhase.setCallback([this](double v) { jbNyquistCore.jbNyquistShannon.setSubPhase(v); });
	parSubPhaseRotation.setCallback([this](double v) { jbNyquistCore.jbNyquistShannon.setSubPhaseRotationSpeed(v * MIDIMASTER.getMasterRate()); });

	parCoreGain.setCallback([this](double v) { jbNyquistCore.setInternalGain(v); });
	/* NyquistShannon Callbacks*/
	parResetMode.setCallback([this](double v) { jbNyquistCore.setResetMode((int)v); });
	parPhase.setCallback([this](double v) { jbNyquistCore.jbNyquistShannon.setPhase(v); });
	parPitch.setCallback([this](double v) { jbNyquistCore.jbNyquistShannon.setFrequencyB(v); });
	parTone.setCallback([this](double v) { jbNyquistCore.jbNyquistShannon.setTone(v); });
	parToneSmoother.setCallback([this](double v) { jbNyquistCore.jbNyquistShannon.setToneSmoother(v); });
	parOversampling.setCallback([this](double v) { jbNyquistCore.setOversampling((int)v); });

	parNumDots.setCallback([this](double v) { jbNyquistCore.jbNyquistShannon.setRate(v); });
	parDotQuantize.setCallback([this](double v) { jbNyquistCore.jbNyquistShannon.setSampleDots(v); });
	parToneModNote.setCallback([this](double v) { jbNyquistCore.jbNyquistShannon.setEnableToneModNote(v > 0.5); });
	parToneModPitch.setCallback([this](double v) { jbNyquistCore.jbNyquistShannon.setEnableToneModPitch(v > 0.5); });
	parToneModFreq.setCallback([this](double v) { jbNyquistCore.jbNyquistShannon.setEnableToneModFreq(v > 0.5); });
	parArtifact.setCallback([this](double v) { jbNyquistCore.jbNyquistShannon.setArtifact(v/2+.5); });
}

void JerobeamNyquistShannonModule::setInitPatchValues()
{
	parFeedbackAmp.setValue(1);

	for (auto & obj : BasicOscillatorModuleArray)
		obj->parFrequency.setValue(4);

	delayModule->parBypass.setValue(1);

	jbNyquistCore.ampEnv.setDelayTime(0);
	jbNyquistCore.ampEnv.setAttackTime(0);
	jbNyquistCore.ampEnv.setDecayTime(0);
	jbNyquistCore.ampEnv.setReleaseTime(0);

	parDotQuantize.ptr->setDefaultModParameters(0, 1, 0, 1, jura::ModulationConnection::ABSOLUTE, 0.5);
}

void JerobeamNyquistShannonModule::setHelpTextAndLabels()
{
	parFreqMult.text = "Master Rate";
	parResetMode.text = "Reset";
	parEnvResetMode.text = "Reset";
	parCoreGain.text = "Nyquist Gain";
	parEnvGlobalTime.text = "<- Linear / Curved ->";

	parHarmonicMult.text = "Harmonic";
	parSubPhase.text = "Sub Phase";
	parSubPhaseRotation.text = "^- Rotation Speed";
	parToneSmoother.text = "^- Smoothing";

	parNumDots.text = "# Of Dots";
	parDotQuantize.text = "Dot Quantize";
	parArtifact.text = "Artifact (Sharp)";

	parToneModNote.text = "KeyTr";
	parToneModPitch.text = "Pitch";
	parToneModFreq.text = "Freq";

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

	/* Nyquist Controls */
	parCoreGain.helpText =  "Gain of nyquist oscillator. Set to 0 to turn off. Useful for when you want to hear modulators through DC Offset modultion.";
	parPhase.helpText = "Offset of nyquist phase. Use 0.5 to start oscillations at 0 to avoid clicks.";
	parSubPhase.helpText = "Offset of sub phase / inner sinusoid phase.";
	parArtifact.helpText = "Sharpness of triangle oscillator. Triangle controls the movement of phase. It is used for TriPhasor feedback mode.";

	parPitch.helpText = "Sub oscillation frequency is pitch x master frequency. ";
	parTone.helpText = "Tone or density of sub oscillations.";
	parToneSmoother.helpText = "Slows down tone changes from the Tone Mod section";

	parNumDots.helpText = "Number of quantization steps.";
	parDotQuantize.helpText = "Amount of discontinuity between steps.";

	parToneModNote.helpText = "Enables modulation of tone via keytracking centered on C4.";
	parToneModPitch.helpText = "Enables modulation tone via pitch parameter.";
	parToneModFreq.helpText = "Enables modulation of tone via master frequency.";
}

void JerobeamNyquistShannonModule::setSampleRate(double v)
{
	sampleRate = v;

	jbNyquistCore.setSampleRate(v);

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

void JerobeamNyquistShannonModule::triggerAllNotesOff()
{
	/* if statements don't work, we need a feature "start from current time" */
	//if (parBP1Reset < (double)MonoSynth::EnvResetMode::env_never)
	Breakpoint1Module->noteOff(getMidiState().note);
	//if (parBP2Reset < (double)MonoSynth::EnvResetMode::env_never)
	Breakpoint2Module->noteOff(getMidiState().note);
}

void JerobeamNyquistShannonModule::triggerAttack()
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

	Breakpoint1Module->noteOn(parBP1Reset == (double)JerobeamNyquistShannonCore::EnvResetMode::env_never, getMidiState().note, int(getMidiState().velocity) * 127);
	Breakpoint2Module->noteOn(parBP2Reset == (double)JerobeamNyquistShannonCore::EnvResetMode::env_never, getMidiState().note, int(getMidiState().velocity) * 127);
}

void JerobeamNyquistShannonModule::triggerNoteOnLegato()
{
	if (parBP1Reset < (double)JerobeamNyquistShannonCore::EnvResetMode::env_when_not_legato)
		Breakpoint1Module->noteOn(false, MIDIMASTER.getState().note, int(MIDIMASTER.getState().velocity) * 127);
	if (parBP2Reset < (double)JerobeamNyquistShannonCore::EnvResetMode::env_when_not_legato)
		Breakpoint2Module->noteOn(false, MIDIMASTER.getState().note, int(MIDIMASTER.getState().velocity) * 127);
}

void JerobeamNyquistShannonModule::triggerNoteOffLegato()
{
	if (parBP1Reset < (double)JerobeamNyquistShannonCore::EnvResetMode::env_on_note)
		Breakpoint1Module->noteOn(false, MIDIMASTER.getState().note, int(MIDIMASTER.getState().velocity) * 127);
	if (parBP2Reset < (double)JerobeamNyquistShannonCore::EnvResetMode::env_on_note)
		Breakpoint2Module->noteOn(false, MIDIMASTER.getState().note, int(MIDIMASTER.getState().velocity) * 127);
}

void JerobeamNyquistShannonModule::triggerRelease()
{
	/* if statements don't work, we need a feature "start from current time" */
	//if (parBP1Reset < (double)MonoSynth::EnvResetMode::env_never)
	Breakpoint1Module->noteOff(MIDIMASTER.getState().note);
	//if (parBP2Reset < (double)MonoSynth::EnvResetMode::env_never)
	Breakpoint2Module->noteOff(MIDIMASTER.getState().note);
}

void JerobeamNyquistShannonModule::setBeatsPerMinute(double v)
{
	MIDIMASTER.setBPM(v);

	Breakpoint1Module->setBeatsPerMinute(v*0.25);
	Breakpoint2Module->setBeatsPerMinute(v*0.25);

	delayModule->delay.setTempoInBPM(v);
}

void JerobeamNyquistShannonModule::setPlayheadInfo(AudioPlayHead::CurrentPositionInfo info)
{
	jbNyquistCore.setPlayheadInfo(info);
}

void JerobeamNyquistShannonModule::processBlock(double **inOutBuffer, int numChannels, int numSamples)
{
	const bool doApplyModulations = modulationManager.getNumConnections() != 0;

	const bool doApplySmoothing = smoothingManager->needsSmoothing();

	for (int n = 0; n < numSamples; n++)
	{
	  double left = 0, right = 0;

		bool isSilent = jbNyquistCore.isSilent();

		if (doApplySmoothing)
			smoothingManager->updateSmoothedValues();

		if (!isSilent && doApplyModulations)
			modulationManager.applyModulations();

		MIDIMASTER.incrementPitchGlide();

		if (!isSilent)
			jbNyquistCore.processSampleFrame(&left, &right);

		equalizerModule->processStereoFrame(&left, &right);
		limiterModule->processSampleFrame(&left, &right);
		delayModule->processSampleFrame(&left, &right);
		oscilloscopeModule->processStereoFrame(&left, &right);

		if (inOutBuffer[0]) inOutBuffer[0][n] = left;
		if (inOutBuffer[1]) inOutBuffer[1][n] = right;
	}	
}
