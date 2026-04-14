#pragma once

#include "JuceHeader.h"

class ChaosflyCore : public MonoSynth
{

public:

	/** This is an enumeration of the available waveforms for the oscillators. */
	enum waveforms
	{
		SINE,
		TRIANGLE,
		SQUARE,
		SAW_UP,
		SAW_DOWN,
		ELLIPSE,

		NUM_WAVEFORMS
	};

	/** Enumeration of different synchronization modes. */
	enum syncModes
	{
		SYNC_OFF,        // no sync
		SYNC_RESET,          // reset to start phase
		SYNC_JUMP,           // jump phase 
		SYNC_REVERSE,        // reverse direction

		NUM_SYNCMODES
	};

	ChaosflyCore();

	bool isSilent() override;

	/** Computes a stereo output sample frame at a time */
	virtual void processSampleFrame(double* InL, double* InR, double* OutL, double* OutR);

	/** Sets the sample rate in Hz. */
	void setSampleRate(double v);

	/** Sets the basic frequency of the carrier oscillator (oscillator 2). */
	void setFrequency(double newFrequency);
	void setPitch(double v);
	void setOctaveOffset(double v);
	void setTuneOffset(double v);

	/** Sets the factor by which the filtered input signal is mixed into the output. */
	void setInputMix(double newMixFactor);

	/** Sets the scale factor for the cutoff frequency of the input filter with respect to the master
	frequency. */
	void setInputFilterFreqScaler(double newScaler);

	/** Sets the amount of detuning for the input filter with respect to the master frequency (in
	semitones). */
	void setInputFilterDetune(double newDetune);

	/** Sets the waveform for the input oscillator (the signal of which will get mixed into the audio
	audio input). */
	void setInputOscWaveform(int newWaveform);

	/** Sets the frequency scaler for the input oscillator with respect to the master frequency. */
	void setInputOscFrequencyScaler(double newScaler);

	void setOscInOctave(double v);
	void setInputOscDetune(double v);

	/** Sets the amplitude of the input oscillator. */
	void setInputOscAmplitude(double newAmplitude);

	void setOsc1Amplitude(double v) { osc1Amp = v; }
	void setOsc2Amplitude(double v) { osc2Amp = v; }
	void setOsc1Bypass(bool v) { bypassOsc1 = v; }
	void setOsc2Bypass(bool v) { bypassOsc2 = v; }

	/** Sets the ratio of the frequencies of the two oscillators. It acts as a multiplier for the
	frequency of oscillator 1 (seen as modulator in the most simple FM/PM patch). Oscillator 2 (the
	carrier) will use the master frequency as is. I think, using it this way means the value is
	actually the reciprocal of what is called C/M ratio in FM synthesis. (check this) */
	void setFrequencyRatio(double newRatio);

	/** Sets a scale factor for the depth of both modulations 1->2 and 2->1. */
	void setModulationDepthScaler(double newScaler);

	void setModDepth11(double v) { mod11 = v; }
	void setModDepth22(double v) { mod22 = v; }
	void setModDepth12(double v) { mod12 = v; }
	void setModDepth21(double v) { mod21 = v; }

	void setFreqVsPhaseMod11(double v) { fmpm11 = v; }
	void setFreqVsPhaseMod22(double v) { fmpm22 = v; }
	void setFreqVsPhaseMod12(double v) { fmpm12 = v; }
	void setFreqVsPhaseMod21(double v) { fmpm21 = v; }

	/** Depth of the modulation envelope as normalized value between 0..1. */
	void setModEnvDepth(double newDepth);

	/** Sets the 1st filter's cutoff frequency as scale factor with respect to the  master
	frequency. */
	void setFilter1FreqScaler(double newScaler);
	void setFilter2FreqScaler(double newScaler);  // same for 2nd filter

	/** Sets up the 1st filter's frequency scale factor in terms of a detune value in
	semitones.   */
	void setFilter1Detune(double newDetune);
	void setFilter2Detune(double newDetune);

