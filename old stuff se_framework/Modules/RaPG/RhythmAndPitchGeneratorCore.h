#pragma once

#include "JuceHeader.h"
#include "se_framework/ElanSynthLib/ElanSynthLib.h"

class RaPG_Envelope : public CurveEnvelope, public TriggerReceiver
{
	RaPG_Envelope() = default;
	~RaPG_Envelope() = default;

	void triggerWasReceived() override
	{
		triggerOn();
	}
};

class CoreParameter_ExponentialFrequency
{
public:
	CoreParameter_ExponentialFrequency(double freq) { setFrequency(freq); };
	~CoreParameter_ExponentialFrequency() = default;

	double getFrequency()
	{
		return clip(finalFrequency, .1, 20000.0);
	}

	void setFrequency(double v)
	{
		frequency = v;
		recalculateFrequency();
	}

	void setFrequencyOffset(double v)
	{
		frequencyOffset = v;
		recalculateFrequency();
	}

	void setFrequencyMod(double v)
	{
		frequencyMod = v;
		recalculateFrequency();
	}

protected:

	void recalculateFrequency()
	{
		finalFrequency = std::max<double>(0, exp(10 * frequencyOffset) + frequency * .001 * exp(10 * frequencyMod) - 28);
	}

	double frequency = 20000;
	double frequencyOffset = 0;
	double frequencyMod = 0;
	double finalFrequency = 20000;
};

class CoreParameter_Pitch_Frequency
{
public:
	CoreParameter_Pitch_Frequency(double pitch) { setPitch(pitch); };
	~CoreParameter_Pitch_Frequency() = default;

	double getFrequency()
	{
		return clip(finalFrequency, -20000.0, +20000.0);
	}

	void setPitch(double v)
	{
		pitch = v;
		recalculatePitch();
	}

	void setPitchOffset(double v)
	{
		pitchOffset = v;
		recalculatePitch();
	}

	void setFrequencyOffset(double v)
	{
		frequencyOffset = v;
		recalculateFrequency();
	}

	double getValue()
	{
		return finalFrequency;
	}

	void setRate(double v)
	{
		rate = v;
		recalculateFrequency();
	}

	double getRate()
	{
		return rate;
	}

protected:

	void recalculatePitch()
	{
		pitchToFrequencyValue = pitchToFreq(pitch + pitchOffset);
		recalculateFrequency();
	}

	void recalculateFrequency()
	{
		finalFrequency = pitchToFrequencyValue + frequencyOffset;
		finalFrequency *= rate;
	}

	double pitch = 69;
	double pitchOffset = 0;
	double frequencyOffset = 0;
	double rate = 1;
	double pitchToFrequencyValue = 440;
	double finalFrequency = 440;
};

class CoreParameter_Amplitude
{
public:
	CoreParameter_Amplitude(double initValue) { setValue(initValue); }
	~CoreParameter_Amplitude() = default;

	void setValue(double v)
	{
		value = v;
		recalculate();
	}

	void setMod(double v)  
	{
		mod = v;
		recalculate();
	}

	double getValue()
	{
		return out;
	}

protected:

	void recalculate()
	{
		out = value * mod;
	}

	double value = 0;
	double mod = 1;
	double out = 1;
};

class RaPG_Oscillator : public TriSawOscillator
{
public:
	RaPG_Oscillator()
	{
		filter.setMode(RAPT::rsOnePoleFilter<double, double>::LOWPASS_BLT);
		paramSmoother.setSmootherType(ParamSmoother::type::GAUSSIAN);
	}
	~RaPG_Oscillator() = default;

	void setSampleRate(double v)
	{
		sampleRate = v;
		TriSawOscillator::setSampleRate(v);
		filter.setSampleRate(v);
		paramSmoother.setSampleRate(v);
		noiseMinFreq = v * 2.0/12.0;
		maxFreq = sampleRate;
		noise.setSampleRate(v);
	}

	void reset()
	{
		TriSawOscillator::reset();
	}

