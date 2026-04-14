#include "ChaosArpCore.h"

TriSawOscillatorCore::TriSawOscillatorCore()
{
}

void TriSawOscillatorCore::reset() { phasor.reset(); }

void TriSawOscillatorCore::setPhaseOffset(double v) { phasor.setPhaseOffset(v); }

void TriSawOscillatorCore::setAttackBend(double v) { atkbend = v; }

void TriSawOscillatorCore::setDecayBend(double v) { decbend = v; }

void TriSawOscillatorCore::setAttackSigmoid(double v)
{
	if (v < 0)
		v *= 2;
	atksig = v;
}

void TriSawOscillatorCore::setDecaySigmoid(double v)
{
	if (v < 0)
		v *= 2;
	decsig = v;
}

void NoteAndPitchQuantizer::setSampleRate(double v)
{
	noteChangeTimer.setSampleRate(v);
	stepChangeTimer.setSampleRate(v);
	noteLengthTimer.setSampleRate(v);
	fullResetTimer.setSampleRate(v);
}

void NoteAndPitchQuantizer::reset()
{
	noteChangeTimer.reset();
	stepChangeTimer.reset();
	noteLengthTimer.reset();
	fullResetTimer.reset();
}

double NoteAndPitchQuantizer::getQuantizedPitch()
{
	return currentNote;
}

void NoteAndPitchQuantizer::setPitchValue(double v) { pitchValue = v; }

void NoteAndPitchQuantizer::setOctaveAmplitude(double v)
{
	octaveAmplitude = v;
}

void NoteAndPitchQuantizer::setPitchOffset(double v)
{
	pitchOffset = v;
}

void NoteAndPitchQuantizer::setStepLengthBars(double v)
{
	stepChangeBars = v;
	recalculateStepChangeFrequency();
}
void NoteAndPitchQuantizer::setNoteChangeBars(double v)
{
	noteChangeBars = v;
	recalculateNoteChangeFrequency();
}

void NoteAndPitchQuantizer::setNoteLengthBars(double v)
{
	noteLengthBars = v;
	recalculateNoteLengthFrequency();
}

void NoteAndPitchQuantizer::addEnabledNote(int v)
{
	enabledNotesSorted.addSorted(sorter, v);
	enabledNotesUnsorted.add(v);
}

void NoteAndPitchQuantizer::removeEnabledNote(int v)
{
	enabledNotesSorted.removeFirstMatchingValue(v);
	enabledNotesUnsorted.removeFirstMatchingValue(v);
}

void NoteAndPitchQuantizer::removeAllEnabledNotes(int v)
{
	enabledNotesSorted.clear();
	enabledNotesUnsorted.clear();
}

double NoteAndPitchQuantizer::convertBeatsToFrequency(double BPM, double beats)
{
	return BPM/120.0 * 2.0/beats;
}

double NoteAndPitchQuantizer::convertBarsToFrequency(double BPM, double bars)
{
	return BPM/120.0 * 0.5/bars;
}

void NoteAndPitchQuantizer::recalculatePitch()
{
	if (enabledNotesSorted.isEmpty())
		return;

	double actualPitchValue = pitchValue * octaveAmplitude;

	int numHeldNotes = enabledNotesSorted.size();
	int octaveModifier = floor(actualPitchValue) * 12;
	int noteIndex = actualPitchValue * (double)numHeldNotes;

	int noteIndexPreventError = fmod(abs(noteIndex), numHeldNotes);

	currentNote = asPlayedEnabled ? enabledNotesUnsorted[noteIndexPreventError] : enabledNotesSorted[noteIndexPreventError];
	currentNote += pitchOffset + octaveModifier;
}

void NoteAndPitchQuantizer::recalculateNoteChangeFrequency()
{
	noteChangeTimer.setFrequency(convertBarsToFrequency(BPM, noteChangeBars)*resetSpeedMultiplier);
}

void NoteAndPitchQuantizer::recalculateNoteLengthFrequency()
{
	noteLengthTimer.setFrequency(convertBarsToFrequency(BPM, noteLengthBars*noteLenghtMod)*resetSpeedMultiplier);
}

void NoteAndPitchQuantizer::recalculateStepChangeFrequency()
{
	stepChangeTimer.setFrequency(convertBarsToFrequency(BPM, stepChangeBars)*resetSpeedMultiplier);
}

void NoteAndPitchQuantizer::recalculateFullResetFrequency()
{
	fullResetTimer.setFrequency(convertBeatsToFrequency(BPM, fullResetBeats)/**resetSpeedMultiplier*/);
}
