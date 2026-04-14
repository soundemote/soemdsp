#include "BasicMidiPlugin.h"

BasicModule::BasicModule(juce::CriticalSection * lockToUse
	, jura::MetaParameterManager * metaManagerToUse
	, jura::ModulationManager * modManagerToUse)
	:	AudioModuleWithMidiIn(lockToUse, metaManagerToUse, modManagerToUse)
{
	ScopedLock scopedLock(*lock);
}

//double BasicModule::getBarPhase() { return (playheadInfo.ppqPosition - playheadInfo.ppqPositionOfLastBarStart) * 0.25; }

//double BasicModule::getPhaseBasedOnBarsPerCycle(double barsPerCycle)
//{
//	jassert(barsPerCycle > 0);
//
//	if (barsPerCycle <= 1)
//	{
//		return fmod(getBarPhase(), barsPerCycle) / barsPerCycle;
//	}
//	else
//	{
//		int barLengthsPassed = floor(playheadInfo.ppqPositionOfLastBarStart / barsPerCycle);
//		return fmod(getBarPhase() + barLengthsPassed, barsPerCycle);
//	}
//}

String BasicModule::midiNoteToString(double v)
{
	jassert(v >= 0 && v <= 127);
	return NoteNames[(size_t)v];
}

String BasicModule::midiNoteToStringWithNum(double v)
{
	jassert(v >= 0 && v <= 127);
	return NoteNames[(size_t)v]+" ("+String(v, 0)+")";
}

const vector<String> BasicModule::NoteNames
{
	"C-2","C#-2","D-2","D#-2","E-2","F-2","F#-2","G-2","G#-2","A-2","A#-2","B-2",
	"C-1","C#-1","D-1","D#-1","E-1","F-1","F#-1","G-1","G#-1","A-1","A#-1","B-1",
	"C0","C#0","D0","D#0","E0","F0","F#0","G0","G#0","A0","A#0","B0",
	"C1","C#1","D1","D#1","E1","F1","F#1","G1","G#1","A1","A#1","B1",
	"C2","C#2","D2","D#2","E2","F2","F#2","G2","G#2","A2","A#2","B2",
	"C3","C#3","D3","D#3","E3","F3","F#3","G3","G#3","A3","A#3","B3",
	"C4","C#4","D4","D#4","E4","F4","F#4","G4","G#4","A4","A#4","B4",
	"C5","C#5","D5","D#5","E5","F5","F#5","G5","G#5","A5","A#5","B5",
	"C6","C#6","D6","D#6","E6","F6","F#6","G6","G#6","A6","A#6","B6",
	"C7","C#7","D7","D#7","E7","F7","F#7","G7","G#7","A7","A#7","B7",
	"C8","C#8","D8","D#8","E8","F8","F#8","G8"
};

// maybe better to use this for standard: http://www.nortonmusic.com/midi_cc.html
const vector<String> BasicModule::MidiControllerNames
{
	"Bank Sel 0","Modwheel 1","Breath 2","CC 3","Foot Pdl 4",
	"CC 5","CC 6","Volume 7","Balance 8",
	"CC 9","CC 10","Expression 11","CC 12",
	"CC 13","CC 14","CC 15","CC 16",
	"CC 17","CC 18","CC 19","CC 20",
	"CC 21","CC 22","CC 23","CC 24",
	"CC 25","CC 26","CC 27","CC 28",
	"CC 29","CC 30","Bank Sel (f) 31","Modwheel (f) 32",
	"Breath Ctrl (f) 33","CC34 34","Foot Pdl (f) 35","Porta Time (f) 36",
	"Data Entry (f) 37","Volume (f) 38","Balance (f) 39","CC40 40",
	"Pan (f) 41","Expression (f) 42","Effect Ctrl 1 (f) 43","Effect Ctrl 2 (f) 44",
	"CC 45","CC 46","CC 47","CC 48",
	"CC 49","CC 50","CC 51","CC 52",
	"CC 53","CC 54","CC 55","CC 56",
	"CC 57","CC 58","CC 59","CC 60",
	"CC 61","CC 62","CC 63","Sustain Pdl 64",
	"Portamento 65","Sustenuto 66","Soft Pdl 67","Legato Sw 68",
	"Hold 2 Pdl 69","Variation 70","Timbre 71","Release Time 72",
	"Attack Time 73","Brightness 74","CC 75","CC 76",
	"CC 77","CC 78","CC 79","CC 80",
	"CC 81","CC 82","CC 83","Porta Ctrl 84",
	"CC 85","CC 86","CC 87","CC 88",
	"CC 89","CC 90","Reverb Level 91","Tremolo Level 92",
	"Chorus Level 93","Celeste Level 94","Phaser Level 95","CC 96",
	"CC 97","CC 98","CC 99","CC 100",
	"CC 101","CC 102","CC 103","CC 104",
	"CC 105","CC 106","CC 107","CC 108",
	"CC 109","CC 110","CC 111","CC 112",
	"CC 113","CC 114","CC 115","CC 116",
	"CC 117","CC 118","CC 119","All Sounds Off 120",
	"All Controllers Off 121","CC 122","All Notes Off 123","CC 124",
	"CC 125","CC 126","CC 127"
};