	double getSample()
	{
		pitch.setPitch(paramSmoother.inc());

		double f = pitch.getFrequency();
		double noiseAmp = 0;
		if (f > noiseMinFreq)
		{
			noise.setFrequency(f*2);
			noise.increment();
			noiseAmp = jmap<double>(abs(f), noiseMinFreq, maxFreq, 0, 0.05);
		}

		double oscFreq = [&]()
		{
			double nosieVal = noise.getSmoothedNoiseValue() * noiseAmp;
			return f + f * nosieVal;
		}();

		TriSawOscillator::setFrequency(oscFreq);

		triSawValue = TriSawOscillator::getSample();
		filteredTriSawValue = filter.getSample(triSawValue);
		filteredAmplifiedTriSawValue = filteredTriSawValue * amplitude.getValue();

		return filteredAmplifiedTriSawValue;
	}

	void setAmplitude(double v)
	{
		amplitude.setValue(v);
	}

	void setAmplitudeMod(double v)
	{
		amplitude.setMod(v);
	}

	void setNote(double v, bool glideToNote)
	{
		paramSmoother.setTargetValue(v);
		if (!glideToNote)
			paramSmoother.setInternalValue(v);
	}

	void setGlideSpeed(double v)
	{
		glideSpeed = v;
		recalculateGlide();
	}

	void setPitchMod(double v)
	{
		pitch.setPitchOffset(v);
		TriSawOscillator::setFrequency(pitch.getFrequency());
	}

	void setFrequencyMod(double v)
	{
		pitch.setFrequencyOffset(v);
		TriSawOscillator::setFrequency(pitch.getFrequency());
	}

	void setFilterFrequency(double v)
	{
		filterFrequency.setFrequency(v);
		filter.setCutoff(filterFrequency.getFrequency());
	}

	void setFilterFrequencyMod(double v)
	{
		filterFrequency.setFrequencyMod(v);
		filter.setCutoff(filterFrequency.getFrequency());
	}

	void setFilterFrequencyOffset(double v)
	{
		filterFrequency.setFrequencyOffset(v);
		filter.setCutoff(filterFrequency.getFrequency());
	}

	void informMasterRate(double v, bool modulatePitch)
	{
		masterRate = v;
		if (modulatePitch)
			pitch.setRate(v);
		else
			pitch.setRate(1);

		recalculateGlide();
	}

	void informBPM(double v)
	{
		BPM = v;
		recalculateGlide();
	}
	 
	double triSawValue = 0;
	double filteredTriSawValue = 0;
	double filteredAmplifiedTriSawValue = 0;

protected:

	ParamSmoother paramSmoother;

	OnePoleFilter filter;
	elan::RandomGeneratorTimed noise;

	void recalculateGlide()
	{
		paramSmoother.setSmoothingTime((120.0/BPM * glideSpeed) / masterRate);
	}

	CoreParameter_Amplitude amplitude{ 0 };
	CoreParameter_Pitch_Frequency pitch{ 69 };
	CoreParameter_ExponentialFrequency filterFrequency{ 20000 };

	double glideSpeed = 0;
	double smoothedNote = 69;
	double note = 69;
	double masterRate = 1;
	double BPM = 120;
	double maxFreq = 22050;
	double noiseMinFreq = 14700;
	double sampleRate = 44100;
};

class HeldNotesTracker
{
public:
	HeldNotesTracker() = default;
	~HeldNotesTracker() = default;

	void addEnabledNote(int v)
	{
		enabledNotesSorted.addSorted(sorter, v);
		enabledNotesUnsorted.add(v);
		numHeldNotes = enabledNotesSorted.size();
	}

	void removeEnabledNote(int v)
	{
		enabledNotesSorted.removeFirstMatchingValue(v);
		enabledNotesUnsorted.removeFirstMatchingValue(v);
		numHeldNotes = enabledNotesSorted.size();
	}
	void removeAllEnabledNotes(int v)
	{
		enabledNotesSorted.clear();
		enabledNotesUnsorted.clear();
		numHeldNotes = 0;
	}

	int getLowestNote()
	{
		if (numHeldNotes <= 0)
			return -1;

		return enabledNotesSorted.getFirst();
	}

