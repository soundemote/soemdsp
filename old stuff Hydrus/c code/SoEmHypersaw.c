/* 1. BOILER PLATE */

// tcc -Os -ffunction-sections -fdata-sections
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <windows.h>
#include <math.h>

#define __GNUC__
#define LOGFILE "Plugin_Synth.log"

#include "aeffect.h"
#include "aeffectx.h"

#include "../library/DBG.h"
#include "../library/math.h"
#include "../library/globals.h"

__declspec(dllexport) struct AEffect* VSTPluginMain(audioMasterCallback audioMaster);

typedef struct VstEvents VstEvents;
typedef struct VstEvent VstEvent;
typedef struct VstMidiEvent VstMidiEvent;
typedef struct VstParameterProperties VstParameterProperties;
typedef struct VstTimeInfo VstTimeInfo;

/* 2. #DEFINE CONSTANTS, COMPILED CONSTANTS, AND PARAMETER ENUMS */

#define MASTER_SEED 696969 // this is cum, gamer.
#define HYPERSAW_MAX_OSCILLATORS 64
#define NUM_INPUT_CHANNELS 2
#define NUM_OUTPUT_CHANNELS 2
#define PRESETS_MAX 128

enum DETUNETYPE { PRIME, GAUSSIAN, LINEAR };

enum ParameterIndexes
{
	/* GLOBAL */
	parSeed,
	parGlobalVolume,
	parPitchOffset,
	parWaveshape,
	parNumSaws,
	parMorph,
	parReset,
	parMono,
	parRandomPhaseOffset,

	/* VIBRATO */	
	parVibrato_Speed,
	parVibrato_RandomSpeed,
	parVibrato_RandSpdOffset,
	parVibrato_RangeIO,
	parVibrato_RandomRange,
	parVibrato_RandRngOffset,
	parVibrato_Desync,

	/* PORTAMENTO */
	parPortamento_Time,
	parPortamento_Random,
	parNoteDesync,

	/* DETUNE */
	parDetune_Type,
	parDetune_Amount,
	parDetune_Curve,

	/* DRIFT */
	parDriftColor,
	parDriftDetail,
	parDriftAmountIO,
	parDriftLPF,
	parDriftHPF,

	/* ENVELOPE */
	parDelayRandom,
	parAttack,
	parAttackRandom,
	parDecay,
	parDecayRandom,

	NUM_PARAMS
};

/* 3. INCLUDE DSP LIBRARY AND CUSTOM DSP */

#include "../soundemote_dsp_library.h"
#include "../library/Oscillators/Hypersaw.c"

/* 4. START OF SPECIFIC DSP CODE */

struct Voice;
typedef struct Voice
{
	struct Voice* Prev;
	struct Voice* Next;
	bool Active;
	double pitch;
	double frequency;
	double increment;
	double velocityNormalized;

	bool needsIncrementUpdate;
	bool needsMorphUpdate;

	PolyBLEP osc;

	HypersawUnit hypersawUnit[HYPERSAW_MAX_OSCILLATORS];

} Voice;

typedef struct Synth
{
	struct AEffect instance;
	audioMasterCallback Host;
	struct Voice *ActiveVoices;
	struct Voice Voices[128];

	struct Preset Presets[PRESETS_MAX];
	int CurrentPreset;

	double morph;
	Par PARS[NUM_PARAMS];

} Synth;

#include "../PluginIncludes/Hypersaw/updater.h"

void int2string(VstInt32 value, char *string) { sprintf(string, "%d", value); }
void float2string(float value, char *string) { sprintf(string, "%.2f", value); }
VstInt32 float2int(float number) { return (VstInt32)(1000.0f * number); }

void setSampleRate(Synth* o, float sampleRateIn) 
{
	samplerate = sampleRateIn;
	halfsamplerate = samplerate * 0.5;
	sampleperiod = 1.0/sampleRateIn;
	_440_z_sampleRate = 440 * sampleperiod;
}

