#pragma once

#include "JuceHeader.h"
#include <jura_framework/audio/jura_AudioModule.h>
#include "se_framework/PluginComponents/myparams.h"

class CurveEnvelope
{
public:
	CurveEnvelope()
	{
		env.modifyBreakpoint(0, 0.0, 0.0, rosic::ModBreakpoint::ANALOG, 1.0); // start
		env.modifyBreakpoint(1, 0.0, 1.0, rosic::ModBreakpoint::ANALOG, 1.0); // attack
		env.modifyBreakpoint(2, 1.0, 0.5, rosic::ModBreakpoint::ANALOG, 1.0); // sustain-loop start
		env.modifyBreakpoint(3, 2.0, 0.5, rosic::ModBreakpoint::ANALOG, 1.0); // sustain-loop end
		env.modifyBreakpoint(4, 3.0, 0.0, rosic::ModBreakpoint::ANALOG, 1.0); // end
		env.setLoopMode(true);
		env.setLoopStartIndex(2);
		env.setLoopEndIndex(3);
	}
	virtual ~CurveEnvelope() = default;

	void setSampleRate(double v) { env.setSampleRate(v); }

	double getSample() { return env.getSample(); }

	void setIsBypassed(bool v)
	{
		isBypassed = v;

		if (isBypassed)
			env.noteOff();
	}

	void triggerOn()
	{
		if (isBypassed)
			return;

		if (resetOnTrigger)
			env.noteOn(false);
		else if (resetIfNotLegato)
			env.noteOn(noteIsOn);
		else
			env.noteOn(true);

		noteIsOn = true;
	}
	void triggerOff()
	{
		noteIsOn = false;
		env.noteOff();
	}

	bool isSilent() { return env.endIsReached; }

	void setEnableSync(bool v)
	{
		env.setSyncMode(v);
	}
	void setBPM(double v)
	{
		env.setBeatsPerMinute(v);
	}

	void setEnableLoop(bool v)
	{
		isLooping = v;

		if (isLooping)
		{
			env.setLoopStartIndex(0);
			env.setLoopEndIndex(1);
		}
		else
		{
			env.setLoopStartIndex(2);
			env.setLoopEndIndex(3);
		}

		recalculateSustainLevel();
	}

	void recalculateSustainLevel()
	{
		if (isLooping)
			env.setBreakpointLevel(2, 0);
		else
			env.setBreakpointLevel(2, sustain_level*amp_scale);

		env.setBreakpointLevel(3, env.getBreakpointLevel(2));
	}
	
	void setAmplitudeScale(double x)
	{
		//amp_scale = x;
		//env.setBreakpointLevel(1, amp_scale);
		//env.setBreakpointLevel(2, sustain_level*amp_scale);
		//env.setBreakpointLevel(3, sustain_level*amp_scale);
	}

	void setAttack(double x)
	{
		attack = x;
		env.setBreakpointTime(1, attack);
		env.setBreakpointLevel(1, amp_scale);
	}

	void setDecay(double x)
	{
		decay = x;
		env.setBreakpointTime(2, attack + decay);
		env.setBreakpointTime(3, attack + decay + 1.0);
		env.setBreakpointLevel(2, sustain_level*amp_scale);
		env.setBreakpointLevel(3, sustain_level*amp_scale);
	}

	void setSustainLevel(double v)
	{
		sustain_level = v;
		recalculateSustainLevel();
	}

	void setRelease(double v)
	{
		release = v;
		env.setBreakpointTime(4, attack + decay + 1.0 + release);
		env.setBreakpointLevel(4, 0.0);
	}


	void setAttackCurve(double v)
	{
		if (v > 0.0)
			env.setBreakpointShape(1, rosic::ModBreakpoint::ANALOG);
		else
			env.setBreakpointShape(1, rosic::ModBreakpoint::GROWING);

		env.setBreakpointShapeAmount(1, v);

	}

	void setDecayCurve(double v)
	{
		if (v > 0.0)
			env.setBreakpointShape(2, rosic::ModBreakpoint::ANALOG);
		else
			env.setBreakpointShape(2, rosic::ModBreakpoint::GROWING);

		env.setBreakpointShapeAmount(2, v);
	}

