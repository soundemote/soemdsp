#pragma once

#include "elan_juce_helpers/source/maths.h"

struct MIDISTATE
{
	int note = 48; // 0 to 127
	double noteSmoothed = 48; // 0 to 127

	double velocity = 0; // 0 to 1

	double pitchBend = 0; //  0 to 1
	double channelPressure = 0; //  0 to 10

	int aftertouchNote = -1; // 0 to 127
	double afterTouchValue = 0; // 0 to 1

	int ccIndex = 0; // 0 to 127
	double ccValue = 0; // 0 to 1
};

class MidiMaster;

class MidiSlave
{
	friend MidiMaster;

public:

	MidiSlave() = default;

	virtual ~MidiSlave() = default;

	MIDISTATE & getMidiState();
	MidiMaster * getMidiMasterPtr();
	double getCurrentFrequency() { return currentFrequency; }

	// (LEGATO) note on messages if no notes are currently held
	virtual void triggerAttack() {}
	// (LEGATO) note on messages if a note is already held and a different note is triggered
	virtual void triggerNoteOnLegato() {}
	// (LEGATO) note off messages if most recent held note is released
	virtual void triggerNoteOffLegato() {}
	// (LEGATO) note off message if last held note is released
	virtual void triggerRelease() {}

	// any note on message except if note is already held (prevents stuck notes)
	virtual void triggerNoteOn() {}
	// any note off message even if no notes are held
	virtual void triggerNoteOff() {}
	// only override this if you need special handling for allNotesOff, otherwise it is ALREADY HANDLED by triggerNoteOff callback!
	virtual void triggerAllNotesOff() {}

	// called when master rate changes
	virtual void triggerMasterRateChange() {}
	// called when pitch changes which includes midi note and smoothed pitch changes
	virtual void triggerPitchChange() {}
	// called when midi note is changed, this is when you want discrete pitch changes rather than per sample smoothed pitch changes
	virtual void triggerNoteChange() {}
	// called when host BPM cahnges
	virtual void triggerBPMChange() {}
	// called when midi velocity changes
	virtual void triggerVelocityChange() {}
	// called when midi pitch bend value is changed
	virtual void triggerPitchBendChange() {}
	// called when midi controller is changed
	virtual void triggerMidiControllerChange() {}
	// called when after touch value is changed
	virtual void triggerAfterTouchChange() {}
	// called when channel pressure is changed
	virtual void triggerChannelPressureChange() {}

	MidiMaster * midiMasterPtr = nullptr;
protected:	

	bool useOnlyNoteChangeTrigger = false;
	bool needsBPMChanges = false;
	double currentBeatFreq = 0;

	double currentFrequency = 0;

	int tempoSelection = 0;
	int beatMultiplierOption = 0;
	bool resetWhenBeatTempoOrMultChanges = true;
};

class MidiMaster
{
	friend MidiSlave;

public:

	static const vector<double> TempoMultipliers;

	/* IMPORTANT PER-SAMPLE CALLS */
	void incrementMidiBufferIterator();
	void incrementPitchGlide();	

	MidiMaster()
	{
		pitchSmoother.setSmootherType(ParamSmoother::type::EXPONENTIAL);
	}

	virtual ~MidiMaster() = default;

	virtual void setSampleRate(double newSampleRate);

	void noteOn(int key, int velocity, int channel = 1);
	void noteOff(int key, int channel = 1);
	void allNotesOff()
	{
		for (auto i : UserHeldNotes)
			noteOff(1, i);
		triggerAllNotesOff();
	}
	void setMidiController(int cc, int value, int /*channel = 1*/)
	{		
		state.ccIndex = cc;
		state.ccValue = value * INV_127;

		midiControllerValues[state.ccIndex] = state.ccValue;

		triggerMidiControllerChange();
	}
	void setPitchBend(int value, int /*channel = 1*/)
	{
		state.pitchBend = value * INV_8192 - 1;
		triggerPitchBendChange();
	}
	void setAfterTouch(int note, int value, int /*channel = 1*/)
	{
		state.aftertouchNote = note;
		state.afterTouchValue = aftertouchValues[note] = value * INV_127;
		triggerAftertouchChange();
	}
	void setChannelPressure(int value, int /*channel = 1*/)
	{
		state.channelPressure = value * INV_127;
		triggerChannelPressureChange();
	}

