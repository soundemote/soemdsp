#pragma once

class BasicOscillatorCore : public MidiSlave
{	
public:
	enum KeytrackingMode { Off, On };
	enum ResetMode { on_note, when_not_legato, when_silent, one_shot, never };
	static const vector<String> ResetModeMenuItems;

	BasicOscillatorCore() = default;
  virtual ~BasicOscillatorCore() = default;

	/* Mono Synth */
	void setSampleRate(double v);
	virtual void processSampleFrame(double *outL, double *outR);

	virtual void triggerAttack() override;
	virtual void triggerNoteOnLegato() override;
	virtual void triggerNoteOffLegato() override;
	virtual void triggerRelease() override;

	void triggerBPMChange() override { updateBeatFreq(); }
	void triggerMasterRateChange() override { updateFreq(); }
	void triggerPitchChange() override { calculatePitchToFrequency(); }

	void triggerOneShotRelease();

	bool isSilent();
	void reset();
	void setResetMode(ResetMode v)
	{
		resetMode = v;

		updateFreq();
	}

	void handleOneShotMode()
	{
		if (resetMode != ResetMode::one_shot
			|| oneShotIsFinished
			|| adsr.getEnvState() == LinearADSR::EnvState::DELAY)
				return;

		if (phaseTimer >= 1)
		{
			oneShotIsFinished = true;
			adsr.triggerRelease();
			osc.setFrequency(0);
			return;
		}

		phaseTimer += abs(osc.phasor.getIncrementAmount());
	}

	void setGain(double v) { gain = v; }

	void setIsUnipolar(bool v) { isUnipolar = v; }	
	void setIsInverted(bool v) { isInverted = v; }

	void setResetWhenTempoChanges(bool v) { resetWhenBeatTempoOrMultChanges = v; }

	void setFreqFine(double v) { freqFine = v; updateFreq(); }
	void setFreqCoarse(double v) { freqCoarse = v; updateFreq(); }
	void setSemitoneOffset(double v) { semitoneOffset = v; calculatePitchToFrequency(); }

	void setTempoSelection(int v)
	{
		jassert(v >= 0 && v < (int)MidiMaster::TempoMultipliers.size());

		tempoSelection = v;

		if (resetWhenBeatTempoOrMultChanges)
			reset();

		updateBeatFreq();
	}

	void setKeytrackingMode(KeytrackingMode v) { keytrackingMode = v;	updateFreq();	}

	BasicOscillator osc;
	LinearADSR adsr;
	MonoTwoPoleBandpass filter;

protected:

	KeytrackingMode keytrackingMode = KeytrackingMode::Off;
	ResetMode resetMode = ResetMode::never;

	void calculatePitchToFrequency()
	{
		currentPitchToFreq = RAPT::rsPitchToFreq(getMidiState().noteSmoothed + semitoneOffset);
		updateFreq();
	}

	void updateBeatFreq()
	{
		beatFreq = midiMasterPtr->getBPMFactor() * MidiMaster::TempoMultipliers[tempoSelection];
		updateFreq();

		if (resetWhenBeatTempoOrMultChanges)
			osc.reset();		
	}

	void updateFreq()
	{ 
		if (resetMode == ResetMode::one_shot && oneShotIsFinished)
		{
			osc.setFrequency(0);
			return;
		}

		if (keytrackingMode == KeytrackingMode::On)
			osc.setFrequency((freqFine + currentPitchToFreq + beatFreq) * getMidiMasterPtr()->getMasterRate());
		else
			osc.setFrequency((freqFine + freqCoarse + beatFreq) * getMidiMasterPtr()->getMasterRate());
	}

	double gain = 0;
	double freqFine = 1;
	double freqCoarse = 0;
	double beatFreq = 0;
	double semitoneOffset = 0;
	double currentPitchToFreq = 1;
	double finalFrequency = 1;
	double phaseTimer = 0;

	bool isUnipolar = false;
	bool isInverted = false;
	double currentValue = 0;
	bool resetWhenBeatTempoOrMultChanges = false;
	bool oneShotIsFinished = true;
};

class BasicOscillatorCoreOversampled : public BasicOscillatorCore
{
public:
	BasicOscillatorCoreOversampled() = default;
	virtual ~BasicOscillatorCoreOversampled() = default;

	void setOversampling(int newFactor)
	{
		oversampling = newFactor;
		BasicOscillatorCore::setSampleRate(oversampling * hostSampleRate);

		antiAliasFilterL.setSubDivision(oversampling);
		antiAliasFilterR.setSubDivision(oversampling);
	}

	void setSampleRate(double sr);

	void processSampleFrame(double *outL, double *outR) override;
protected:
	double hostSampleRate = 44100.0;
	double oversampling = 1.0;
	EllipticSubBandFilter antiAliasFilterL, antiAliasFilterR;
};