	void setReleaseCurve(double v)
	{
		if (v > 0.0)
			env.setBreakpointShape(4, rosic::ModBreakpoint::ANALOG);
		else
			env.setBreakpointShape(4, rosic::ModBreakpoint::GROWING);

		env.setBreakpointShapeAmount(4, v);
	}

	void setResetOnTrigger(bool v)
	{
		resetOnTrigger = true;
	}

protected:

	double attack = 0.1;
	double decay = 0.5;
	double release = 1;
	double amp_scale = 1;
	double sustain_level = 1;

	bool isLooping = false;
	bool resetOnTrigger = false;
	bool resetIfNotLegato = false;
	bool noteIsOn = false;

	bool isBypassed = false;

	rosic::BreakpointModulator env;
};

class TriSawOscillatorCore
{
public:
	TriSawOscillatorCore();
	virtual ~TriSawOscillatorCore() = default;

	Phasor phasor;

	void setSampleRate(double v)
	{
		phasor.setSampleRate(v);
		recalculateAsymLimit();
	}
	void reset();

	double getSample()
	{
		double p = phasor.getSample();

		double h = 0.5 * (limitedAsym + 1);
		double a0 = -1;
		double a1 = 2 / h;
		double b0 = (1 + h) / (1 - h);
		double b1 = -1 - b0;

		if (p < h)
			return TriSaw::shape(a0 + a1 * p, atkbend, -0.5 * atksig); // upward section
		else
			return TriSaw::shape(b0 + b1 * p, -decbend, -0.5 * decsig);  // downward section
	}

	void setFrequency(double v)
	{
		phasor.setFrequency(v);
		recalculateAsymLimit();
	}
	void setPhaseOffset(double v);
	void setAsymmetrical(double v)
	{
		incomingAsym = v;
		recalculateAsym();
	}

	void setAttackBend(double v);
	void setDecayBend(double v);
	void setAttackSigmoid(double v);
	void setDecaySigmoid(double v);

	double atkbend;
	double decbend;
	double atksig;
	double decsig;

	double incomingAsym;
	double limitedAsym;
	double asymLimit;

private:

	const double asymTransitionSamplesMin = 1;

	void recalculateAsymLimit()
	{
		double inc = phasor.getIncrementAmount();
		asymLimit = std::max(0.0, 1.0 - 2.0 * asymTransitionSamplesMin*inc);
		recalculateAsym();
	}
	void recalculateAsym()
	{
		limitedAsym = clip(incomingAsym, -asymLimit, +asymLimit);
	}

	typedef RAPT::rsTriSawOscillator<double> TriSaw;
};

class NoteAndPitchQuantizer : public MidiSlave
{
public:
	NoteAndPitchQuantizer()
	{
		noteChangeTimer.triggerFunction = [this]()
		{
			noteLengthTimer.reset(0.0);
			noteLengthTimer.startTimer();
			recalculatePitch();

			noteOnFunction();

			if (currentNote != lastNote)
				if (noteLenghtMod >= 1.0)
					ArpLegatoFunction();
				else
					arpNoteFunction();

			lastNote = currentNote;
		};

		noteLengthTimer.triggerFunction = [this]()
		{
			noteLengthTimer.stopTimer();
			noteOffFunction();
		};

		stepChangeTimer.triggerFunction = [this]()
		{
			//noteChangeTimer.reset();
			recalculatePitch();
			arpStepFunction();
		};

		fullResetTimer.triggerFunction = [this]()
		{
			noteChangeTimer.reset();
			stepChangeTimer.reset();
			recalculatePitch();
			arpPatternFunction();
		};

		recalculateNoteChangeFrequency();
		recalculateNoteLengthFrequency();
		recalculateStepChangeFrequency();
		recalculateFullResetFrequency();
	}

	~NoteAndPitchQuantizer() = default;

	// any note on message except if note is already held (prevents stuck notes)
	void triggerNoteOn() override
	{
		addEnabledNote(MidiSlave::midiMasterPtr->getNoteOn());
	}
	// any note off message
	void triggerNoteOff() override
	{
		removeEnabledNote(MidiSlave::midiMasterPtr->getNoteOff());
	}
	// (LEGATO) note on messages if no notes are currently held
	void triggerAttack() override
	{
		currentNote = MidiSlave::midiMasterPtr->getState().note;
		UserNoteOnFunction();
		startAllTimers();
	}
	// (LEGATO) note on messages if a note is already held
	void triggerNoteOnLegato() override
	{
		isCallbackTriggeredByUser = true;
		currentNote = MidiSlave::midiMasterPtr->getState().note;
		UserLegatoFunction();
	}
	// (LEGATO) note off messages if most recent held note is released
	void triggerNoteOffLegato() override
	{
		currentNote = MidiSlave::midiMasterPtr->getState().note;
		UserLegatoFunction();
	}
	// (LEGATO) note off message if last held note is released
	void triggerRelease() override
	{
		noteOffFunction();
		resetAndStopAllTimers();
		resetAndStopAllTimers();
	}

