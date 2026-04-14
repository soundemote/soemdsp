#include "ChaosflyCore.h"

#include <cfloat> // for DBL_MIN/MAX on linux


ChaosflyCore::ChaosflyCore()
{
  fbFilter.setMode(StateVariableFilter::BELL);
  inFilter.setMode(LadderFilter::LP_24);

	hpFilter.setMode(2);

  setFilter1Mode(LadderFilter::HP_6);
  setFilter2Mode(LadderFilter::LP_6);

  sat1.setSigmoid(&NormalizedSigmoids::softClipHexic);
  sat2.setSigmoid(&NormalizedSigmoids::softClipHexic);

  ampEnv.setEditMode(BreakpointModulator::EDIT_WITH_SHIFT);
  modEnv.setEditMode(BreakpointModulator::EDIT_WITH_SHIFT);
}

void ChaosflyCore::setSampleRate(double v)
{
	MonoSynth::sampleRate = v;

  filter1.setSampleRate(v);
  filter2.setSampleRate(v);
  fbFilter.setSampleRate(v);
	hpFilter.setSampleRate(v);

  ampEnv.setSampleRate(v);
  modEnv.setSampleRate(v);

	osc1.setSampleRate(v);
	osc2.setSampleRate(v);

	inputOsc.setSampleRate(v);
	inFilter.setSampleRate(v);

	noiseFilter.setSampleRate(v);

	compressor.setSampleRate(v);
}

void ChaosflyCore::setOctaveOffset(double v)
{
	octaveOffset = v*12;
	recalculateFrequency();
}

void ChaosflyCore::setTuneOffset(double v)
{
	tuneOffset = v;
	recalculateFrequency();
}

void ChaosflyCore::setPitch(double v)
{
	pitch = v;
	recalculateFrequency();
}

void ChaosflyCore::recalculateFrequency()
{
	frequency = RAPT::rsPitchToFreq(pitch + tuneOffset + octaveOffset);

	setInputFilterFreqScaler(inScl);
	setFilter1FreqScaler(hpScl);
	setFilter2FreqScaler(lpScl);
	setFeedbackFreqScaler(fbScl);
}

void ChaosflyCore::setInputMix(double newMixFactor)
{
  inMix = newMixFactor;
}

void ChaosflyCore::setInputFilterFreqScaler(double newScaler)
{
  inScl = newScaler;
  inFilter.setCutoff(RAPT::rsClip(inScl * frequency, 0.0, 20000.0));
}

void ChaosflyCore::setInputFilterDetune(double newDetune)
{
  setInputFilterFreqScaler(RAPT::rsPitchOffsetToFreqFactor(newDetune));
}

void ChaosflyCore::setInputOscWaveform(int newWaveform)
{
	switch (oscInShape = newWaveform)
	{
	case 0: inputOsc.setWaveform(BasicOscillator::WaveformMode::SINE); return;
	case 1: inputOsc.setWaveform(BasicOscillator::WaveformMode::TRI_NOAA); return;
	case 2: inputOsc.setWaveform(BasicOscillator::WaveformMode::SQUARE); return;
	case 3: inputOsc.setWaveform(BasicOscillator::WaveformMode::SAW_UP_NOAA); return;
	case 4: inputOsc.setWaveform(BasicOscillator::WaveformMode::SAW_DOWN_NOAA); return;
	}
}

void ChaosflyCore::setInputOscFrequencyScaler(double newScaler)
{
  oscInFrqScl = newScaler;
}

void ChaosflyCore::setOscInOctave(double v)
{
	setInputOscFrequencyScaler(RAPT::rsPitchOffsetToFreqFactor(oscInDetune + (oscInOctave = v*12)));
}

void ChaosflyCore::setInputOscDetune(double v)
{
  setInputOscFrequencyScaler(RAPT::rsPitchOffsetToFreqFactor((oscInDetune = v) + oscInOctave));
}

void ChaosflyCore::setInputOscAmplitude(double newAmplitude)
{
  oscInAmp = newAmplitude;
}

void ChaosflyCore::setFrequencyRatio(double newRatio)
{
  freqRatio = newRatio;
}

void ChaosflyCore::setModulationDepthScaler(double newDepth)
{
  modScale = newDepth;
}

void ChaosflyCore::setModEnvDepth(double newDepth)
{
  modEnvDepth = newDepth;
}

