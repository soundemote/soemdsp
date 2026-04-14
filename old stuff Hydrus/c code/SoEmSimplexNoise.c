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

__declspec(dllexport) struct AEffect* VSTPluginMain(audioMasterCallback audioMaster);

typedef struct VstEvents VstEvents;
typedef struct VstEvent VstEvent;
typedef struct VstMidiEvent VstMidiEvent;
typedef struct VstParameterProperties VstParameterProperties;
typedef struct VstTimeInfo VstTimeInfo;

#define PRESETS_MAX 128

enum ParameterIndexes
{
	parNoiseMode,

	parFrequency,

	parAmpX,
	parAmpY,
	parAmpZ,

	parRotateX,
	parRotateY,
	parRotateZ,	

	NUM_PARAMS
};

#include "../library/globals.h"

#include "../library/variable.h"
#include "../library/Parameters.h"
#include "../library/utility/Rotator3D.h"
#include "../library/modulators/SimplexNoise.h"

const int channels = 2;

typedef struct Synth
{
	struct AEffect instance;
	audioMasterCallback Host;

	struct Rotator3D rotator3d;

	struct Preset Presets[PRESETS_MAX];
	int CurrentPreset;

	double phase;
	double increment;

	double freq;

	double ampX, ampY, ampZ;

	Par PARS[NUM_PARAMS];

} Synth;

void int2string(VstInt32 value, char *string) { sprintf(string, "%d", value); }
void float2string(float value, char *string) { sprintf(string, "%.2f", value); }
VstInt32 float2int(float number) { return (VstInt32)(1000.0f * number); }

void convertPhaseToSimplexPosition(double v) {
	double bi = unipolarToBipolar(v);
	return bi * (bi > 0.0 ? 32767.0 : 32768.0);
}

void setSampleRate(Synth* o, float sampleRateIn) 
{
	samplerate = sampleRateIn;
	halfsamplerate = samplerate * 0.5;
	sampleperiod = 1.0/sampleRateIn;
}

void setBlockSize(VstInt32 blockSize) { blockSize = blockSize; }

VstInt32 processEvents(Synth* effect, VstEvents* ev) 
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
	case parFrequency:
		strncpy(text, "Hz", 4);
		return;
	default:
	 	*text = 0;
		return;
	}
}

bool getParameterProperties(VstInt32 index, VstParameterProperties* p) { return false; }

void getParameterLabel(Synth* s, VstInt32 index, char *label) {
	Par* p = &s->PARS[index];

	float val = map0to1(p->normVal, p->min, p->max);

	switch(index) {
	case parNoiseMode:
		switch ((int)(p->normVal)) {
		case 0:
			sprintf(label, "Simplex");
			return;
		case 1:
			sprintf(label, "FBM");
			return;
		}
	case parFrequency:
		if (p->normVal <= 0)
			int2string(0, label);
		else
			float2string(s->freq, label);
		return;
	default:
	 	float2string(val, label);
		return;
	}
}

void setparam(Synth* s, VstInt32 index, float value) 
{
	Par* p = &s->PARS[index];
	p->normVal = value;
	p->val = map0to1(p->normVal, p->min, p->max);

	s->Presets[s->CurrentPreset].value[index] = value;

	switch(index) {
	case parFrequency:
		s->freq = pitchToFrequency(p->val);
		return;
	case parAmpX:
		s->ampX = p->val;
		return;
	case parAmpY:
		s->ampY = p->val;
		return;
	case parAmpZ:
		s->ampZ = p->val;
		return;
	case parRotateX:
		Rotator3D_setAngleX(&s->rotator3d, p->val);
		Rotator3D_updateCoeffs(&s->rotator3d);
		return;
	case parRotateY:
		Rotator3D_setAngleY(&s->rotator3d, p->val);
		Rotator3D_updateCoeffs(&s->rotator3d);
		return;
	case parRotateZ:
		Rotator3D_setAngleZ(&s->rotator3d, p->val);
		Rotator3D_updateCoeffs(&s->rotator3d);
		return;
	default:
		return;
	}
}

float getparam(Synth* s, VstInt32 index) {
	Par* p = &s->PARS[index];

	switch(index) {
	default:
		return p->normVal;
	}

	return 0;
}

void setProgram(Synth* s, VstInt32 index) { 
	s->CurrentPreset = index;
	for (int i = 0; i < NUM_PARAMS; ++i) 	
		setparam(s, i, s->Presets[index].value[i]);	
}