	double getSmoothedMidiPitch() { return currentPitchToFreq; }
	double getMasterRate() { return masterRate; }
	double getFrequency() { return currentFrequency; }
	int getNoteOn() { return lastNoteOn; }
	int getNoteOff() { return lastNoteOff; }
	bool areNotesHeld() { return !UserHeldNotes.empty(); }
	int getCurrentChannel() { return currentChannel; }

	/* Midi Note & Velocity */
	vector<int> UserHeldNotes;

	MIDISTATE & getState() { return state; }

	/* Frequency & Tempo */
	enum NoteChangeMode { Legato, OnNote };
	static const vector<String> KeytrackingMenuItems;
	static const vector<String> NoteChangeModeMenuItems;
	virtual void setNoteChangeMode(int v) { noteChangeMode = (NoteChangeMode)v; }

	void setBPM(double v);
	double getBPM() { return bpm; }
	double getBPMFactor() { return bpmFactor; }
	void setPlayheadInfo(juce::AudioPlayHead::CurrentPositionInfo info) { playheadInfo = info; }
	void setMasterRate(double v) { masterRate = v; triggerMasterRateChange(); }

	/* Pitch Glide Handling */
	enum GlideMode { when_doing_legato, always };
	static const vector<String> GlideModeMenuItems;
	void setGlideMode(int mode) { glideMode = (GlideMode)mode; }
	void setGlideSeconds(double v) { pitchSmoother.setSmoothingTime(v*.001); }

	void addMidiSlave(MidiSlave * obj)
	{
		obj->midiMasterPtr = this;
		jassert(std::find(midiSlaves.begin(), midiSlaves.end(), obj) == midiSlaves.end());
		midiSlaves.push_back(obj);
	}

	void handleMidiBufferIterator(MidiBuffer& buffer)
	{
		if (buffer.isEmpty())
			return;

		midiBufferFinished = false;
		midiBuffer = &buffer;
		midiBufferIterator = buffer.findNextSamplePosition(0);		
		midiMessageSamplePosition = (*midiBufferIterator).samplePosition;
		currentMidiSamplePosition = 0;
	}

	void handleMidiMessage(MidiMessage message)
	{
		if (message.isNoteOn())
			noteOn(message.getNoteNumber(), message.getVelocity(), message.getChannel());
		else if (message.isNoteOff())
			noteOff(message.getNoteNumber(), message.getChannel());
		else  if (message.isAllNotesOff())
			allNotesOff();
		else if (message.isController())
			setMidiController(message.getControllerNumber(), message.getControllerValue(), message.getChannel());
		else if (message.isPitchWheel())
			setPitchBend(message.getPitchWheelValue(), message.getChannel());
		else if (message.isAftertouch())
			setAfterTouch(message.getNoteNumber(), message.getAfterTouchValue(), message.getChannel());
		else if (message.isChannelPressure())
			setChannelPressure(message.getChannelPressureValue(), message.getChannel());
	}
	
protected:
	void triggerAllNotesOff()
	{
		for (auto & s : midiSlaves)
			s->triggerAllNotesOff();
	}
	void triggerNoteOn()
	{
		for (auto & s : midiSlaves)
			s->triggerNoteOn();
	}
	void triggerNoteOff()
	{
		for (auto & s : midiSlaves)
			s->triggerNoteOff();
	}
	void triggerNoteChange()
	{
		for (auto & s : midiSlaves)
			s->triggerNoteChange();
	}
	void triggerAttack()
	{
		triggerNoteChange();
		for (auto & s : midiSlaves)
			s->triggerAttack();
	}
	void triggerNoteOnLegato()
	{
		triggerNoteChange();
		for (auto & s : midiSlaves)
			s->triggerNoteOnLegato();
	}
	void triggerNoteOffLegato()
	{
		triggerNoteChange();
		for (auto & s : midiSlaves)
			s->triggerNoteOffLegato();
	}
	void triggerRelease()
	{
		triggerNoteChange();
		for (auto & s : midiSlaves)
			s->triggerRelease();
	}
	void triggerPitchChange()
	{
		for (auto & s : midiSlaves)
			s->triggerPitchChange();
	}
	void triggerBPMChange()
	{
		for (auto & s : midiSlaves)
			s->triggerBPMChange();
	}
	void triggerMasterRateChange()
	{
		for (auto & s : midiSlaves)
			s->triggerMasterRateChange();
	}
	void triggerVelocityChange()
	{
		for (auto & s : midiSlaves)
			s->triggerVelocityChange();
	}
	void triggerMidiControllerChange()
	{
		for (auto & s : midiSlaves)
			s->triggerMidiControllerChange();
	}
	void triggerPitchBendChange()
	{
		for (auto & s : midiSlaves)
			s->triggerPitchBendChange();
	}
	void triggerAftertouchChange()
	{
		for (auto & s : midiSlaves)
			s->triggerAfterTouchChange();
	}
	void triggerChannelPressureChange()
	{
		for (auto & s : midiSlaves)
			s->triggerChannelPressureChange();
	}