void ChaosflyCore::setFilter1FreqScaler(double newScaler)
{
  hpScl = newScaler;
  filter1.setCutoff(RAPT::rsClip(hpScl * frequency, 0.0, 20000.0));
}

void ChaosflyCore::setFilter2FreqScaler(double newScaler)
{
  lpScl = newScaler;
  filter2.setCutoff(RAPT::rsClip(lpScl * frequency, 0.0, 20000.0));
}

void ChaosflyCore::setFilter1Detune(double newDetune)
{
  setFilter1FreqScaler(RAPT::rsPitchOffsetToFreqFactor(newDetune));
}
void ChaosflyCore::setFilter2Detune(double newDetune)
{
  setFilter2FreqScaler(RAPT::rsPitchOffsetToFreqFactor(newDetune));
}

void ChaosflyCore::setFilter1Resonance(double v)
{
  filter1.setResonance(v);
}
void ChaosflyCore::setFilter2Resonance(double v)
{
  filter2.setResonance(v);
}

void ChaosflyCore::setFilter1Mode(int newMode)
{
  filter1.setMode(newMode);
}
void ChaosflyCore::setFilter2Mode(int newMode)
{
  filter2.setMode(newMode);
}

void ChaosflyCore::setInputToFeedback(double newScaler)
{
  inToFb = newScaler;
}

void ChaosflyCore::setFeedbackFreqScaler(double newScaler)
{
  fbScl = newScaler;
  fbFilter.setFrequency(RAPT::rsClip(fbScl * frequency, 0.0, 20000.0));
}

void ChaosflyCore::setFeedbackDetune(double newDetune)
{
  setFeedbackFreqScaler(RAPT::rsPitchOffsetToFreqFactor(newDetune));
}

void ChaosflyCore::setFeedbackBoostAmount(double newAmount)
{
  fbFilter.setGain(RAPT::rsDbToAmp(newAmount));
}

void ChaosflyCore::setFeedbackBoostWidth(double newWidth)
{
  fbFilter.setBandwidth(newWidth);
}

void ChaosflyCore::setOutputGainRaw(double newGain)
{
  gain = newGain;
}

void ChaosflyCore::setOutputBias(double v)
{
	bias = v;
}

void ChaosflyCore::setOutputGainDecibels(double newGain)
{
  setOutputGainRaw(RAPT::rsDbToAmp(newGain));
}

void ChaosflyCore::setDetune1(double newDetune)
{
  freqFactor1 = RAPT::rsPitchOffsetToFreqFactor(newDetune);
}

void ChaosflyCore::setDetune2(double newDetune)
{
  freqFactor2 = RAPT::rsPitchOffsetToFreqFactor(newDetune);
}

void ChaosflyCore::setFreqFactor1(double newFactor)
{
  freqFactor1 = newFactor;
}

void ChaosflyCore::setFreqFactor2(double newFactor)
{
  freqFactor2 = newFactor;
}

void ChaosflyCore::setFreqOffset1(double newOffset)
{
  freqOffset1 = newOffset;
}

void ChaosflyCore::setFreqOffset2(double newOffset)
{
  freqOffset2 = newOffset;
}

void ChaosflyCore::setWaveform1(int newWaveform)
{
	switch (shape1 = newWaveform)
	{
	case SINE: osc1.setWaveform(BasicOscillator::WaveformMode::SINE); return;
	case TRIANGLE: osc1.setWaveform(BasicOscillator::WaveformMode::TRI_NOAA); return;
	case SQUARE: osc1.setWaveform(BasicOscillator::WaveformMode::SQUARE); return;
	case SAW_UP: osc1.setWaveform(BasicOscillator::WaveformMode::SAW_UP_NOAA); return;
	case SAW_DOWN: osc1.setWaveform(BasicOscillator::WaveformMode::SAW_DOWN_NOAA); return;
	case ELLIPSE: osc1.setWaveform(BasicOscillator::WaveformMode::ELLIPSE); return;
	}  
}

void ChaosflyCore::setWaveform2(int newWaveform)
{
	switch (shape2 = newWaveform)
	{
	case SINE: osc2.setWaveform(BasicOscillator::WaveformMode::SINE); return;
	case TRIANGLE: osc2.setWaveform(BasicOscillator::WaveformMode::TRI_NOAA); return;
	case SQUARE: osc2.setWaveform(BasicOscillator::WaveformMode::SQUARE); return;
	case SAW_UP: osc2.setWaveform(BasicOscillator::WaveformMode::SAW_UP_NOAA); return;
	case SAW_DOWN: osc2.setWaveform(BasicOscillator::WaveformMode::SAW_DOWN_NOAA); return;
	case ELLIPSE: osc2.setWaveform(BasicOscillator::WaveformMode::ELLIPSE); return;
	}
}

