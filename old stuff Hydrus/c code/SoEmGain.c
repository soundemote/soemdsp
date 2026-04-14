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

__attribute__((dllexport)) struct AEffect* VSTPluginMain(audioMasterCallback audioMaster);

typedef struct VstEvents VstEvents;
typedef struct VstEvent VstEvent;
typedef struct VstMidiEvent VstMidiEvent;
typedef struct VstParameterProperties VstParameterProperties;
typedef struct VstTimeInfo VstTimeInfo;

#define PRESETS_MAX 128

enum ParameterIndexes
{
	parAmplitudeFine,
	parAmplitude,
	parAmplitudeCoarse,
	parDecibelsFine,
	parDecibels,
	parDecibelsCoarse,
	// parDummyReadoutAmplitude,
	// parDummyReadoutDecibels,
	NUM_PARAMS
};

const int channels = 2;
const int NumParams = NUM_PARAMS;
float sampleRate;
float halfSampleRate;
double samplePeriod = 0;
int blockSize;

#include "library/Parameters.h"

#include "library/Utility/LinearSmoother.h"

typedef struct Synth
{
	struct AEffect instance;
	audioMasterCallback Host;

    struct Preset Presets[PRESETS_MAX];
	int CurrentPreset;

	double totalAmplitude;
	double totalDecibels;
	double finalAmplitude;
    
    LinearSmoother smoother;

	Par PARS[NUM_PARAMS];

} Synth;

void int2string(VstInt32 value, char *string) { sprintf(string, "%d", value); }
void float2string(float value, char *string) { sprintf(string, "%.2f", value); }
VstInt32 float2int(float number) { return (VstInt32)(1000.0f * number); }

void setSampleRate(Synth* o, float sampleRateIn) 
{
    sampleRate = sampleRateIn;
	halfSampleRate = sampleRate * 0.5;
	samplePeriod = 1.0 / sampleRate;

    LinearSmoother_setSampleRate(&o->smoother);
}

void setBlockSize(Synth* o, VstInt32 blockSize) 
{
    LinearSmoother_setFrequency(&o->smoother, (double)blockSize);
}

VstInt32 processEvents(Synth* s, VstEvents* ev) 
{
	return 0; 
}

bool getVendorString(char* ptr) { strncpy((char*)ptr, "Soundemote", kVstMaxVendorStrLen); return true; }

void getParameterName(Synth* s, VstInt32 index, char* text) 
{
	strncpy((char*)text, (const char*)&s->PARS[index].name, kVstMaxLabelLen);
}

void getParameterDisplay (Synth* effect, VstInt32 index, char* text) 
{
	switch(index)
	{
	//case parDummyReadoutDecibels:
	case parDecibelsFine:
	case parDecibels:
    case parDecibelsCoarse:
        strcpy(text, "dB");
        break;
	default:
	 	*text = 0;
		return;
	}
}

bool getParameterProperties(VstInt32 index, VstParameterProperties* p) { return false; }

void getParameterLabel(Synth* s, VstInt32 index, char *label) 
{
	Par* p = &s->PARS[index];

	float convertedValue = map0to1(p->normVal, p->min, p->max);

	switch(index)
	{
	// case parDummyReadoutDecibels:
	// 	sprintf(label, "%2.f", s->finalAmplitude);
	// 	break;
	// case parDummyReadoutAmplitude:
	// 	sprintf(label, "%3.f", ampTodb(s->finalAmplitude));
	// 	break;
	default:
	 	float2string(convertedValue, label);
		return;
	}
}