	int getMiddleNoteHigherBias()
	{
		if (numHeldNotes <= 0)
			return -1;

		int index = (int)ceil(0.5 + double(numHeldNotes) / 2.0);

		return enabledNotesSorted[index];
	}

	int getMiddleNoteLowerBias()
	{
		if (numHeldNotes <= 0)
			return -1;

		int index = (int)ceil(double(numHeldNotes) / 2.0);

		return enabledNotesSorted[index];
	}

	int getHighestNote()
	{
		if (numHeldNotes <= 0)
			return -1;

		return enabledNotesSorted.getLast();
	}

protected:

	int numHeldNotes = 0;

	juce::Array<int> enabledNotesSorted;
	juce::Array<int> enabledNotesUnsorted;
	DefaultElementComparator<int> sorter;
};

class RaPG_RhythmGenerator;

class RaPG_ClockDivider : public ClockDivider, public MidiSlave
{
public:

	RaPG_ClockDivider(RaPG_RhythmGenerator * masterRhthmGenerator);
	~RaPG_ClockDivider() = default;

	void run();

	// called when host BPM cahnges
	void triggerBPMChange() override;

	void setSampleRate(double v);

	void setDivision(int v)
	{
		ClockDivider::setDivision(v);
		recalculateTriggerFrequency();
	}

	void informMasterTimerFrequency(double v)
	{
		masterTimerFrequency = v;
		recalculateTriggerFrequency();
	}

	void setPrimaryPulseDelayNormalized(double v);

	void setSecondaryPulseDelayNormalized(double v);

	void recalculateTriggerFrequency();

	void recalculatePulseFrequency(int index);

	void setIsEnabled(bool v);

	double primaryPulseDelayNormalized = 0;
	double secondaryPulseDelayNormalized = 0;
	bool isEnabled = true;
	double triggerFrequency = .25;
	double bpm = 120;
	double masterTimerFrequency = 1;

	DelayedTrigger pulseTimer1;
	DelayedTrigger pulseTimer2;

	int currentTimer = 0;

	int id;

	RaPG_RhythmGenerator * master;
};

class RaPG_RhythmGenerator
{
	friend class RaPG_ClockDivider;
public:
	RaPG_RhythmGenerator();
	~RaPG_RhythmGenerator() = default;

	void run();
	void trigger();
	void reset()
	{
		currentStep = 0;

		noise.reset();

		masterClockDivider.reset();

		for (auto & clock : AllClocksArray)
			clock->reset();
	}

	void clockWasTriggered(RaPG_ClockDivider * clock);

	void setMasterClockDivision(int division)
	{
		masterClockDivider.setDivision(division);
		for (auto & clock : AllClocksArray)
			clock->recalculateTriggerFrequency();
	}
	int getClockDivision() { return masterClockDivider.getDivision(); }
	void setSeed(int v) { noise.setSeed(v); }
	void setSteps(int v) { steps = v; }
	void setOffset(double v) { offsetValue = v; }
	void setEnableGhostClicks(bool v) { enableGhostClicks = v; }
	void setIsEnabled(bool v) { isEnabled = v; }
	int getCurrentClockIndex()
	{
		if (currentClock != nullptr)
			return currentClock->id;
		return 0;
	}

	std::function<void()> triggerFunction = []() { return; };
	std::function<void()> ghostTriggerFunction = []() { return; };

	vector<RaPG_ClockDivider *> AllClocksArray;
	Array<RaPG_ClockDivider*> RunningClockArray;
	Array<RaPG_ClockDivider*> EnabledClockArray;

	double clockIndexValueForOutput = 0;
	double noiseValue = 0;

protected:
	ClockDivider masterClockDivider;
	elan::NoiseGenerator noise;

	RaPG_ClockDivider * currentClock = nullptr;
	RaPG_ClockDivider * currentGhostClock = nullptr;

	bool enableGhostClicks = true;
	int steps = 8;	
	double offsetValue = 0;
	int numClocksEnabled;
	int currentStep = 0;
	bool isEnabled = true;
	int enabledClockIndex = 0;
	int enabledGhostClockIndex = 0;
};

