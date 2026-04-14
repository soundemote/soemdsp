#pragma once

using jura::MetaParameterManager;
using jura::ModulationManager;

class BasicOscillatorModule : public BasicModule, public jura::ModulationSource
{
  friend class BasicOscillatorEditor;

public:
	BasicOscillatorModule(juce::CriticalSection * lockToUse, MetaParameterManager * metaManagerToUse, ModulationManager * modulationManager, MidiMaster * midiMasterToUse);

	// plugin
	//jura::AudioModuleEditor *createEditor(int type = 0) override;
	void setupModulesAndModulation() override;
	void createParameters() override;
	void setupCallbacks() override;
	void setInitPatchValues() override;
	void setHelpTextAndLabels() override;

	// host
	void setSampleRate(double v) override;
	void processBlock(double **inOutBuffer, int numChannels, int numSamples) override;
	
	// ModulationSource
	double renderModulation() override
	{
		basicOscillatorCore.processSampleFrame(&modValue, &modValue);
		return modValue;
	}

	// dsp
	void setNoiseSeed(int v) { basicOscillatorCore.osc.setNoiseSeed(v); }

	// oscillator
	void setWaveform(int v)
	{
		basicOscillatorCore.osc.setWaveform(v);
		String newName = getModulationSourceName();
		switch (v)
		{
		case BasicOscillator::WaveformMode::NONE:
			newName += ": PhaseCtrl";
			break;
		case BasicOscillator::WaveformMode::NOISE_ONLY:
			newName += ": Noise";
			break;
		case BasicOscillator::WaveformMode::SINE:
			newName += ": Sine";
			break;
		case BasicOscillator::WaveformMode::TRI:
			newName += ": Tri";
			break;
		case BasicOscillator::WaveformMode::SAW:
			newName += ": Saw";
			break;
		case BasicOscillator::WaveformMode::PHASOR:
			newName += ": Saw noAA";
			break;
		case BasicOscillator::WaveformMode::SQUARE:
			newName += ": Sqr";
			break;
		case BasicOscillator::WaveformMode::RAND_SQUARE:
			newName += ": RandSqr";
			break;
		case BasicOscillator::WaveformMode::RAND_TRI:
			newName += ": RandTri";
			break;
		case BasicOscillator::WaveformMode::RAND_SAW:
			newName += ": RandSaw";
			break;
		case BasicOscillator::WaveformMode::RAND_SLOPE_BI:
			newName += ": R UpDn Pk";
			break;
		case BasicOscillator::WaveformMode::RAND_SLOPE_UNI:
			newName += ": R UpDn Ct";
			break;
		}
		setModulationSourceDisplayName(newName);
	}

	// common
	myparams parResetMode{ &paramManager };
	ParameterProfile::BipolarAmplitude parGain{ &paramManager };
	ParameterProfile::BipolarFineFrequency parFrequency{ &paramManager };
	ParameterProfile::BipolarCoarseFrequency parFreqCoarse{ &paramManager };
	ParameterProfile::Tempo parTempo{ &paramManager };
	ParameterProfile::Semitone parTune{ &paramManager };
	myparams parKeytracking{ &paramManager };
	ParameterProfile::LowpassFilterFrequency parLPF{ &paramManager };
	ParameterProfile::HighpassFilterFrequency parHPF{ &paramManager };
	ParameterProfile::Oversampling parOversampling{ &paramManager };

	// basic oscillator
	myparams parUnipolar{ &paramManager };
	myparams parInvert{ &paramManager };
	myparams parWaveform{ &paramManager };
	ParameterProfile::Linear parPhaseOffset{ &paramManager };
	ParameterProfile::Linear parNoiseAmp{ &paramManager };
	ParameterProfile::Linear parInternalAmp{ &paramManager };
	ParameterProfile::ExponentialTime parEnvDelay{ &paramManager };
	ParameterProfile::ExponentialTime parEnvAttack{ &paramManager };
	ParameterProfile::ExponentialTime parEnvDecay{ &paramManager };
	myparams parEnvSustain{ &paramManager };
	ParameterProfile::ExponentialTime parEnvRelease{ &paramManager };

	/* Core DSP objects */
	BasicOscillatorCoreOversampled basicOscillatorCore;
	jura::BreakpointModulatorAudioModule * breakpointEnv1Module;

protected:

	bool isUnipolar = false;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BasicOscillatorModule)
};