void ChaosflyCore::setClipLevel1(double newLevel)
{
  sat1.setWidth(2*std::max<double>(newLevel,0.00001));
}

void ChaosflyCore::setClipLevel2(double newLevel)
{
  sat2.setWidth(2*std::max<double>(newLevel, 0.00001));
}

void ChaosflyCore::setClipCenter1(double newCenter)
{
  sat1.setCenter(newCenter);
}

void ChaosflyCore::setClipCenter2(double newCenter)
{
  sat2.setCenter(newCenter);
}

void ChaosflyCore::setOsc1FreqMin(double v)
{
	if (v <= -20000)
		v = -DBL_MAX;

	Osc1FreqMin = v;
}
void ChaosflyCore::setOsc1FreqMax(double v)
{
	if (v >= 20000)
		v = DBL_MAX;

	Osc1FreqMax = v;
}

void ChaosflyCore::setOsc2FreqMin(double v)
{
	if (v <= -20000)
		v = -DBL_MAX;

	Osc2FreqMin = v;
}

void ChaosflyCore::setOsc2FreqMax(double v)
{
	if (v >= 20000)
		v = DBL_MAX;

	Osc2FreqMax = v;
}

void ChaosflyCore::setPhase1(double v)
{
  phase1 = v;
}

void ChaosflyCore::setPhase2(double v)
{
  phase2 = v;
}

void ChaosflyCore::setPhase1Degrees(double v)
{
  setPhase1(v/360.0);
}

void ChaosflyCore::setPhase2Degrees(double v)
{
  setPhase2(v/360.0);
}

void ChaosflyCore::setSyncMode(int v)
{
  syncMode = v;
}

void ChaosflyCore::setSyncThreshold(double v)
{
  syncThresh = v;
}

void ChaosflyCore::setSyncAmount(double v)
{
  syncAmount = v;
}

void ChaosflyCore::setFilterResetOnSync(bool v)
{
  syncFlt = v;
}

void ChaosflyCore::setDuckingRange(double v)
{
  duckWidth = 2*v;
  duckLo = duckCenter - 0.5 * duckWidth;
  duckHi = duckCenter + 0.5 * duckWidth;
  ducker.setWidth(duckWidth);
}

void ChaosflyCore::setDuckingFlatness(double v)
{
  ducker.setFlatTopWidth(v);
}

void ChaosflyCore::setDuckingCenter(double v)
{
  duckCenter = v;
  duckLo = duckCenter - 0.5 * duckWidth;
  duckHi = duckCenter + 0.5 * duckWidth;
  ducker.setCenter(duckCenter);
}

void ChaosflyCore::setDuckingShape(int v)
{
  switch(v)
  {
  case 0: ducker.setPrototypeBell(PositiveBellFunctions::linear);  break;
  case 1: ducker.setPrototypeBell(PositiveBellFunctions::cubic);   break;
  case 2: ducker.setPrototypeBell(PositiveBellFunctions::quintic); break;
  case 3: ducker.setPrototypeBell(PositiveBellFunctions::heptic);  break;
  }
}

void ChaosflyCore::setDuckingFill(double v)
{
  duckFill = v;
}

void ChaosflyCore::setInputClipLevel(double v)
{
	inputClipLevel = v;
}

void ChaosflyCore::setOutputClipLevel(double v)
{
  clipOut = v;
}

void ChaosflyCore::reset()
{
	resetOscillators();
	resetFilters();
	hpFilter.reset();
	direction = +1; // forward
}

void ChaosflyCore::resetOscillators()
{
	osc1.reset();
	osc2.reset();
}

void ChaosflyCore::resetFilters()
{
	filter1.reset();
	filter2.reset();
	fbFilter.reset();
}

void ChaosflyCore::setEllipseB1(double v) { RAPT::rsSinCos(v*TAU, &xoxos_B_sin_1, &xoxos_B_cos_1); }

void ChaosflyCore::setEllipseB2(double v) { RAPT::rsSinCos(v*TAU, &xoxos_B_sin_2, &xoxos_B_cos_2); }