	void setFilter1Resonance(double newReso);
	void setFilter2Resonance(double newReso);
	void setFilter1Mode(int newMode);
	void setFilter2Mode(int newMode);

	void setOutputOsc1PreFilter(bool v) { outputOsc1PreFilter = v; }
	void setModulateWithOsc1ValuePreFilter(bool v) { modulateWithOsc1ValuePreFilter = v; }

	// \todo maybe make both filters multimode ladders, call them Filter1, Filter2 instead of 
	// Lowpass/Highpass and make their position in the signal flow adjustable

	/** Sets the amount by which the input signal is added to the feedback as multiplier. The input
	will be added to the input of the feedback filter. */
	void setInputToFeedback(double newScaler);

	/** Sets the center frequency (as multiplier for the master frequency) for the feedback EQ. */
	void setFeedbackFreqScaler(double newScaler);

	/** Sets up the feedback filter's frequency scale factor in terms of a detune value in
	semitones.   */
	void setFeedbackDetune(double newDetune);

	/** Sets the amount of boost (or cut) in decibels for the feedback EQ. */
	void setFeedbackBoostAmount(double newAmount);
	void setFeedbackBoostWidth(double newWidth);

	/** Sets a linear gain multiplier for final output. */
	void setOutputGainRaw(double newGain);

	void setOutputBias(double v);

	/** Sets a gain for final output in decibels. */
	void setOutputGainDecibels(double newGain);

	/** Sets the detuning with respect to the master frequency for oscillator 1 (in semitones) */
	void setDetune1(double newDetune);
	void setDetune2(double newDetune); // same for osc2

	/** Sets a multiplicative frequency factor for osc1. */
	void setFreqFactor1(double newFactor);
	void setFreqFactor2(double newFactor);
	void setFreqOffset1(double newOffset);
	void setFreqOffset2(double newOffset);

	void setWaveform1(int newWaveform);
	void setWaveform2(int newWaveform);
	void setClipLevel1(double newLevel);
	void setClipLevel2(double newLevel);
	// todo: replace with setClipWidth1/2 ...but then we need to update patches or introduce some
	// backwards-compatibility code in the patch-loader

	/** Sets the clipping center. */
	void setClipCenter1(double newCenter);
	void setClipCenter2(double newCenter);

	/** Sets the lower and upper limit of oscillator frequency. */
	void setOsc1FreqMin(double limit);
	void setOsc2FreqMin(double limit);
	void setOsc1FreqMax(double limit);
	void setOsc2FreqMax(double limit);

	void setPhase1(double newPhase);
	void setPhase2(double newPhase);
	void setPhase1Degrees(double newPhase);
	void setPhase2Degrees(double newPhase);

	/** Selects the synchronization mode - this determines, what happens when a sync-trigger is
	encountered. Possible values are: 0: phase reset, 1: phase jump, 2: direction reversal. */
	void setSyncMode(int newMode);

	/** Sets the threshold for the input edge detector. When an edge in the input is detected that is
	above this threshold, the sound generator will be reset. */
	void setSyncThreshold(double newThreshold);

	/** Sets the amount of the synchronization effect as value between 0 and 1. 0: no sync, 1: full
	sync, between: partial sync (reset phase in between current value and start value) */
	void setSyncAmount(double newAmount);

	/** Decide, where the filters should be reset on sync events (as opposed to resetting only the
	oscillators). */
	void setFilterResetOnSync(bool shouldReset);

	// "Ducking" parameters: the instantaneous amplitude of our sound generator output will be 
	// attenuated according to the instaneous amplitude of the input signal. The louder the input,
	// the more attenuation is applied to the output. This can be used to emulate the voltage 
	// limiting effects in analog filters. It applies an input-dependent microenvelope to our output
	// signal.

	void setDuckBypass(bool v) { duckBypass = v; }

	/** Sets the range of amplitudes for the input signal for which the output signal of the
	sound generator will be not ducked away completetly. This will affect the total width of the
	microenvelope. */
	void setDuckingRange(double newRange);