void setBlockSize(VstInt32 blockSize) { blockSize = blockSize; }

VstInt32 processEvents(Synth* s, VstEvents* ev) 
{
	for (int i = 0; i < ev->numEvents; i++) 
	{
		VstEvent* event = ev->events[i];
		
		if (event->type == kVstMidiType) 
		{
			VstMidiEvent* midiEvent = (VstMidiEvent*) event;
			switch(midiEvent->midiData[0] & 0xF0)
			{
				case 0x80: //type=kMidiNoteOff
				{
					Voice *selectedVoice = &(s->Voices[(int)(midiEvent->midiData[1] & 0x7F)]);
					selectedVoice->velocityNormalized = 0;

					if (selectedVoice->Active)
					{
						selectedVoice->Active = false;
						if (selectedVoice->Next != NULL) selectedVoice->Next->Prev = selectedVoice->Prev;
						if (selectedVoice->Prev != NULL) selectedVoice->Prev->Next = selectedVoice->Next;
						else s->ActiveVoices = selectedVoice->Next;
						selectedVoice->Next = NULL;
						selectedVoice->Prev = NULL;
					}
				}
				break;

				case 0x90: //type=kMidiNoteOn;
				{
					Voice *selectedVoice = &(s->Voices[(int)(midiEvent->midiData[1] & 0x7F)]);
					selectedVoice->velocityNormalized = (midiEvent->midiData[2] & 0x7F) * _1_z_127;

					if (!selectedVoice->Active)
					{
						selectedVoice->Active = true;
						selectedVoice->Next = s->ActiveVoices;
						selectedVoice->Prev = NULL;

						if (s->ActiveVoices != NULL) 
							s->ActiveVoices->Prev = selectedVoice;

						if (selectedVoice->needsIncrementUpdate)
						{
							selectedVoice->needsIncrementUpdate = false;
							updateIncrementSingleVoice(s, selectedVoice);
						}

						// if (selectedVoice->needsMorphUpdate)
						// {
						// 	selectedVoice->needsMorphUpdate = false;
						// 	updateMorphSingleVoice(s, selectedVoice);
						// }

						s->ActiveVoices = selectedVoice;
					}
				}
				break;
				case 0xB0: //type=kMidiControlChange;
					break;
				case 0xC0: //type=kMidiProgramChange;
					break;
				case 0xE0: //type=kMidiPitchWheel;
					break;
				case 0xA0: //type=kMidiNoteAfterTouch;
					break;
				case 0xD0: //type=kMidiChannelAfterTouch;
					break;
			}
		} 
	}
	
	return 0; 
}

bool getVendorString(char* ptr) { strncpy((char*)ptr, "Soundemote", kVstMaxVendorStrLen); return true; }

void getParameterName(Synth* s, VstInt32 index, char* text) 
{
	strncpy((char*)text, (char*)&s->PARS[index].name, kVstMaxLabelLen);
}

void getParameterLabel(Synth* s, VstInt32 index, char *label) 
{
	Par* p = &s->PARS[index];

	float val = map0to1(p->normVal, p->min, p->max);

	switch(index)
	{
	default:
	 	float2string(val, label);
		return;
	}
}

void getParameterDisplay (Synth* s, VstInt32 index, char* text) 
{
	switch(index)
	{
	case parGlobalVolume:
		strcpy((char*)text, "dB");
	default:
	 	*text = 0;
		return;
	}
}

bool getParameterProperties(VstInt32 index, VstParameterProperties* p) { return false; }

void sendVstEventsToHost(Synth* effect, VstEvents* events)
{
	if (effect->Host) 
		(effect->Host) (effect, audioMasterProcessEvents, 0, 0, events, 0);
}