	ParamSmoother pitchSmoother;

	vector<MidiSlave *> midiSlaves;

	// data:
	MIDISTATE state;
	juce::AudioPlayHead::CurrentPositionInfo playheadInfo;

	bool noteWasChanged = false;
	double currentFrequencyWithoutBeats = 1;
	bool noteWasNoteOff = false;

	int currentChannel = 1;
	int currentVelocity = 0;
	int lastNoteOn = -1;
	int lastNoteOff = -1;

	double sampleRate = 44100.0;
	bool note_is_tied = false;
	bool release_was_triggered = true;
	bool oneShotIsFinished = true;
	double noteOffset = 0;

	double bpm = -1;
	std::array<double, 128> midiControllerValues{ 0 };
	std::array<double, 128> aftertouchValues{ 0 };

	/* OPTIONS */
	GlideMode glideMode = GlideMode::when_doing_legato;
	NoteChangeMode noteChangeMode = NoteChangeMode::Legato;

	double getBarPhase();
	double getPhaseBasedOnBarsPerCycle(double barsPerCycle);

	int tempoSelection = 0;
	int beatMultiplierOption = 0;

	double bpmFactor = 1.0;
	double masterRate = 1.0;

	double currentFreqMult = 1.0;
	double currentPitchToFreq = 440.0;

	double currentFrequency = 440.0; // final frequency calculation

	/* Midi Buffer handling */
	MidiBuffer* midiBuffer = nullptr;
	MidiBufferIterator midiBufferIterator;
	int midiMessageSamplePosition;
	int currentMidiSamplePosition;
	bool midiBufferFinished = true;
};

//class Voice
//{
//public:
//	Voice(int key, int vel) : key(key), vel(vel) { noteOn(key, vel); }
//	~Voice() = default;
//
//	virtual void noteOn(int key, int vel) = 0;
//	virtual void noteOff(int key, int vel) = 0;
//	virtual double getOutput() const = 0;
//
//	virtual void updatePitchBend(double v) { /*do something with state->pitchBend*/ }
//
//	void endVoice()
//	{
//		manager->removeVoice(this);
//	}
//
//protected:
//	int key;
//	int vel;
//	VoiceState * state;
//	VoiceManager * manager;
//};
//
//class VoiceManager
//{
//public:
//	VoiceManager() = default;
//	~VoiceManager() = default;
//
//	void addVoice(Voice * voice)
//	{
//		voices.push_back(voice);
//	}
//
//	void removeVoice(Voice * voice)
//	{
//		for (auto iter = voices.begin(); iter != voices.end(); ++iter)
//			if (*iter = voice)
//				voices.erase(iter);
//	}
//
//	double getSample()
//	{
//		double out;
//		for (const auto & voice : voices)
//			out += voice->getOutput();
//		return out;
//	}
//
//	vector<Voice *> voices;
//};
//
//class VoiceState
//{
//public:
//	VoiceState() = default;
//	~VoiceState() = default;
//
//	double pitchBend = 0;
//};