	/** Sets the relative (with respect to the total range) width of the flat top zone between 0
	and 1. At 0, we see a nice bell shape for the microenvelope and towards 1 it becomes more and
	more squarish. */
	void setDuckingFlatness(double newFlatness);

	/** You may shift the center value for the ducking from 0 to somewhere else. For a sawtooth
	input, this shifts the microenvelope back and forth within the cycle. */
	void setDuckingCenter(double newCenter);

	/** Sets the transition shape for the ducking: 0: linear, 1: cubic, 2: quintic, 3: heptic. */
	void setDuckingShape(int newShape);

	/** Sets the parameter that makes the ducker "fill the square" (0: no fill, 1: full fill). This
	feature is under construction... */
	void setDuckingFill(double newFill);

	/** Sets the level at which the chaos generator's input signal is clipped. */
	void setInputClipLevel(double v);

	/** Sets the level at which the chaos generator's output signal is clipped. */
	void setOutputClipLevel(double newLevel);

	void reset();
	void resetOscillators();
	void resetFilters();

	/** \name Embedded Objects */

	LadderFilter inFilter; // filter for input signal
	LadderFilter filter1;
	LadderFilter filter2;
	rosic::BreakpointModulator ampEnv, modEnv; // ADSR envelopes for amplitude and modulation
	StateVariableFilter fbFilter; // bandpass filter in feedback path
	ParametricBellFunction ducker;   // voltage limit simulation
	ScaledAndShiftedSigmoid sat1, sat2;   // saturators for 1st and 2nd osc

	BasicOscillator osc1, osc2, inputOsc;

	void setEllipseA1(double v) { xoxos_A_1 = v; }
	void setEllipseA2(double v) { xoxos_A_2 = v; }
	void setEllipseB1(double v);
	void setEllipseB2(double v);
	void setEllipseC1(double v) { xoxos_C_1 = v; }
	void setEllipseC2(double v) { xoxos_C_2 = v; }

	double
		xoxos_A_1{},
		xoxos_B_sin_1{},
		xoxos_B_cos_1{ 1.0 },
		xoxos_C_1{},
		xoxos_A_2{},
		xoxos_B_sin_2{},
		xoxos_B_cos_2{ 1.0 },
		xoxos_C_2{};

	elan::NoiseGenerator noise;
	double noiseAmp{ 0 };
	MonoTwoPoleBandpass noiseFilter;
	OnePoleFilterStereo hpFilter;

	rosic::SoftKneeCompressor compressor;

	double feedbackGain{};
	double inputSignal{};
	double outsideSignal{};

	/** Triggers oscillator reset (and possibly filter reset, too). Called from syncIfNeccessary. */
	void triggerSync();
	void syncOscillators(int osc1or2 = 0);
	// maybe include an "advance" parameter later that advances the state for a subsample time
	// difference (which should be obtained by determining the subsample time instant opf the sync
	// event)

protected:

	/** Given a current input sample "in" and a previous input sample "inOld", this function figures
	out, if we need to trigger a sync reset event. */
	bool needsSyncTrigger(double in, double inOld);
	// todo: include a return parameter that is used for subsample precision sync (figures out,
	// when exactly between the current and previous sample, the sync event occurred

	double softClip(double x, double level);

	// user parameters:
	double inputClipLevel{10};
	double inMix{}; // factor for mixing filtered input to output
	double tuneOffset{};
	double octaveOffset{};
	double pitch{36};
	double frequency{440}; // the master frequency (applies to both oscillators)
	double freqRatio{1};
	double modScale{1}; // master modulation depth scaler (scales mod12, mod21)

	double osc1Amp{1};
	double osc2Amp{1};
	bool bypassOsc1 = false;
	bool bypassOsc2 = false;

	bool outputOsc1PreFilter = false;
	bool modulateWithOsc1ValuePreFilter = false;

	double // modulation depths
		mod11{},
		mod22{},
		mod12{},
		mod21{};