bool ChaosflyCore::needsSyncTrigger(double in, double inOld)
{
	if (syncMode == SYNC_OFF)
		return false;

	// sync to input edges:
	if (fabs(in-inOld) > syncThresh)
		return true;

	// sync, when input enters allowed amplitude range for ducker:
	if (inOld < duckLo && in >= duckLo)   // ...from below
		return true;
	if (inOld > duckHi && in <= duckHi)   // ...from above
		return true;

	return false;
}

void ChaosflyCore::triggerSync()
{
	syncOscillators();
	if (syncFlt)
		resetFilters();
}

void ChaosflyCore::syncOscillators(int osc1or2)
{
	switch (osc1or2)
	{
	case 1:
		switch (syncMode)
		{
		case SYNC_RESET:
		{
			osc1.phasor.partialReset(syncAmount);
		};
		break;
		case SYNC_JUMP:
		{
			osc1.phasor.increment(syncAmount);
		};
		break;
		case SYNC_REVERSE:
		{
			direction *= -1;
			osc1.phasor.partialReset(1 - syncAmount);
		};
		break;
		}
		break;
	case 2:
		switch (syncMode)
		{
		case SYNC_RESET:
		{
			osc2.phasor.partialReset(syncAmount);
		};
		break;
		case SYNC_JUMP:
		{
			osc2.phasor.increment(syncAmount);
		};
		break;
		case SYNC_REVERSE:
		{
			direction *= -1;
			osc2.phasor.partialReset(1 - syncAmount);
		};
		break;
		}
		break;
	default:
		switch (syncMode)
		{
		case SYNC_RESET:
		{
			osc1.phasor.partialReset(syncAmount);
			osc2.phasor.partialReset(syncAmount);
		};
		break;
		case SYNC_JUMP:
		{
			osc1.phasor.increment(syncAmount);
			osc2.phasor.increment(syncAmount);
		};
		break;
		case SYNC_REVERSE:
		{
			direction *= -1;
			osc1.phasor.partialReset(1 - syncAmount);
			osc2.phasor.partialReset(1 - syncAmount);
		};
		break;
		}
		break;
	}
}

double ChaosflyCore::softClip(double in, double level)
{
	return level * NormalizedSigmoids::softClipHexic(in / level);
}

bool ChaosflyCore::isSilent()
{
	return ampEnv.endIsReached;
}

double getWaveform(double phase, int shape, double A = 0, double B_sin = 1, double B_cos = 0, double C = 0)
{
	phase = wrapPhase(phase);

	switch (shape)
	{
	case 1: return waveshape::tri(phase);
	case 2: return waveshape::square(phase);
	case 3: return waveshape::ramp(phase);
	case 4: return	waveshape::saw(phase);
 	case 5: return waveshape::ellipse(phase, A, B_sin, B_cos, C);
	default: return waveshape::sine(phase);
	}
}

