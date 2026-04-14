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
	parAmplitudeA,
	parAmplitudeB,
	parStepSizeA,	
	parStepSizeB,
	parLPFrequencyA,
	parHPFrequencyB,
	NUM_PARAMS
};

const int channels = 2;
const int NumParams = NUM_PARAMS;
float sampleRate;
float halfSampleRate;
float _7z8xSampleRate;
double samplePeriod = 0;
double _1_z_127 = 1.0/127.0;
int blockSize;

#include "library/Filters/Lowpass.h"
#include "library/Filters/Highpass.h"
#include "library/Oscillators/NoiseGenerator.h"
#include "library/Modulators/WalterRandomWalk.h"

#include "library/Parameters.h"

enum { MONO, STEREO };

typedef struct Synth
{
	struct AEffect instance;
	audioMasterCallback Host;

    struct Preset Presets[PRESETS_MAX];
	int CurrentPreset;

	DualWalterRandomWalk rwL;
	DualWalterRandomWalk rwR;

	// Lowpass lpfL;
	// Highpass hpfL;

	// Lowpass lpfR;
	// Highpass hpfR;

	double velocityNormalized;
	double pitchOffset;
	int monostereo;
	double sweep;

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
	_7z8xSampleRate = 7.0/8.0*sampleRate;

	Lowpass_setSampleRate(&o->rwL.lpf);
	Highpass_setSampleRate(&o->rwL.hpf);

	// Lowpass_setSampleRate(&o->lpfL);
	// Highpass_setSampleRate(&o->hpfL);

	// Lowpass_setSampleRate(&o->lpfR);
	// Highpass_setSampleRate(&o->hpfR);
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
					s->velocityNormalized = 0;
					break;
				case 0x90: //type=kMidiNoteOn;
					s->velocityNormalized = (midiEvent->midiData[2] & 0x7F) * _1_z_127;
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
	case parLPFrequencyA:
	case parHPFrequencyB:
		strcpy((char*)text, "Hz");
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
	case parLPFrequencyA:
		sprintf(label, "%.3f", s->rwL.lpf.frequency);
		break;
	case parHPFrequencyB:
		sprintf(label, "%.3f", s->rwL.hpf.frequency);
		break;
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
	case parSeed:
		s->rwL.walkA.noise.state = s->rwL.walkA.noise.seed = p->val;
		s->rwL.walkB.noise.state = s->rwL.walkB.noise.seed = p->val+409;
		s->rwR.walkA.noise.state = s->rwR.walkA.noise.seed = p->val+91425;
		s->rwR.walkB.noise.state = s->rwR.walkB.noise.seed = p->val+1468;
		return;
	case parMonoStereo:
		s->monostereo = (int)p->val;
		return;
	case parAmplitudeA:
		s->rwL.ampA = p->val;
		s->rwR.ampA = p->val;
		return;
	case parAmplitudeB:
		s->rwL.ampB = p->val;
		s->rwR.ampB = p->val;
		return;
	case parStepSizeA:
	{
		double powVal = pow(p->val, 4);
		s->rwL.walkA.stepSize = powVal;
		s->rwR.walkA.stepSize = powVal;
		return;
	}
	case parStepSizeB:
	{
		double powVal = pow(p->val, 4);
		s->rwL.walkB.stepSize = powVal;
		s->rwR.walkB.stepSize = powVal;
		return;
	}
	case parLPFrequencyA:
	{
		double p2f = min(_7z8xSampleRate, pitchToFrequency(p->val));
		Lowpass_setFrequency(&s->rwL.lpf, p2f);
		Lowpass_setFrequency(&s->rwR.lpf, p2f);
		return;
	}
	case parHPFrequencyB:
	{
		double p2f =  min(_7z8xSampleRate, pitchToFrequency(p->val));
		Highpass_setFrequency(&s->rwL.hpf, p2f);
		Highpass_setFrequency(&s->rwR.hpf, p2f);
		return;
	}
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
			strcpy((char*)ptr, "SoEm RandomWalk (Walter) v1.0");
			break;
		case effGetProductString:
			strcpy((char*)ptr, "Flexible random and noise generator");
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
		case MONO:
			*oleft = *oright = DualWalterRandomWalk_getSample(&s->rwL);
			break;
		case STEREO:
			*oleft  = DualWalterRandomWalk_getSample(&s->rwL);
			*oright = DualWalterRandomWalk_getSample(&s->rwR);
			break;		
		}

		*oleft *= s->velocityNormalized;
		*oright *= s->velocityNormalized;

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
	effect->uniqueID = 9126;
	effect->version = 1;
	effect->object = 0;
	effect->numPrograms = PRESETS_MAX;

	Synth* s = (Synth*)effect;
	s->Host = audioMaster;

	s->PARS[parSeed]         = makePar("Seed", 0, 9999, 0);
	s->PARS[parMonoStereo]   = makePar("Mono or Stereo?", 0, 1, 1);
	s->PARS[parAmplitudeA]   = makePar("AmplitudeA", 0, 10, .5);
	s->PARS[parAmplitudeB]   = makePar("AmplitudeB", 0, 10, .5);
	s->PARS[parStepSizeA]    = makePar("StepSizeA", 0, 1, .5);
	s->PARS[parStepSizeB]    = makePar("StepSizeB", 0, 1, .5);
	s->PARS[parLPFrequencyA] = makePar("LPF Freq (A)", -150, 150, 150);
	s->PARS[parHPFrequencyB] = makePar("HPF Freq (B)", -150, 150, -150);

	makeDefaultPatch(&s->Presets[0], (Par*)&s->PARS, "initialized");

	for (int i = 1; i < PRESETS_MAX; ++i)
	{
		sprintf((char*) &s->Presets[i].name, "Preset %i", i);
		makeDefaultPatch(&s->Presets[i], (Par*)&s->PARS, (char*) &s->Presets[i].name); 
	}

	DualWalterRandomWalk_init(&s->rwL, 3456);
	DualWalterRandomWalk_init(&s->rwR, 7765);

	return effect;
}