VstTimeInfo* getTimeInfo(Synth* effect, VstInt32 filter)
{
	if (effect->Host) 
		return (VstTimeInfo*) (effect->Host) (effect, audioMasterGetTime, 0, filter, 0, 0);

	return 0;
}

void process32(Synth* s, float** inputs, float** outputs, VstInt32 sampleframes)
{
	float* oleft = outputs[0]; 
	float* oright = outputs[1];
	Voice *iterVoice;
	double out;

	while (--sampleframes >= 0)
	{
		iterVoice = s->ActiveVoices;

		while (iterVoice != NULL)
		{					
			out = PolyBLEP_getSample(&iterVoice->osc) * s->PARS[parGlobalVolume].storedVal;

			*oleft  += out;
			*oright += out;

			iterVoice = iterVoice->Next;
		}
		++oleft;
		++oright;
	}
}

void setparam(Synth* s, VstInt32 index, float value)
{
	Voice *iterVoice= s->ActiveVoices;
	Par* p = &s->PARS[index];
	p->normVal = value;
	p->val = map0to1(p->normVal, p->min, p->max);

	s->Presets[s->CurrentPreset].value[index] = value;

	switch(index)
	{
	case parSeed:
		break;
	case parGlobalVolume:
		p->storedVal = dbToAmpAccurate(p->val);
	case parPitchOffset:
		flagAllVoicesForIncrementUpdate(s);
		updateIncrementActiveVoices(s);
	case parWaveshape:
		break;
	case parNumSaws:
		break;
	case parMorph:
		// flagAllVoicesForMorphUpdate(s);
		// updateMorphActiveVoices(s);		
		break;
	case parReset:
		break;
	case parMono:
		break;
	case parRandomPhaseOffset:
		break;
	case parVibrato_Speed:
		break;
	case parVibrato_RandomSpeed:
		break;
	case parVibrato_RandSpdOffset:
		break;
	case parVibrato_RangeIO:
		break;
	case parVibrato_RandomRange:
		break;
	case parVibrato_RandRngOffset:
		break;
	case parVibrato_Desync:
		break;
	case parPortamento_Time:
		break;
	case parPortamento_Random:
		break;
	case parNoteDesync:
		break;
	case parDetune_Type:
		break;
	case parDetune_Amount:
		break;
	case parDetune_Curve:
		break;
	case parDriftColor:
		break;
	case parDriftDetail:
		break;
	case parDriftAmountIO:
		break;
	case parDriftLPF:
		break;
	case parDriftHPF:
		break;
	case parDelayRandom:
		break;
	case parAttack:
		break;
	case parAttackRandom:
		break;
	case parDecay:
		break;
	case parDecayRandom:
		break;
	default:
		break;
	}
}

float getparam(Synth* s, VstInt32 index) 
{
	Par* p = &s->PARS[index];

	switch(index)
	{
	default:
		return p->normVal;
	}
	return 0;
	
}

void setProgram(Synth* s, VstInt32 index)
{ 
	s->CurrentPreset = index;
	for (int i = 0; i < NUM_PARAMS; ++i) 	
		setparam(s, i, s->Presets[index].value[i]);	
}

