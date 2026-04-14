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
	parSeed,
	parMonoStereo,
	parAmplitude,
	parOutputSource,
	parPitchOffset,
	NUM_PARAMS
};

const int channels = 2;
const int NumParams = NUM_PARAMS;
const double _1_z_127 = 1.0/127.0;
float sampleRate;
float halfSampleRate;
double samplePeriod = 0;
int blockSize;

#include "library/Utility/LinearSmoother.h"
#include "library/Oscillators/NoiseGenerator.h"
#include "library/Modulators/SampleAndHold.h"

#include "library/Parameters.h"
#include "library/Filters/Highpass.h"

enum { MONO, STEREO };
enum { NOISE, SAMPLED, SMOOTHED };

typedef struct Synth
{
	struct AEffect instance;
	audioMasterCallback Host;

    struct Preset Presets[PRESETS_MAX];
	int CurrentPreset;

	SampleAndHold snhL;
	SampleAndHold snhR;

	double MASTER_PITCH;
	double MASTER_PITCH_MOD;
	double amplitude;
	double velocityNormalized;
	int outputSource;
	double pitchOffset;
	int monostereo;

	Par PARS[NUM_PARAMS];

} Synth;

void int2string(VstInt32 value, char *string) { sprintf(string, "%d", value); }
void float2string(float value, char *string) { sprintf(string, "%.2f", value); }
VstInt32 float2int(float number) { return (VstInt32)(1000.0f * number); }

void setSampleRate(Synth* o, float sampleRateIn) 
{
	sampleRate = sampleRateIn;
	halfSampleRate = sampleRate * 0.5;
	samplePeriod = 1.0/sampleRateIn;

	SampleAndHold_setSampleRate(&o->snhL);
	SampleAndHold_setSampleRate(&o->snhR);
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
					s->velocityNormalized = 0;
				}
				break;

				case 0x90: //type=kMidiNoteOn;
				{
					s->velocityNormalized = (midiEvent->midiData[2] & 0x7F) * _1_z_127;
					s->MASTER_PITCH_MOD = s->pitchOffset;
					//s->MASTER_PITCH = (int)(midiEvent->midiData[1] & 0x7F);
					double f = pitchToFrequency(/*s->MASTER_PITCH + */s->MASTER_PITCH_MOD);
					SampleAndHold_setFrequency(&s->snhL, f);
					SampleAndHold_setFrequency(&s->snhR, f);
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
	strncpy((char*)text, (const char*)&s->PARS[index].name, kVstMaxLabelLen);
}

void getParameterDisplay (Synth* effect, VstInt32 index, char* text) 
{
	switch(index)
	{
	case parPitchOffset:
		strcpy(text, "Hz");
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
	case parMonoStereo:
		switch ((int)convertedValue)
		{
		case 0:
			strcpy((char*)label, "MONO");
			return;
		case 1:
			strcpy((char*)label, "STEREO");
			return;
		}
	case parOutputSource:
		s->outputSource = (int)convertedValue;
		if (s->outputSource == NOISE)
			sprintf(label, "noise", 0);
		else if (s->outputSource == SAMPLED)
			sprintf(label, "sampled", 0);
		else
			sprintf(label, "smoothed", 0);
		return;
	case parPitchOffset:
		sprintf(label, "%.3f", s->snhL.frequency);
		return;
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
	case parMonoStereo:
		s->monostereo = (int)p->val;
		return;
	case parSeed:
		s->snhL.noise.state = s->snhL.noise.seed = p->val;
		s->snhR.noise.state = s->snhL.noise.seed = p->val+23408;
		return;
	case parAmplitude:
		s->amplitude = p->val;
		return;
	case parOutputSource:
		s->outputSource = (int)p->val;
		return;
	case parPitchOffset:
		s->pitchOffset = p->val;
		s->MASTER_PITCH_MOD = s->pitchOffset;
		double f = min(sampleRate, pitchToFrequency(s->MASTER_PITCH + s->MASTER_PITCH_MOD));
		SampleAndHold_setFrequency(&s->snhL, f);
		SampleAndHold_setFrequency(&s->snhR, f);
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
			setBlockSize((VstInt32)value);
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
			strcpy((char*)ptr, "SoEm SampleAndHold v1.0");
			break;
		case effGetProductString:
			strcpy((char*)ptr, "Sample and Hold with control of sample frequency with noise / sampled / smoothed output.");
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
		switch(s->monostereo)
		{
			FUNC
			LOGI(s->monostereo);
			case MONO:
				switch(s->outputSource)
				{
				case NOISE:
					*oleft  = 
					*oright = SampleAndHold_getNoiseValue(&s->snhR);
					break;
				case SAMPLED:					
					SampleAndHold_increment(&s->snhR);
					*oleft  = 
					*oright = SampleAndHold_getSampledValue(&s->snhR);
					break;		
				case SMOOTHED:
					SampleAndHold_increment(&s->snhR);
					*oleft  = 
					*oright = SampleAndHold_getSmoothedValue(&s->snhR);
					break;
				}
				break;
			case STEREO:
				switch(s->outputSource)
				{
				case NOISE:
					*oleft  = SampleAndHold_getNoiseValue(&s->snhL);
					*oright = SampleAndHold_getNoiseValue(&s->snhR);
					break;
				case SAMPLED:
					SampleAndHold_increment(&s->snhL);
					SampleAndHold_increment(&s->snhR);
					*oleft  = SampleAndHold_getSampledValue(&s->snhL);
					*oright = SampleAndHold_getSampledValue(&s->snhR);
					break;		
				case SMOOTHED:
					SampleAndHold_increment(&s->snhL);
					SampleAndHold_increment(&s->snhR);
					*oleft  = SampleAndHold_getSmoothedValue(&s->snhL);
					*oright = SampleAndHold_getSmoothedValue(&s->snhR);
					break;
				}
				break;
		}
		*oleft *= s->amplitude * s->velocityNormalized;
		*oright *= s->amplitude * s->velocityNormalized;	

		oleft++; 
		oright++;		
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
	effect->flags = effFlagsCanReplacing | effFlagsIsSynth;
	effect->processReplacing = &process32;
	effect->uniqueID = 6253;
	effect->version = 1;
	effect->object = 0;
	effect->numPrograms = PRESETS_MAX;

	Synth* s = (Synth*)effect;
	s->Host = audioMaster;

	s->PARS[parSeed]         = makePar("Seed", 0, 9999, 0);
	s->PARS[parMonoStereo]   = makePar("Mono or Stereo?", 0, 1, 1);
	s->PARS[parAmplitude]    = makePar("Amplitude", 0, 1, .5);
	s->PARS[parOutputSource] = makePar("Output select", 0, 2, 1);
	s->PARS[parPitchOffset]  = makePar("Frequency", -150, 150, 64);

	makeDefaultPatch(&s->Presets[0], (Par*)&s->PARS, "initialized");

	for (int i = 1; i < PRESETS_MAX; ++i)
	{
		sprintf((char*) &s->Presets[i].name, "Preset %i", i);
		makeDefaultPatch(&s->Presets[i], (Par*)&s->PARS, (char*) &s->Presets[i].name); 
	}

	SampleAndHold_init(&s->snhL);
	SampleAndHold_init(&s->snhR);

	return effect;
}