VstIntPtr Dispatcher(AEffect* effect, VstInt32 opcode, VstInt32 index, VstIntPtr value, void* ptr, float opt) {
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
			strcpy((char*)ptr, "SoEm SiplexNoise v1.0");
			break;
		case effGetProductString:
			strcpy((char*)ptr, "Noise generator based on fractional brownian motion.");
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
	    case effGetProgramName: {
				Synth* s = (Synth*)effect;
				strncpy((char*)ptr,(char*) &s->Presets[s->CurrentPreset].name, kVstMaxProgNameLen);
			}
			break;
		case effGetProgramNameIndexed: {
				if (index < PRESETS_MAX) { 
					Synth* s = (Synth*)effect;
					strncpy((char*)ptr,(char*) &s->Presets[index].name, kVstMaxProgNameLen);
					result = 0;
				}
			}
			break;
		case effGetProgram: {
				Synth* s = (Synth*)effect;
				return s->CurrentPreset;
			}
			break;
		case effSetProgram: {
				Synth* s = (Synth*)effect;
				setProgram(s, value);
			}
			break;
		default:
			break;
	}
	return result;
}

void sendVstEventsToHost(Synth* effect, VstEvents* events) {
	if (effect->Host) (effect->Host) (effect, audioMasterProcessEvents, 0, 0, events, 0);
}

VstTimeInfo* getTimeInfo(Synth* effect, VstInt32 filter) {
	if (effect->Host)
		return (VstTimeInfo*) (effect->Host) (effect, audioMasterGetTime, 0, filter, 0, 0);
	return 0;
}

void process32(Synth* s, float** inputs, float** outputs, VstInt32 sampleframes) {
	float* ileft = inputs[0];
	float* iright = inputs[1];
	float* oleft = outputs[0]; 
	float* oright = outputs[1];

	//Float3 finalAmp = { 0, 0, 0 };
	//Float3 signal3D;
	//Float3 output3D;
	//double phase;
	//Rotator3D_getValues(&rotator3d, finalAmp.x, finalAmp.y, finalAmp.z);
	//finalAmp.x = s->ampX;
	
	while (--sampleframes >= 0)
	{
		//SimplexNoise3D(signal3D);
		//Rotator3D_getSample();
		// 
		//if (&s->PARS[parFrequency].normVal <= 0) {
		//	*oleft = *ileft * s->amplitude;	
		//	*oright = *iright * s->amplitude;	
		//}
		//else {
		//	*oleft = Highpass_getSample(&s->hpfL, *ileft) * s->amplitude;	
		//	*oright = Highpass_getSample(&s->hpfR, *iright) * s->amplitude;	
		//}

		ileft++; 
		iright++;
		oleft++; 
		oright++;		
	}
}

AEffect* VSTPluginMain(audioMasterCallback audioMaster) {
	AEffect* effect = (AEffect*) malloc(sizeof(Synth));

	memset(effect, 0, sizeof(Synth));

	effect->magic = kEffectMagic;
	effect->dispatcher = &Dispatcher;
	effect->setParameter = &setparam;
	effect->getParameter = &getparam;
	effect->numParams = NUM_PARAMS;
	effect->numInputs = channels;
	effect->numOutputs = channels;
	effect->flags = effFlagsCanReplacing;
	effect->processReplacing = &process32;
	effect->uniqueID = 9781;
	effect->version = 1;
	effect->object = 0;
	effect->numPrograms = PRESETS_MAX;

	Synth* s = (Synth*)effect;
	s->Host = audioMaster;

	s->PARS[parNoiseMode] = makePar("Mode", 0, 1, 0);

	s->PARS[parFrequency] = makePar("Frequency", -116, 150, -116);

	s->PARS[parAmpX]      = makePar("Amp X", 0, 1, 1);
	s->PARS[parAmpY]      = makePar("Amp Y", 0, 1, 1);
	s->PARS[parAmpZ]      = makePar("Amp Z", 0, 1, 1);

	s->PARS[parRotateX]   = makePar("Amp X", 0, 1, 0);
	s->PARS[parRotateY]   = makePar("Amp Y", 0, 1, 0);
	s->PARS[parRotateZ]   = makePar("Amp Z", 0, 1, 0);

	makeDefaultPatch(&s->Presets[0], (Par*)&s->PARS, "initialized");

	for (int i = 1; i < PRESETS_MAX; ++i) {
		sprintf((char*) &s->Presets[i].name, "Preset %i", i);
		makeDefaultPatch(&s->Presets[i], (Par*)&s->PARS, (char*) &s->Presets[i].name); 
	}

	Rotator3D_init(&s->rotator3d);

	return effect;
}