	void triggerBPMChange() override
	{
		BPM = MidiSlave::midiMasterPtr->getBPM();
		recalculateNoteChangeFrequency();
		recalculateNoteLengthFrequency();
		recalculateStepChangeFrequency();
		recalculateFullResetFrequency();
	}

	void setSampleRate(double v);

	void reset();

	void increment()
	{
		if (isBypassed)
			return;

		noteChangeTimer.incrememt();
		stepChangeTimer.incrememt();
		noteLengthTimer.incrememt();
		fullResetTimer.incrememt();
	}

	void resetAndStopAllTimers()
	{
		noteChangeTimer.reset();
		noteLengthTimer.reset(0.0);
		stepChangeTimer.reset();
		fullResetTimer.reset();

		noteChangeTimer.stopTimer();
		noteLengthTimer.stopTimer();
		stepChangeTimer.stopTimer();
		fullResetTimer.stopTimer();
	}

	void setIsBypassed(bool v)
	{
		isBypassed = v;
		if (isBypassed)
			resetAndStopAllTimers();
		else
			startAllTimers();
	}
	bool getIsBypassed()
	{
		return isBypassed;
	}

	void startAllTimers()
	{
		noteChangeTimer.startTimer();
		noteLengthTimer.startTimer();
		stepChangeTimer.startTimer();
		fullResetTimer.startTimer();
	}

	double getQuantizedPitch();
	int getLowestHeldNote() { return enabledNotesSorted[0]; }

	std::function<void()> noteOnFunction = []() { return; };
	std::function<void()> noteOffFunction = []() { return; };

	std::function<void()> UserNoteOnFunction = []() { return; };
	std::function<void()> ArpNoteOFunctionn = []() { return; };
	std::function<void()> UserLegatoFunction = []() { return; };
	std::function<void()> ArpLegatoFunction = []() { return; };

	std::function<void()> arpNoteFunction = []() { return; };
	std::function<void()> arpStepFunction = []() { return; };
	std::function<void()> arpPatternFunction = []() { return; };

	void setPitchValue(double v);
	void setOctaveAmplitude(double v);
	void setPitchOffset(double v);

	void setStepLengthBars(double v);
	void setNoteChangeBars(double v);
	void setNoteLengthBars(double v);
	void setNoteLengthModifier(double v)
	{
		noteLenghtMod = v;
		recalculateNoteLengthFrequency();
	}
	void setFullResetBeats(double v)
	{
		fullResetBeats = v;
		if (fullResetBeats > 128)
		{
			fullResetTimer.stopTimer();
		}
		recalculateFullResetFrequency();
	}
	void setPatternResetSpeedMultiplier(double v)
	{
		resetSpeedMultiplier = v;
		recalculateNoteChangeFrequency();
		recalculateNoteLengthFrequency();
		recalculateStepChangeFrequency();
		//recalculateFullResetFrequency();
	}

	void addEnabledNote(int v);
	void removeEnabledNote(int v);
	void removeAllEnabledNotes(int v);

	void recalculatePitch();

protected:

	PreciseTimer noteChangeTimer;
	PreciseTimer noteLengthTimer;
	PreciseTimer stepChangeTimer;
	PreciseTimer fullResetTimer;

	// beat value of 1 occurs 4 times in one bar given 120 BPM.
	double convertBeatsToFrequency(double BPM, double beats);

	// bar value of 1 occurs 1 time in one second given 120 BPM.
	double convertBarsToFrequency(double BPM, double bars);

	void recalculateNoteChangeFrequency();
	void recalculateNoteLengthFrequency();
	void recalculateStepChangeFrequency();
	void recalculateFullResetFrequency();

	bool isBypassed = false;
	bool isCallbackTriggeredByUser = false;

	double BPM = 120;