	double  // frequency modulation (FM) vs phase modulation (PM)
		fmpm11{},
		fmpm22{},
		fmpm12{},
		fmpm21{};

	double inScl{1}; // cutoff scaler for input filter

	double // cutoff scalers for filters
		hpScl{0.1},
		lpScl{10};

	double inToFb{}; // scaler for feeding input to feedback
	double fbScl{}; // frequency scaler for feedback boost

	double // phase offsets
		phase1{},
		phase2{};

	double clipOut{2}; // clipping level for output
	double gain{1}; // linear gain for output
	double bias{}; // linear DC bias for output
	double modEnvDepth{1}; // depth of the modulation envelope

	double
		freqFactor1{1},
		freqFactor2{1},
		freqOffset1{},
		freqOffset2{};

	double
		Osc1FreqMin = -DBL_MAX,
		Osc1FreqMax = +DBL_MAX,
		Osc2FreqMin = -DBL_MAX,
		Osc2FreqMax = +DBL_MAX;

	// sync:
	int syncMode{}; // 0: reset, 1: jump, 2: reverse
	double syncThresh{2}; // synchronization threshold for input edge
	double syncAmount{1};  // amount of sync, 0: hardsync, 1: no sync, between: partial sync
	bool syncFlt = true; // switch to turn on/off filter reset on sync

	// ducker:
	bool duckBypass = false;
	double duckCenter{}; // center value for ducker
	double duckWidth{2}; // width for ducker
	double duckLo{-4}; // lower threshold for ducker/micro-envelope sync
	double duckHi{+4}; // upper threshold for ducker/micro-envelope sync
	double duckFill{}; // "filling-the-square" parameter (under construction)

	// input oscillator:
	double oscInOctave{};
	double oscInFrqScl{1};
	double oscInAmp{};
	double oscInDetune{};

	// state variables:
	double inOld{}; // remembered input signal

	double osc1_mod_self{};
	double osc2_mod_self{};
	double lastOutValue{};

	double osc1Value{};
	double osc2Value{}; // output signal of both oscillators
	double direction{+1}; // +1 or -1, multiplies phase increment

	double incomingFreq{440};

	void recalculateFrequency();

	//double posIn{}, pos1{}, out1{}, pos2{}, out2{};
	int shape1{}, shape2{}, oscInShape{};
};

/* robin's notes:
 - use a square rule for the FM/PM parameters - it seems the output bandwidth is proportional to the modulation index squared
 - check the clipping function - it seems it goes above the identity function
 - add smoothing parameter for phase jumping
*/

/* another way to do sync
 - generate an input saw
 - forget about edge detection
 - at saw edge, if above threshold, oscillate until 180 degrees when it enters threshold again,
   continue oscillating starting at 180 degrees when it leaves threshold again,
	 oscillate from 180 to 360 (back to 0) degrees

i mean you just need a few if statements checking phase at any given moment

if (above_threshold && phase < 180) continue_oscillating();

 - implement a "slide-back" behavior

elan: wtf is a slide-back behavior? maybe that's a slow sync or something, where phase slides back instead of instant reset
*/

class ChaosflyCoreOversampled : public ChaosflyCore
{
public:
	ChaosflyCoreOversampled() = default;
	virtual ~ChaosflyCoreOversampled() = default;

	void setOversampling(int v)
	{
		oversampling = v;
		ChaosflyCore::setSampleRate(oversampling * hostSampleRate);

		antiAliasFilterL.setSubDivision(oversampling);
		antiAliasFilterR.setSubDivision(oversampling);
	}

	void setSampleRate(double v)
	{
		hostSampleRate = v;
		ChaosflyCore::setSampleRate(oversampling * hostSampleRate);
	}

	void processSampleFrame(double* InL, double* InR, double* OutL, double* OutR) override;
protected:
	double hostSampleRate{44100};
	double oversampling{1};
	EllipticSubBandFilter antiAliasFilterL, antiAliasFilterR;
};