void setparam(Synth* s, VstInt32 index, float value) 
{
	Par* p = &s->PARS[index];
	p->normVal = value;
	p->val = map0to1(p->normVal, p->min, p->max);

	s->Presets[s->CurrentPreset].value[index] = value;

	switch(index)
	{    
	case parAmplitudeFine:
	case parAmplitude:
	case parAmplitudeCoarse:
	case parDecibelsFine:
	case parDecibels:
	case parDecibelsCoarse:
		s->totalAmplitude = (s->PARS[parAmplitudeCoarse].val + s->PARS[parAmplitude].val + s->PARS[parAmplitudeFine].val);
		s->totalDecibels = s->PARS[parDecibelsCoarse].val + s->PARS[parDecibels].val + s->PARS[parDecibelsFine].val;
        s->finalAmplitude = s->totalAmplitude * dbToAmpAccurate(s->totalDecibels);
		// setparam(s, parDummyReadoutAmplitude, s->finalAmplitude);
		// setparam(s, parDummyReadoutDecibels, ampTodb(s->finalAmplitude));
	default:
		return;
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
			setBlockSize((Synth*)effect, (VstInt32)value);
			break;
		case effProcessEvents:
			result = processEvents(effect, (VstEvents*)ptr);	
			break;
		case effGetPlugCategory:
			result = kPlugCategEffect;
			break;
		case effGetVendorString:
			result = getVendorString(ptr);
			break;
		case effGetEffectName:
			strcpy((char*)ptr, "SoEm Gain v1.0");
			break;
		case effGetProductString:
			strcpy((char*)ptr, "Utility gain and volume plugin.");
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
			free(effect);
			free(ptr);
			break;
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

void sendVstEventsToHost(Synth* effect, VstEvents* events)
{
	if (effect->Host) (effect->Host) (effect, audioMasterProcessEvents, 0, 0, events, 0);
}

VstTimeInfo* getTimeInfo(Synth* effect, VstInt32 filter)
{
	if (effect->Host) return (VstTimeInfo*) (effect->Host) (effect, audioMasterGetTime, 0, filter, 0, 0);
	return 0;
}

void process32(Synth* s, float** inputs, float** outputs, VstInt32 sampleframes)
{
    float* ileft = inputs[0];
    float* iright = inputs[1];
	float* oleft = outputs[0]; 
	float* oright = outputs[1];	
	
	while (--sampleframes >= 0)
	{

        double smoothedGain = LinearSmoother_getSample(&s->smoother, s->finalAmplitude);

		*oleft = smoothedGain * *ileft;
		*oright = smoothedGain * *iright;	

        ++ileft;
        ++iright;
		++oleft; 
		++oright;		
	}
}

AEffect* VSTPluginMain(audioMasterCallback audioMaster)
{
	AEffect* effect = (AEffect*) malloc(sizeof(Synth));
	memset(effect, 0, sizeof(Synth));
	effect->magic = kEffectMagic;
	effect->dispatcher = &Dispatcher;
	effect->setParameter = &setparam;
	effect->getParameter = &getparam;
	effect->numParams = NumParams;
	effect->numInputs = channels;
	effect->numOutputs = channels;
	effect->flags = effFlagsCanReplacing;
	effect->processReplacing = &process32;
	effect->uniqueID = 3417;
	effect->version = 1;
	effect->object = 0;
	effect->numPrograms = PRESETS_MAX;

	Synth* s = (Synth*)effect;
	s->Host = audioMaster;
	
	s->PARS[parAmplitudeFine]   = makePar("Amplitude 0-2", 0, 2, 1);
	s->PARS[parAmplitude]       = makePar("Amplitude 0-10", 0, 10, 0);
	s->PARS[parAmplitudeCoarse] = makePar("Amplitude 0-100", 0, 100, 0);
	s->PARS[parDecibelsFine]    = makePar("Decibels +/-6", -6, 6, 0);
	s->PARS[parDecibels]        = makePar("Decibels +/-10", -10, 10, 0);
	s->PARS[parDecibelsCoarse]  = makePar("Decibels +/-100", -100, 100, 0);
	// s->PARS[parDummyReadoutAmplitude] = makePar("Total in amplitude", 0, 0, 0);
	// s->PARS[parDummyReadoutDecibels] = makePar("Total in dB", 0, 0, 0);

	makeDefaultPatch(&s->Presets[0], (Par*)&s->PARS, "initialized");

	for (int i = 1; i < PRESETS_MAX; ++i)
	{
		sprintf((char*) &s->Presets[i].name, "Preset %i", i);
		makeDefaultPatch(&s->Presets[i], (Par*)&s->PARS, (char*) &s->Presets[i].name); 
	}

    LinearSmoother_init(&s->smoother);

	return effect;
}