void ChaosflyCore::processSampleFrame(double *inL, double *inR, double *OutL, double *OutR)
{
	// input oscillator
	inputOsc.setFrequency(frequency*oscInFrqScl);
	double inputSignal = RAPT::rsClip(oscInAmp * inputOsc.getSample(), -inputClipLevel, +inputClipLevel);

	jassert(!isnan(inputSignal));

	// input signal
	inputSignal  += 0.5 * (*inL + *inR);
	double flt = inFilter.getSample(inputSignal);

	// ducker
	double duck = 1;
	if (duckBypass)
	{
		duck  = ducker.getValue(inputSignal);
		duck *= (1 + duckFill*(1-fabs(inputSignal)));
		duck /= (1+duckFill);
	}

	// sync
	if (needsSyncTrigger(inputSignal, inOld))
		triggerSync();

	inOld = inputSignal;

	// add noise to signal
	inputSignal += noiseFilter.getSample(noise.getSampleBipolar()) * noiseAmp;

	// establish feedback signal
	inputSignal = fbFilter.getSample(osc2Value + osc1_mod_self + inToFb*inputSignal + lastOutValue);

	// osc 1 & 2 mod
	double modEnvVal = modEnv.getSample();
	double modScaler = modScale * (1 + modEnvDepth*modEnvVal);

	// osc1
	if (bypassOsc1)
	{
		osc1Value = 0;
	}
	else
	{
		// mod
		double mod = modScaler * mod21 * inputSignal;
		double fm = cos(PI_z_2*fmpm21) * mod;
		double pm = sin(PI_z_2*fmpm21) * mod;

		double freq = direction * (freqOffset1 + frequency * freqRatio * (freqFactor1 + fm));
		double phaseOffset = phase1 + pm;

		if (Osc1FreqMin == -20000.0 && Osc1FreqMax == +20000.0)
			osc1.phasor.setFrequency(freq);			
		else
			osc1.phasor.setFrequency(RAPT::rsClip(freq, Osc1FreqMin, Osc1FreqMax));

		osc1.phasor.setPhaseOffset(phaseOffset);
		osc1.phasor.increment();
		osc1Value = getWaveform(osc1.phasor.getUnipolarValue(), shape1, xoxos_A_1, xoxos_B_sin_1, xoxos_B_cos_1, xoxos_C_1);

		// clip and amp
		osc1Value = sat1.getValue(osc1Value * osc1Amp);
	}

	// main feedback filters	
	inputSignal = filter1.getSample(osc1Value);
	inputSignal = filter2.getSample(inputSignal);

	// apply compression
	if (compressor.getDryWetRatio() > 1.e-6)
	{
		double compVal = inputSignal;
		compressor.getSampleFrameStereo(&compVal, &compVal);
		inputSignal = compVal;
	}

	// osc 1 & 2 self feedback
	osc1_mod_self = inputSignal * mod11;
	osc2_mod_self = mod22 * osc2Value;

	// osc2
	if (bypassOsc2)
	{
		osc2Value = 0;
	}
	else
	{		
		// mod
		double mod = modScaler  * mod12 * (modulateWithOsc1ValuePreFilter ? osc1Value : inputSignal) + osc2_mod_self;
		double fm = cos(PI_z_2*fmpm12) * mod;
		double pm = sin(PI_z_2*fmpm12) * mod;

		double freq  = direction * (freqOffset2 + frequency * (freqFactor2 + fm));
		double phaseOffset = phase2 + pm;

		if (Osc2FreqMin == -20000.0 && Osc2FreqMax == +20000.0)
			osc2.phasor.setFrequency(freq);
		else
			osc2.phasor.setFrequency(RAPT::rsClip(freq, Osc2FreqMin, Osc2FreqMax));			

		osc2.phasor.setPhaseOffset(phaseOffset);
		osc2.phasor.increment();
		osc2Value = getWaveform(osc2.phasor.getUnipolarValue(), shape2, xoxos_A_2, xoxos_B_sin_2, xoxos_B_cos_2, xoxos_C_2);

		// clip
		osc2Value = sat2.getValue(osc2Value * osc2Amp);
	}
	
	// amplitude envelope
	double ampEnvVal = ampEnv.getSample();
		
	double outL = gain * softClip(duck*ampEnvVal*(outputOsc1PreFilter ? osc1Value : inputSignal), clipOut) + ampEnvVal * inMix * flt;
	double outR = gain * softClip(duck*ampEnvVal*osc2Value, clipOut) + ampEnvVal * inMix * flt;

	lastOutValue = (outL+outR) * 0.5 * feedbackGain;

	// do highpass output filtering
	double hpFilterOut_L = outL + bias;
	double hpFilterOut_R = outR + bias;
	hpFilter.getSampleFrameStereo(&hpFilterOut_L, &hpFilterOut_R, &hpFilterOut_L, &hpFilterOut_R);

	jassert(!isnan(hpFilterOut_L));

	*OutL = hpFilterOut_L * gain;
	*OutR = hpFilterOut_R * gain;
}

void ChaosflyCoreOversampled::processSampleFrame(double * InL, double * InR, double * OutL, double * OutR)
{
	if (oversampling > 1)
	{
		double original_InL = *InL;
		double original_InR = *InR;

		original_InL = RAPT::rsClip(original_InL, -inputClipLevel, +inputClipLevel);
		original_InR = RAPT::rsClip(original_InR, -inputClipLevel, +inputClipLevel);

		for (int i = 0; i < oversampling; i++)
		{
			ChaosflyCore::processSampleFrame(&original_InL, &original_InR, OutL, OutR);

			/*channel mode*/
			MonoSynth::handleChannelMode(OutL, OutR);

			*OutL = antiAliasFilterL.getSampleDirect1(*OutL);
			*OutR = antiAliasFilterR.getSampleDirect1(*OutR);
		}
	}
	else
	{
		ChaosflyCore::processSampleFrame(InL, InR, OutL, OutR);
		MonoSynth::handleChannelMode(OutL, OutR);
	}
}
