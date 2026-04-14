#include "MidiMaster.h"

const vector<String> MidiMaster::GlideModeMenuItems{ "Legato", "Always", };
const vector<String> MidiMaster::KeytrackingMenuItems{ "Keytracking: On", "Keytracking: Off" };
const vector<String> MidiMaster::NoteChangeModeMenuItems{ "Change Pitch: Always", "Change Pitch: Note On" };

MIDISTATE & MidiSlave::getMidiState() { return midiMasterPtr->state; }

MidiMaster * MidiSlave::getMidiMasterPtr() { return midiMasterPtr; }

const vector<double> MidiMaster::TempoMultipliers
{
	0.0,
	0.25 * 1 / 4.0, 0.25 * 1 / 3.0, 0.25 * 1 / 2.0, 0.25*1.0,
	1.0 / (4.0*0.75 / 1.0), 0.25*1.5, 0.25*2.0,
	1.0 / (4.0*0.75 / 2.0), 0.25*3.0, 0.25*4.0,
	1.0 / (4.0*0.75 / 4.0), 0.25*6.0, 0.25*8.0,
	1.0 / (4.0*0.75 / 8.0), 0.25*12.0, 0.25*16.0,
	1.0 / (4.0*0.75 / 16.0), 0.25*24.0, 0.25*32.0,
	1.0 / (4.0*0.75 / 32.0), 0.25*48.0, 0.25*64.0,
	1.0 / (4.0*0.75 / 64.0), 0.25*96.0, 0.25*128.0,
};

void MidiMaster::setSampleRate(double v)
{
	pitchSmoother.setSampleRate(sampleRate = v);
}

void MidiMaster::noteOn(int key, int velocity, int channel)
{
	lastNoteOn = key;

	currentChannel = channel;

	if (currentVelocity != velocity)
	{
		currentVelocity = velocity;
		state.velocity = velocity * INV_127;
		triggerVelocityChange();
	}

	// ensure note is not already in the array
	if (std::find(UserHeldNotes.begin(), UserHeldNotes.end(), lastNoteOn) == UserHeldNotes.end())
	{
		UserHeldNotes.push_back(lastNoteOn);
		triggerNoteOn();
	}

	noteWasChanged = lastNoteOn != state.note;
	state.note = UserHeldNotes.back();

	if (UserHeldNotes.size() == 1)
	{
		if (glideMode == GlideMode::always)
			pitchSmoother.setTargetValue(state.note);
		else
			pitchSmoother.setInternalValue(state.note);

		triggerAttack();
	}
	else
	{
		pitchSmoother.setTargetValue(state.note);
		triggerNoteOnLegato();
	}
}

void MidiMaster::noteOff(int key, int /*channel*/)
{
	lastNoteOff = key;

	triggerNoteOff();

	UserHeldNotes.erase(std::remove(UserHeldNotes.begin(), UserHeldNotes.end(), key), UserHeldNotes.end());

	noteWasChanged = key == state.note;

	if (UserHeldNotes.empty())
		triggerRelease();

	else if (noteWasChanged && noteChangeMode != NoteChangeMode::OnNote)
	{
		state.note = UserHeldNotes.back();

		pitchSmoother.setTargetValue((double)state.note);

		triggerPitchChange();
		triggerNoteOffLegato();
	}
}

double MidiMaster::getBarPhase()
{
	return (playheadInfo.ppqPosition - playheadInfo.ppqPositionOfLastBarStart) * 0.25;
}

double MidiMaster::getPhaseBasedOnBarsPerCycle(double barsPerCycle)
{
	jassert(barsPerCycle > 0);

	if (barsPerCycle <= 1)
	{
		return fmod(getBarPhase(), barsPerCycle) / barsPerCycle;
	}
	else
	{
		int barLengthsPassed = int((playheadInfo.ppqPositionOfLastBarStart*0.25) / barsPerCycle);
		return fmod(getBarPhase() + barLengthsPassed, barsPerCycle);
	}
}

void MidiMaster::incrementPitchGlide()
{
	if (state.noteSmoothed != state.note)
	{
		state.noteSmoothed = pitchSmoother.getCurrentValue();
		pitchSmoother.inc();
		for (auto & s : midiSlaves)
			s->triggerPitchChange();
	}
}

void MidiMaster::incrementMidiBufferIterator()
{
	if (midiBufferFinished)
		return;

	while (currentMidiSamplePosition == midiMessageSamplePosition)
	{
		handleMidiMessage((*midiBufferIterator).getMessage());	

		midiBufferFinished = ++midiBufferIterator == midiBuffer->end();

		if (midiBufferFinished)
			return;

		midiMessageSamplePosition = (*midiBufferIterator).samplePosition;		
	}

	++currentMidiSamplePosition;
}

void MidiMaster::setBPM(double v)
{
	if (bpm == v)
		return; // Robin needs to fix this, bpm is sent every sample it seems.

	bpmFactor = (bpm = v) / 60.0;
	for (auto & s : midiSlaves)
		s->triggerBPMChange();
}