class RaPG_PitchQuantizer : public HeldNotesTracker
{
public:
	RaPG_PitchQuantizer()
	{
		clockdivider.setTriggerFunction([this]()
		{
			if (numSteps > 0)
			{
				currentStep = currentStep % numSteps;

				if (currentStep == 0)
					noise.reset();
			}

			noiseValue = noise.getSampleUnpiolar();

			recalculatePitch();

			if (lastNote != currentNote)
			{
				if (noteLengthTimer.getIsTimerRunning() || noteLengthMod >= 2.0)
					legatoNoteFunction();
				else
					newNoteFunction();

				noteLengthTimer.restart();

				lastNote = currentNote;
			}

			++currentStep;
		});

		noteLengthTimer.setTriggerFunction([this]()
		{
			if (noteLengthMod <= 2.0)
				noteOffFunction();
		});
	}
	~RaPG_PitchQuantizer() = default;

	void setSampleRate(double v)
	{
		noteLengthTimer.setSampleRate(v);
	}

	void run()
	{
		noteLengthTimer.incrememt();
	}

	void reset()
	{
		noise.reset();
		clockdivider.reset();
		noteLengthTimer.reset();
		lastNote = -1;
		currentStep = 0;
	}

	void triggerClock()
	{
		clockdivider.trigger();
	}

	void triggerNoteOn(int note)
	{
		HeldNotesTracker::addEnabledNote(note);
	}

	void triggerNoteOff(int note)
	{
		HeldNotesTracker::removeEnabledNote(note);
	}

	double getPitch()
	{
		return currentNote;
	}

	void setOctaveAmplitude(double v)
	{
		octaveAmplitude = v;
	}

	void setOctaveOffset(double v)
	{
		octaveOffset = v;
	}

	void setIsUnipolar(bool v)
	{
		isUnipolar = v;
	}

	void setSeed(int v)
	{
		noise.setSeed(v);
	}

	void setSteps(int v)
	{
		numSteps = v;
	}

	void setClockDivide(int v)
	{
		clockdivider.setDivision(v);
	}

	void setIsEnabled(bool v)
	{
		isEnabled = v;
		recalculatePitch();
	}

	void setNoteLengthMod(double v)
	{
		noteLengthMod = v;
		recalculateNoteLengthFrequency();
	}

	std::function<void()> newNoteFunction = []() { return; };
	std::function<void()> legatoNoteFunction = []() { return; };
	std::function<void()> noteOffFunction = []() { return; };

	void informMasterTimerFrequency(double v)
	{
		masterTimerFrequency = v;
		recalculateNoteLengthFrequency();
	}

	double noiseValue = 0;

protected:
	double octaveAmplitude = 0;
	double octaveOffset = 0;
	bool isUnipolar = true;
	int currentNote = 36;
	int lastNote = -1;
	int currentStep = 0;
	int numSteps = 8;
	bool isEnabled = true;
	bool isLegato = false;
	double noteLengthMod = .5;
	double BPM = 120;
	double masterTimerFrequency = 1;
	double masterTriggerFrequency = 1;

	void recalculatePitch()
	{
		if (numHeldNotes == 0)
			return;

		double actualPitchValue = 0;

		if (isEnabled)
		{
			if (isUnipolar)
				actualPitchValue = noiseValue * octaveAmplitude;
			else
				actualPitchValue = unipolarToBipolar(noiseValue) * octaveAmplitude;
		}

		actualPitchValue += octaveOffset;

		int noteIndex = int(actualPitchValue * (double)numHeldNotes);
		
		int currentOctave, wrappedIndex;

		wrappedIndex = wrapIndexBipolarMultiOctave(noteIndex, numHeldNotes, &currentOctave);
		 
		currentNote = enabledNotesSorted[wrappedIndex];
		currentNote += currentOctave * 12;
	}

	void recalculateNoteLengthFrequency()
	{
		masterTriggerFrequency = masterTimerFrequency / (double)clockdivider.getDivision();

		noteLengthTimer.setSeconds(1/masterTriggerFrequency * noteLengthMod);
	}

	elan::NoiseGenerator noise;
	ClockDivider clockdivider;
	DelayedTrigger noteLengthTimer;
};