	double stepChangeBars = 1/16;
	double noteChangeBars = 1/16;
	double noteLengthBars = 1/16;
	double noteLenghtMod = 1;
	double fullResetBeats = 8;
	double resetSpeedMultiplier = 1;

	double pitchValue = 0; // input signal

	double octaveAmplitude = 1; // how much to multiply incoming pitch values
	double pitchOffset = 0; // how much to ofset incoming pitch values
	double totalPitchAmplitude = 1; // total pitch amplitude calculated based on octave amplitude and number of pitch classes
	double currentUnquantizedPitch; // final value for unquantized pitch
	int currentQuantizedPitch; // final value for quantized pitch

	int currentNoteClass = 0; // unquantized pitch converted to pitch class
	int currentNote = 36; // final output
	int currentOctave = 0;
	int lastNote = 0;

	bool asPlayedEnabled = true;
	int reverseOrderModifier = 1;
	int middleC = 48;

	juce::Array<int> enabledNotesSorted;
	juce::Array<int> enabledNotesUnsorted;
	DefaultElementComparator<int> sorter;
};

class LFOCore
{
public:
	LFOCore() = default;
	~LFOCore() = default;

	void setSampleRate(double v)
	{
		triSawOsc.setSampleRate(v);
	}

	double getSample()
	{
		if (isBypassed)
			return 0;

		double out = triSawOsc.getSample() * amplitude * phaseInversion;

		if (isUnipolar)
			out = bipolarToUnipolar(out);

		return bipolarToUnipolar(out);
	}

	void setIsUnipolar(bool v)
	{
		isUnipolar = v;
	}

	void reset() { triSawOsc.reset(); }

	void setAmplitude(double v) { amplitude = v; }

	void setIsBypassed(bool v)
	{
		isBypassed = v;
	}

	void setPhaseOffset(double v)
	{
		triSawOsc.setPhaseOffset(v);
	}

	void setFrequencyOffset(double v)
	{
		frequencyOffset = v;
		recalculateFrequency();
	}

	void setPhaseInverted(bool v)
	{
		phaseInversion = v ? -1 : 1;
	}

	void setOctaveOffset(double v)
	{
		octaveModifier = v * 12.0;
		recalculateFrequency();
	}

	void setPitchOffset(double v)
	{
		pitchModifier = v;
		recalculateFrequency();
	}

	void setNote(int v)
	{
		currentNote = v;
		recalculateFrequency();
	}

	void setPortamento(double v) { double portamentoTime = v; }

	void setWaveAsymmetrical(double v) { triSawOsc.setAsymmetrical(v); }

	void setWaveBendAttack(double v) { triSawOsc.setAttackBend(v); }

	void setWaveBendDecay(double v) { triSawOsc.setDecayBend(v); }

	void setWaveSigmoidAttack(double v) { triSawOsc.setAttackSigmoid(v); }

	void setWaveSigmoidDecay(double v) { triSawOsc.setDecaySigmoid(v); }

protected:
	bool isBypassed = false;
	bool isUnipolar = false;

	double phaseInversion = 1;
	double octaveModifier = 0;
	double pitchModifier = 0;
	double frequencyOffset = 0;
	double amplitude = 1;
	double currentNote = 36;
	double currentNoteSmoothed = 36;
	double portamentoTime = 0;

	void recalculateFrequency()
	{
		double f = pitchToFreq(octaveModifier + pitchModifier + currentNote);
		triSawOsc.setFrequency(f + frequencyOffset);
	}

	TriSawOscillatorCore triSawOsc;
};

class OutputOscCore : public LFOCore
{
public:
	void setWaveAsymmetrical(double v) { triSawOsc.setAsymmetrical(v); }

	void setWaveBendAktDec(double v)
	{
		bend = v;
		recalculateBend();
	}
	void setWaveBendOffset(double v)
	{
		bendOffset = v;
		recalculateBend();
	}

	void setWaveSigmoidAtkDec(double v)
	{
		sigmoid = v;
		recalculateSigmoid();
	}
	void setWaveSigmoidOffset(double v)
	{
		sigmoidOffset = v;
		recalculateSigmoid();
	}

protected:

	void recalculateBend()
	{
		double absoluteValue = bendOffset;
		double target = 1;

		if (bendOffset < 0)
		{
			absoluteValue = -bendOffset;
			target = -target;
		}

		triSawOsc.setAttackBend(juce::jmap(absoluteValue, bend, target));
		triSawOsc.setDecayBend(juce::jmap(absoluteValue, -bend, target));
	}
	void recalculateSigmoid()
	{
		double absoluteValue = sigmoidOffset;
		double target = 1;

		if (sigmoidOffset < 0)
		{
			absoluteValue = -sigmoidOffset;
			target = -target;
		}

		triSawOsc.setAttackSigmoid(juce::jmap(absoluteValue, sigmoid, target));
		triSawOsc.setDecaySigmoid(juce::jmap(absoluteValue, sigmoid, -target));
	}

	double bend = 0;
	double bendOffset = 0;
	double sigmoid = 0;
	double sigmoidOffset = 0;
};

class ChaosOscCore
{
public:
	ChaosOscCore()
	{
		lorenz.setRho(28);
		lorenz.setSigma(10);
		lorenz.setBeta(8.0/3.0);
	}

	void setSampleRate(double v)
	{
		filter.setSampleRate(v);
		lorenz.setSampleRate(v);
	}

	double getSample()
	{
		lorenz.iterateState();
		double x, y, z;
		lorenz.getState(&x, &y, &z);
		rotator.apply(&x, &y, &z);	

		currentValue = x * phaseInversion;
		//currentValue = triSawOsc.getSample() * phaseInversion;
		currentValueFiltered = filter.getSample(currentValue);

		currentArpOutValue = isArpOutFiltered ? currentValueFiltered : currentValue;
		if (isArpOutUnipolar)
			currentArpOutValue = bipolarToUnipolar(currentArpOutValue);
		currentArpOutValue *= arpOutAmp;

		return currentValueFiltered;
	}

	void reset()
	{
		filter.reset();
 		lorenz.reset();
	}

	// Main oscillator
	void setPhaseOffset(double v) { phaseOffset = v; }

	void setPhaseInverted(bool v) { phaseInversion = v ? -1 : 1; }

	void setIsArpOutUnipolar(bool v) { isArpOutUnipolar = v; }

	void setOctaveOffset(double v)
	{
		octaveModifier = v*12;
		recalculatePitchtoFreq();
	}

	void setPitchOffset(double v)
	{
		pitchModifier = v;
		recalculatePitchtoFreq();
	}

	void setPitchEnabled(bool v)
	{
		isPitchEnabled = v;
		recalculatePitchtoFreq();
	}

	void setTempo(double v) {}

	void setFrequencyOffset(double v)
	{
		frequencyModifier = v;
		recalculateFrequency();
	}

	void rotateX(double v) { rotator.setAngleX(v); }
	void rotateY(double v) { rotator.setAngleY(v); }
	void rotateZ(double v) { rotator.setAngleZ(v); }
	void setSigma(double v) { lorenz.setSigma(v); }
	void setRho(double v) { lorenz.setRho(v); }
	void setBeta(double v) { lorenz.setBeta(v); }

	void setArpOutAmp(double v) { arpOutAmp = v; }

	void setIsArpOutFiltered(bool v) { isArpOutFiltered = v; }

	double currentValue = 0;
	double currentValueFiltered = 0;
	double currentArpOutValue = 0;

	// Receives
	double dummyDouble = 0;
	double * inputValueToFM = &dummyDouble;
	double * inputValueToPM = &dummyDouble;

	// Filter
	void setLowpassCutoff(double v) { filter.setLowpassCutoff(v); }
	void setHighpassCutoff(double v) { filter.setHighpassCutoff(v); }
	elan::LorentzSystem lorenz;

protected:

	void recalculatePitchtoFreq()
	{
		pitchToFreqModifier = !isPitchEnabled ? 0 : pitchToFreq(octaveModifier + pitchModifier);
		recalculateFrequency();
	}
	void recalculateFrequency()
	{
		lorenz.setPseudoFrequency(frequencyModifier + pitchToFreqModifier);
	}

	double phaseOffset = 0;
	double phaseInversion = 1;

	double octaveModifier = 0;
	double pitchModifier = 0;
	double pitchToFreqModifier = 440;
	double frequencyModifier = 0;

	bool isPitchEnabled = false;

	double arpOutAmp = 0;

	bool isArpOutFiltered = false;
	bool isArpOutUnipolar = true;

	MonoTwoPoleBandpass filter;
	RAPT::rsRotationXYZ<double> rotator;
};