VstIntPtr Dispatcher(AEffect* effect, VstInt32 opcode, VstInt32 index, VstIntPtr value, void* ptr, float opt)
{	
	VstIntPtr result = 0;
	switch (opcode)
	{
		case effSetSampleRate:
			setSampleRate((Synth*)effect, opt);
			break;
		case effSetBlockSize:
			setBlockSize((VstInt32)value);
			break;
		case effProcessEvents:
			result = processEvents(effect, (VstEvents*)ptr);	
			break;
		case effGetPlugCategory:
			result = kPlugCategSynth;
			break;
		case effGetVendorString:
			result = getVendorString(ptr);
			break;
		case effGetEffectName: 
			strcpy((char*)ptr, "SoEm Hypersaw v0.1");
			break;
		case effGetProductString:
			strcpy((char*)ptr, "Hypersaw component to the the Hypersaw Symphony");
			break;
		case effGetParamName:
			getParameterName((Synth*)effect, index, (char*)ptr);
			break;
		case effGetParamDisplay:
			getParameterDisplay((Synth*)effect, index, (char*)ptr);
			break;
		case effGetParamLabel:
			getParameterLabel(effect, index, (char*)ptr);
			break;
		case effGetParameterProperties:
			result = getParameterProperties(index, (VstParameterProperties*)ptr);
			break;
		case effClose:
			{		
			Synth* s = (Synth*)effect;
			free(s);
			//free(ptr);
			result = 1;
			break;
			}
		case effGetProgramName:
			{
				Synth* s = (Synth*)effect;
				strncpy((char*)ptr,(char*) &s->Presets[s->CurrentPreset].name, kVstMaxProgNameLen);
			}
			break;
		case effGetProgramNameIndexed:
			{
				if (index < PRESETS_MAX) 
				{ 
					Synth* s = (Synth*)effect;
					strncpy((char*)ptr,(char*) &s->Presets[index].name, kVstMaxProgNameLen);
					result = 0;
				}
			}
			break;
		case effGetProgram:
			{
				Synth* s = (Synth*)effect;
				return s->CurrentPreset;
			}
			break;
		case effSetProgram:
			{
				Synth* s = (Synth*)effect;
				setProgram(s, value);
			}
			break;
		default:
			break;
	} 

	return result;
}

