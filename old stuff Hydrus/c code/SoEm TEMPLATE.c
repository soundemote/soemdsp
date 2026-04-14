// tcc -Os -ffunction-sections -fdata-sections
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

#include "library/DBG.h"
#include "library/math.h"
#include "library/variable.h"


__attribute__((dllexport)) struct AEffect* VSTPluginMain(audioMasterCallback audioMaster);

typedef struct VstEvents VstEvents;
typedef struct VstEvent VstEvent;
typedef struct VstMidiEvent VstMidiEvent;
typedef struct VstParameterProperties VstParameterProperties;
typedef struct VstTimeInfo VstTimeInfo;

#define MASTER_SEED 2433003
#define HYPERSAW_MAX_OSCILLATORS 32
#define ADDITIVE_MAX_HARMONICS 512
#define ADDITIVE_MAX_FREQUENCY 20000
#define PRESETS_MAX 128

enum ENVSTATE { OFF, DELAY, ATTACK, DECAY, SUSTAIN, RELEASE };

enum ParameterIndexes
{
	parGlobalVolume,
	parPitchOffset,

	NUM_PARAMS
};

const int channels = 2;
const double _1_z_127 = 1.0 / 127.0;
double _440_z_sampleRate;
float sampleRate;
float halfSampleRate;
double samplePeriod = 0;
int blockSize;
int initTimes;

#include "library/Filters/Lowpass.h"
#include "library/Filters/Highpass.h"
#include "library/Utility/LinearSmoother.h"

#include "library/Modulators/LinearEnvelope.h"
#include "library/Oscillators/NoiseGenerator.h"
#include "library/Modulators/SampleAndHold.h"
#include "library/Modulators/RandomWalk.h"

#include "library/Oscillators/SineSaw.h"
#include "library/Oscillators/SineSquare.h"
#include "library/Oscillators/PolyBLEP.h"
#include "library/Oscillators/Ellipse.h"

#include "library/Modulators/VibratoGenerator.h"

#include "library/Oscillators/Hypersaw.c"

#include "library/Parameters.h"

struct Voice;
typedef struct Voice
{
	struct Voice* Prev;
	struct Voice* Next;
	bool Active;
	double pitch;
	double increment;
	double velocityNormalized;

	bool needsUpdate;

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

	double masterIncrement;
	double masterLFOAmplitude;
	double masterLFOIncrement;

	Par PARS[NUM_PARAMS];

} Synth;

updateSingleVoice(Synth* s, Voice* v)
{
	v->needsUpdate = false;
	v->increment = samplePeriod * pitchToFrequency(v->pitch + s->PARS[parPitchOffset].val);
	PolyBLEP_incrementChanged(&v->osc);
}

updateActiveVoices(Synth* s)
{
	Voice *iterVoice = s->ActiveVoices;
	while (iterVoice != NULL)
	{
		updateSingleVoice(s, iterVoice);
		iterVoice = iterVoice->Next;
	}
}


void int2string(VstInt32 value, char *string) { sprintf(string, "%d", value); }
void float2string(float value, char *string) { sprintf(string, "%.2f", value); }
VstInt32 float2int(float number) { return (VstInt32)(1000.0f * number); }

void setSampleRate(Synth* o, float sampleRateIn) 
{
	sampleRate = sampleRateIn;
	halfSampleRate = sampleRate * 0.5;
	samplePeriod = 1.0/sampleRateIn;
	_440_z_sampleRate = 440 * samplePeriod;
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

						if (selectedVoice->needsUpdate)
						{
							selectedVoice->needsUpdate = false;
							updateSingleVoice(s, selectedVoice);
						}

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
	case parGlobalVolume:
		p->storedVal = dbToAmpAccurate(p->val);
	case parPitchOffset:
		for (int i = 0; i < 128; ++i)
			s->Voices[i].needsUpdate = true;
		updateActiveVoices(s);
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
	effect->numInputs = channels;
	effect->numOutputs = channels;
	effect->flags = effFlagsCanReplacing | effFlagsIsSynth;
	effect->processReplacing = &process32;
	effect->uniqueID = 6253;
	effect->version = 1;
	effect->object = 0;
	effect->numPrograms = PRESETS_MAX;

	Synth* s = (Synth*)effect;
	s->Host = audioMaster;
	s->ActiveVoices = NULL;	

	s->PARS[parGlobalVolume] = makePar("Volume", -70, 0, 0);	
	s->PARS[parPitchOffset]  = makePar("Pitch", -128, 36, 0);

	makeDefaultPatch(&s->Presets[0], (Par*)&s->PARS, "initialized");

	for (int i = 1; i < PRESETS_MAX; ++i)
	{
		sprintf((char*) &s->Presets[i].name, "Preset %i", i);
		makeDefaultPatch(&s->Presets[i], (Par*)&s->PARS, (char*) &s->Presets[i].name); 
	}

	for (int i = 0; i < 128; ++i)
    {
		s->Voices[i].pitch = i;

		PolyBLEP_init(&s->Voices[i].osc);
		s->Voices[i].osc.incrementPtr = &s->Voices[i].increment;
		s->Voices[i].needsUpdate = false;
	}

	return effect;
}