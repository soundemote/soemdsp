#include "BasicOscillatorModule.h"

BasicOscillatorModule::BasicOscillatorModule(juce::CriticalSection * lockToUse, MetaParameterManager * metaManagerToUse, ModulationManager * modulationManager, MidiMaster * midiMasterToUse) :
	BasicModule(lockToUse, metaManagerToUse, modulationManager)
{
	basicOscillatorCore.midiMasterPtr = midiMasterToUse;
	BasicModule::setupParametersAndModules();
}

void BasicOscillatorModule::setupModulesAndModulation()
{
}

void BasicOscillatorModule::createParameters() //BasicOscillator
{
	// common
	parResetMode.initCombobox("Reset Mode", BasicOscillatorCore::ResetMode::when_not_legato, BasicOscillatorCore::ResetModeMenuItems);
	parGain.initSlider("Gain", -8, 8, 1);
	parFrequency.initSlider("Frequency", -20, +20, 0);
	parFreqCoarse.initSlider("Coarse Tune", -5000, +5000, 0);
	parTempo.initSlider("Tempo", 0, double(TempoOptionsStr.size()-1), 0);
	parTune.initSlider("Tune", -72, 72, 0);
	parKeytracking.initButton("Keytracking", 0);

	// oscillator
	parUnipolar.initButton("Unipolar", 0);
	parInvert.initButton("Invert", 0);
	parWaveform.initCombobox("Waveform", 2);
	parPhaseOffset.initSlider("Phase", 0, 1, 0);
	parNoiseAmp.initSlider("NoiseAmplitude", 0, 1, 0);

	// adsr
	parEnvDelay.initSlider("Delay", 1.e-5, 5, 1.e-5);
	parEnvAttack.initSlider("Attack", 1.e-5, 10, 1.e-2);
	parEnvDecay.initSlider("Decay", 1.e-5, 10, 1);
	parEnvSustain.initSlider("Sustain", 0, 1, 1);
	parEnvRelease.initSlider("Release", 1.e-5, 10, 1);

	// filter
	parHPF.initSlider("HighpassFreq", .01, 24000, .01);
	parLPF.initSlider("LowpassFreq", .01, 24000, 24000);

	parOversampling.initSlider("Oversampling", 1, 8, 1);

	parWaveform.addMenuItems({ "Phase Ctrl", "Noise",
		"SINE", "TRI", "SAW", "SAW (no AA)", "SQUARE",
		"Random Square", "Random Tri", "Random Saw", "R. Up/Dn Peak", "R. Up/Dn Center" });

	paramManager.instantiateParameters(this);
}

void BasicOscillatorModule::setupCallbacks()
{
	parGain.setCallback([this](double v) { basicOscillatorCore.setGain(v); });
	parResetMode.setCallback([this](double v) { basicOscillatorCore.setResetMode((BasicOscillatorCore::ResetMode)(int)v); });
	parFrequency.setCallback([this](double v) { basicOscillatorCore.setFreqFine(v); });
	parFreqCoarse.setCallback([this](double v) { basicOscillatorCore.setFreqCoarse(v); });
	parTempo.setCallback([this](double v) { basicOscillatorCore.setTempoSelection((int)v); });
	parTune.setCallback([this](double v) { basicOscillatorCore.setSemitoneOffset(v); });
	parKeytracking.setCallback([this](double v) { basicOscillatorCore.setKeytrackingMode((BasicOscillatorCore::KeytrackingMode)(int)(v)); });

	parUnipolar.setCallback([this](double v) { basicOscillatorCore.setIsUnipolar(v > 0.5); });
	parInvert.setCallback([this](double v) { basicOscillatorCore.setIsInverted(v > 0.5); });
	parWaveform.setCallback([this](double v) { setWaveform((int)v); });
	parPhaseOffset.setCallback([this](double v) { basicOscillatorCore.osc.setPhaseOffset(v); });
	parNoiseAmp.setCallback([this](double v) { basicOscillatorCore.osc.setNoiseAmp(v); });

	parEnvDelay.setCallback([this](double v) { basicOscillatorCore.adsr.setDelayTime(v); });
	parEnvAttack.setCallback([this](double v) { basicOscillatorCore.adsr.setAttackTime(v); });
	parEnvDecay.setCallback([this](double v) { basicOscillatorCore.adsr.setDecayTime(v); });
	parEnvSustain.setCallback([this](double v) { basicOscillatorCore.adsr.setSustainAmplitude(v); });
	parEnvRelease.setCallback([this](double v) { basicOscillatorCore.adsr.setReleaseTime(v); });

	parHPF.setCallback([this](double v) { basicOscillatorCore.filter.setHighpassCutoff(v); });
	parLPF.setCallback([this](double v) { basicOscillatorCore.filter.setLowpassCutoff(v); });

	parOversampling.setCallback([this](double v) { basicOscillatorCore.setOversampling((int)v); });
}

void BasicOscillatorModule::setInitPatchValues()
{
}

void BasicOscillatorModule::setHelpTextAndLabels()
{
	parKeytracking.text = "KyTr->";
	parResetMode.text = "Reset";
	parWaveform.text = "Shape";
	parNoiseAmp.text  = "Random Gain";
	parTune.text = "Tune";
	parFreqCoarse.text = "Freq";
	parFrequency.text = "Freq (Fine)";
	parUnipolar.text = "Uni";
	parInvert.text = "Inv";
}

void BasicOscillatorModule::setSampleRate(double v)
{
	sampleRate = v;
	basicOscillatorCore.setSampleRate(v);
}

void BasicOscillatorModule::processBlock(double** inOutBuffer, int /*numChannels*/, int numSamples)
{
	double left, right;
	for (int n = 0; n < numSamples; n++)
	{
		basicOscillatorCore.processSampleFrame(&left, &right);

		if (inOutBuffer[0]) inOutBuffer[0][n] = left;
		if (inOutBuffer[1]) inOutBuffer[1][n] = right;
	}
}