AEffect* VSTPluginMain(audioMasterCallback audioMaster)
{
	AEffect* effect = (AEffect*) malloc(sizeof(Synth));
	memset(effect, 0, sizeof(Synth));
	effect->magic = kEffectMagic;
	effect->dispatcher = &Dispatcher;
	effect->setParameter = &setparam;
	effect->getParameter = &getparam;
	effect->numParams = NUM_PARAMS;
	effect->numInputs = NUM_INPUT_CHANNELS;
	effect->numOutputs = NUM_OUTPUT_CHANNELS;
	effect->flags = effFlagsCanReplacing | effFlagsIsSynth;
	effect->processReplacing = &process32;
	effect->uniqueID = 6253;
	effect->version = 1;
	effect->object = 0;
	effect->numPrograms = PRESETS_MAX;

	Synth* s = (Synth*)effect;
	s->Host = audioMaster;
	s->ActiveVoices = NULL;	

	/* GLOBAL */
	s->PARS[parSeed] = makePar("Seed", 0, 999, 0);
	s->PARS[parGlobalVolume] = makePar("Volume", -70, 0, 0);	
	s->PARS[parPitchOffset]  = makePar("Pitch", -128, 36, 0);
	s->PARS[parWaveshape] = makePar("Waveshape", 0, 0, 0);
	s->PARS[parNumSaws] = makePar("NumSaws", 1, HYPERSAW_MAX_OSCILLATORS, 1);
	s->PARS[parMorph] = makePar("Morph", 0, 1, 0);
	s->PARS[parReset] = makePar("Reset", 0, 1, 0);
	s->PARS[parMono] = makePar("Mono", 0, 1, 0);
	s->PARS[parRandomPhaseOffset] = makePar("RandomPhaseOffset", 0, 1, .1);

	/* DRIFT */
	s->PARS[parDriftColor] = makePar("DriftColor", 0, 1, 0);
	s->PARS[parDriftDetail] = makePar("DriftDetail", 0, 1, 0);
	s->PARS[parDriftAmountIO] = makePar("DriftAmountIO", 100, 1, 0);
	s->PARS[parDriftLPF] = makePar("DriftLPF", 0, 1, 1);
	s->PARS[parDriftHPF] = makePar("DriftHPF", 0, 1, 0);

	/* VIBRATO */	
	s->PARS[parVibrato_Speed] = makePar("Vibrato Speed", 0, 10, 0);
	s->PARS[parVibrato_RandomSpeed] = makePar("Vibrato RandomSpeed", 0, 1, 0);
	s->PARS[parVibrato_RandSpdOffset] = makePar("Vibrato RandSpdOffset", 0, 1, 0);
	s->PARS[parVibrato_RangeIO] = makePar("Vibrato Range [IO]", 0, 12, 0);
	s->PARS[parVibrato_RandomRange] = makePar("Vibrato RandomRange", 0, 10, 0);
	s->PARS[parVibrato_RandRngOffset] = makePar("Vibrato RandRngOffset", 0, 10, 0);
	s->PARS[parVibrato_Desync] = makePar("Vibrato Desync", 0, 1, 0);

	/* PORTAMENTO */
	s->PARS[parPortamento_Time] = makePar("Portamento Time", 0, 3, 0);
	s->PARS[parPortamento_Random] = makePar("Portamento Random", 0, 100, 0);
	s->PARS[parNoteDesync] = makePar("Note Desync", 0, 1, 0);

	/* DETUNE */
	s->PARS[parDetune_Type] = makePar("Detune Type", 0, 1, 0);
	s->PARS[parDetune_Amount] = makePar("Detune Amount", 0, 1, 0);
	s->PARS[parDetune_Curve] = makePar("Detune Curve", -1, 1, 0);

	/* ENVELOPE */
	s->PARS[parDelayRandom] = makePar("DelayRandom", 0, 5, 0);
	s->PARS[parAttack] = makePar("Attack", 0, 5, 0);
	s->PARS[parAttackRandom] = makePar("AttackRandom", 0, 5, 0);
	s->PARS[parDecay] = makePar("Decay", 0, 5, 0);
	s->PARS[parDecayRandom] = makePar("DecayRandom", 0, 5, 0);

	makeDefaultPatch(&s->Presets[0], (Par*)&s->PARS, "initialized");

	for (int i = 1; i < PRESETS_MAX; ++i)
	{
		sprintf((char*) &s->Presets[i].name, "Preset %i", i);
		makeDefaultPatch(&s->Presets[i], (Par*)&s->PARS, (char*) &s->Presets[i].name); 
	}

	for (int i = 0; i < 128; ++i)
    {
		PolyBLEP_init(&s->Voices[i].osc);

		s->Voices[i].pitch = i;
		s->Voices[i].needsIncrementUpdate = false;
		s->Voices[i].needsMorphUpdate = false;

		// point all oscillators increments to a 
		for (int x = 0; x < HYPERSAW_MAX_OSCILLATORS; ++x)
		{
			PolyBLEP_init(&s->Voices[i].hypersawUnit[x].osc);

			s->Voices[i].hypersawUnit[x].osc.v[OSC_increment].dr = &s->Voices[i].increment;
			s->Voices[i].hypersawUnit[x].osc.v[OSC_frequency].dr = &s->Voices[i].frequency;
			s->Voices[i].hypersawUnit[x].osc.v[OSC_morph].dr = &s->morph;	

			s->Voices[i].hypersawUnit[x].lpf.v[FLT_increment].dr = &s->PARS[parDriftLPF].storedVal;
			s->Voices[i].hypersawUnit[x].hpf.v[FLT_increment].dr = &s->PARS[parDriftHPF].storedVal;

			s->Voices[i].hypersawUnit[x].walkPM.lpf.v[FLT_increment].dr = &s->PARS[parDriftLPF].storedVal;
			//s->Voices[i].hypersawUnit[x].walkPM.hpf.v[FLT_increment].dr = &s->PARS[parDriftHPF].storedVal;
		}	

		s->Voices[i].osc.v[OSC_increment].dr = &s->Voices[i].increment;
		s->Voices[i].osc.v[OSC_frequency].dr = &s->Voices[i].frequency;
		s->Voices[i].osc.v[OSC_morph].dr = &s->morph;
	}	

	return effect;
}