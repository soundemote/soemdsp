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

__declspec(dllexport) struct AEffect* VSTPluginMain(audioMasterCallback audioMaster);

typedef struct VstEvents VstEvents;
typedef struct VstEvent VstEvent;
typedef struct VstMidiEvent VstMidiEvent;
typedef struct VstParameterProperties VstParameterProperties;
typedef struct VstTimeInfo VstTimeInfo;

#define MASTER_SEED 2433003
#define PRESETS_MAX 128

enum ENVSTATE { OFF, DELAY, ATTACK, DECAY, SUSTAIN, RELEASE };

enum OUTPUTMODE { CH_STEREO, CH_MONO, CH_LEFT, CH_RIGHT };

enum ModulationFilters
{
	LPFa2am = 0,
	LPFa2pt,
	LPFa2fr,
	LPFa2ph,
	LPFa2mp,
	LPFb2am,
	LPFb2pt,
	LPFb2fr,
	LPFb2ph,
	LPFb2mp,

	HPFa2am = 0,
	HPFa2pt,
	HPFa2fr,
	HPFa2ph,
	HPFa2mp,
	HPFb2am,
	HPFb2pt,
	HPFb2fr,
	HPFb2ph,
	HPFb2mp
};

enum ParameterIndexes
{
	parGlobalVolume,
	parStereoMode,
	parGlobalPitchOffset,

	parGlobalMod,		
	parGlobalModA,
	parGlobalModB,

	parWaveformA,
	parWaveformB,

	parBothFilterpitch,
	parAllLPFpitch,
	parAllHPFpitch,

	parAmplitudeA,
	parAmplitudeB,
	parA2Am,
	parB2Am,
	parLPFAm,
	parHPFAm,

	parFrequencyOffsetA,
	parFrequencyOffsetB,
	parA2Fr,
	parB2Fr,
	parLPFFr,
	parHPFFr,

	parPitchOffsetA,
	parPitchOffsetB,
	parA2Pt,
	parB2Pt,
	parLPFPt,
	parHPFPt,

	parPhaseOffsetA,
	parPhaseOffsetB,
	parA2Ph,
	parB2Ph,
	parLPFPh,
	parHPFPh,

	parMorphOffsetA,
	parMorphOffsetB,
	parA2Mp,
	parB2Mp,
	parLPFMp,
	parHPFMp,

	NUM_PARAMS
};

#include "library/DBG.h"
#include "library/math.h"
#include "library/Parameters.h"
#include "library/variable.h"

const int channels = 2;
const double _1_z_127 = 1.0 / 127.0;
const double _2_z_12 = 2.0 / 12;
double sampleRate;
double halfSampleRate;
double samplePeriod = 0;
double _7z8_x_sampleRate;
double TAU_z_sampleRate;
int blockSize;
int initTimes;

#include "library/Filters/Lowpass.h"
#include "library/Filters/Highpass.h"

#include "library/Utility/LinearSmoother.h"

#include "library/Oscillators/PolyBLEP.h"

struct Voice;
typedef struct Voice
{
	struct Voice* Prev;
	struct Voice* Next;
	bool Active;
	double phase;
	double frequencyA;
	double frequencyB;
	int pitch;
	double incrementA;
	double incrementB;
	double morphA;
	double morphB;
	double velocityNormalized;

	PolyBLEP oscA;
	PolyBLEP oscB;

    Highpass hpf[10];
    Lowpass lpf[10];

	double LowpassIncrements[5];
	double HighpassFrequencies[5];

    double b2am;
    double b2pt;
    double b2fr;
    double b2ph;
    double b2mp;

    double a2am;
    double a2pt;
    double a2fr;
    double a2ph;
    double a2mp;
} Voice;

typedef struct Synth
{
	struct AEffect instance;
	audioMasterCallback Host;
	struct Voice *ActiveVoices;	
	int CurrentPreset;

	Par PARS[NUM_PARAMS];
	struct Preset Presets[PRESETS_MAX];
	double pitchIncrementTable[128];
	struct Voice Voices[128];

	double maxOscFrequency;
	double maxOscIncrement;
	double maxFilterFrequency;
	double maxFilterIncrement;

} Synth;

void int2string(VstInt32 value, char *string) { sprintf(string, "%d", value); }
void float2string(float value, char *string) { sprintf(string, "%.2f", value); }
VstInt32 float2int(float number) { return (VstInt32)(1000.0f * number); }

void updateOscAIncrement(Synth* s, Voice* v)
{		
	v->morphA = wrapPhaseBidirectional(s->PARS[parMorphOffsetA].val + v->a2mp - 0.5);
	// SineSquare_morphChanged(&v->oscA);

	double pitch = v->pitch + s->PARS[parPitchOffsetA].val + s->PARS[parGlobalPitchOffset].val;
	v->frequencyA = min(s->maxOscFrequency, pitchToFrequency(pitch + pitch * v->a2pt));
	v->incrementA = v->frequencyA * samplePeriod;	
	
	double maxPartials = 0.5 * (v->frequencyA == 0.0 ? 2.0 : halfSampleRate / v->frequencyA);	

	//SineSquare_incrementChanged(&v->oscA, maxPartials);
}

void updateOscBIncrement(Synth* s, Voice* v)
{
	v->morphB = wrapPhaseBidirectional(s->PARS[parMorphOffsetB].val + v->b2mp - 0.5);
	// SineSquare_morphChanged(&v->oscB);

	double pitch = v->pitch + s->PARS[parPitchOffsetB].val + s->PARS[parGlobalPitchOffset].val;
	v->frequencyB = min(s->maxOscFrequency, pitchToFrequency(pitch + pitch * v->b2pt));
	v->incrementB = v->frequencyB * samplePeriod;
	
	double maxPartials = 0.5 * (v->frequencyB == 0.0 ? 2.0 : halfSampleRate / v->frequencyB);

	//SineSquare_incrementChanged(&v->oscB, maxPartials);
}

updateLPFIncrementVoice(Synth* s, Voice* v, int filterIdx, int parIdx)
{
	double pitchOffset =  s->PARS[parIdx].val + (v->pitch-64) + s->PARS[parBothFilterpitch].val + s->PARS[parGlobalPitchOffset].val + s->PARS[parAllLPFpitch].val;
	v->LowpassIncrements[filterIdx] = min(s->maxFilterFrequency, pitchToFrequency(pitchOffset)) * samplePeriod;

	double x = exp(-TAU * v->LowpassIncrements[filterIdx]);

	v->lpf[filterIdx].v[FLT_coeff].dw = x;
	v->lpf[filterIdx+5].v[FLT_coeff].dw = x;

	Lowpass_incrementChanged(&v->lpf[filterIdx]); // 0 through 4
	Lowpass_incrementChanged(&v->lpf[filterIdx+5]); // 5 through 9
}

updateHPFIncrementVoice(Synth* s, Voice* v, int filterIdx, int parIdx)
{
	double pitchOffset = s->PARS[parIdx].val + (v->pitch-64) + s->PARS[parBothFilterpitch].val + s->PARS[parGlobalPitchOffset].val + s->PARS[parAllHPFpitch].val;
	v->HighpassFrequencies[filterIdx] = min(s->maxFilterFrequency, pitchToFrequency(pitchOffset));

	Highpass_incrementChanged(&v->hpf[filterIdx]); // 0 through 4
	Highpass_incrementChanged(&v->hpf[filterIdx+5]); // 5 through 9
}

void updateLPFIncrement(Synth* s, int parIdx)
{
	int filterIdx = 0;

	switch(parIdx)
	{
	case parLPFAm:
		filterIdx = 0;
		break;
	case parLPFPt:
		filterIdx = 1;
		break;
	case parLPFFr:
		filterIdx = 2;
		break;
	case parLPFPh:
		filterIdx = 3;
		break;
	case parLPFMp:
		filterIdx = 4;
		break;
	}

	Voice *v = s->ActiveVoices;
	while (v != NULL)
	{
		updateLPFIncrementVoice(s, v, filterIdx, parIdx);
		v = v->Next;
	}
}

void updateHPFIncrement(Synth* s, int parIdx)
{
	int filterIdx = 0;

	switch(parIdx)
	{
	case parHPFAm:
		filterIdx = 0;
		break;
	case parHPFPt:
		filterIdx = 1;
		break;
	case parHPFFr:
		filterIdx = 2;
		break;
	case parHPFPh:
		filterIdx = 3;
		break;
	case parHPFMp:
		filterIdx = 4;
		break;
	}

	Voice *v = s->ActiveVoices;
	while (v != NULL)
	{
		updateHPFIncrementVoice(s, v, filterIdx, parIdx);
		v = v->Next;
	}
}

void setSampleRate(Synth* s, float sampleRateIn) 
{
	_7z8_x_sampleRate = 7.0/8.0*sampleRate;
	sampleRate = sampleRateIn;
	halfSampleRate = sampleRate * 0.5;
	samplePeriod = 1.0/sampleRateIn;
	TAU_z_sampleRate = TAU/sampleRate;

	for (int i = 0; i < 128; ++i)
	{
		s->pitchIncrementTable[i] = pitchToFrequency(i) * samplePeriod;

		for (int x = 0; x < 10; ++x)
		{			
			updateLPFIncrement(s, x);
			updateHPFIncrement(s, x);
		}
	}

	// s->maxOscFrequency    = halfSampleRate;
    // s->maxOscIncrement    = 1.0 / min(maxOscFrequency, halfSampleRate);
	// s->maxFilterFrequency = _7z8_x_sampleRate;
	// s->maxFilterIncrement = 1.0 / maxFilterFrequency;

	s->maxOscFrequency    = sampleRate;
    s->maxOscIncrement    = sampleRate;
	s->maxFilterFrequency = sampleRate;
	s->maxFilterIncrement = sampleRate;
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
					int midiNote = (int)(midiEvent->midiData[1] & 0x7F);
					Voice *selectedVoice = &(s->Voices[midiNote]);
					selectedVoice->velocityNormalized = (midiEvent->midiData[2] & 0x7F) * _1_z_127;

					if (!selectedVoice->Active)
					{
						selectedVoice->Active = true;
						selectedVoice->Next = s->ActiveVoices;
						selectedVoice->Prev = NULL;

						float p = (float) midiEvent->midiData[1];						

						if (s->ActiveVoices != NULL) 
							s->ActiveVoices->Prev = selectedVoice;
						
						updateLPFIncrementVoice(s, selectedVoice, 0, parLPFAm);
						updateLPFIncrementVoice(s, selectedVoice, 1, parLPFPt);
						updateLPFIncrementVoice(s, selectedVoice, 2, parLPFFr);
						updateLPFIncrementVoice(s, selectedVoice, 3, parLPFPh);
						updateLPFIncrementVoice(s, selectedVoice, 4, parLPFMp);

						updateHPFIncrementVoice(s, selectedVoice, 0, parHPFAm);
						updateHPFIncrementVoice(s, selectedVoice, 1, parHPFPt);
						updateHPFIncrementVoice(s, selectedVoice, 2, parHPFFr);
						updateHPFIncrementVoice(s, selectedVoice, 3, parHPFPh);
						updateHPFIncrementVoice(s, selectedVoice, 4, parHPFMp);

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
	strcpy((char*)text, (char*)&s->PARS[index].name);
}

void getParameterLabel(Synth* s, VstInt32 index, char *label) 
{
	Par* p = &s->PARS[index];

	float val = map0to1(p->normVal, p->min, p->max);

	switch(index)
	{
	case parGlobalVolume:
		sprintf(label, "%.2f dB", val);
		return;
	case parWaveformA:
	case parWaveformB:
		switch((int)val)
		{
		case SIN:
			strcpy((char*)label, "SIN");
			return;
		case COS:
			strcpy((char*)label, "COS");
			return;
		case SQUARE:
			strcpy((char*)label, "SQUARE");
			return;
		case PULSE:
			strcpy((char*)label, "PULSE");
			return;
		case PULSE_CENTER:
			strcpy((char*)label, "PULSE_CENTER");
			return;
		case SAW:
			strcpy((char*)label, "SAW");
			return;
		case RAMP:
			strcpy((char*)label, "RAMP");
			return;
		case TRI:
			strcpy((char*)label, "TRI");
			return;
		case TRISAW:
			strcpy((char*)label, "TRISAW");
			return;
		case TRISQUARE:
			strcpy((char*)label, "TRISQUARE");
			return;
		case TRIPULSE:
			strcpy((char*)label, "TRIPULSE");
			return;
		case RECTIFIED_SIN_HALF:
			strcpy((char*)label, "RECTIFIED_SIN_HALF");
			return;
		case RECTIFIED_SIN_FULL: 
			strcpy((char*)label, "RECTIFIED_SIN_FULL");
			return;
		case TRAPEZOID:
			strcpy((char*)label, "TRAPEZOID");
			return;
		}
	case parStereoMode:
		switch((int)val)
		{
		case CH_STEREO:
			sprintf(label, "STEREO");
			return;
		case CH_MONO:
			sprintf(label, "MONO");
			return;
		case CH_LEFT:
			sprintf(label, "LEFT");
			return;
		case CH_RIGHT:
			sprintf(label, "RIGHT");
		}		
		return;
	default:
	 	float2string(val, label);
		return;
	}
}

void getParameterDisplay (Synth* effect, VstInt32 index, char* text) 
{
	switch(index)
	{
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
	Voice *v;

	while (--sampleframes >= 0)
	{
		v = s->ActiveVoices;

		while (v != NULL)
		{		
			updateOscAIncrement(s, v);			

			double oscAval = PolyBLEP_getSample(&v->oscA);

			double outputA = oscAval * s->PARS[parAmplitudeA].val + oscAval * v->a2am;

			double modMultA = oscAval * s->PARS[parGlobalModA].val * s->PARS[parGlobalMod].val;

            v->b2am = Lowpass_getSample(&v->lpf[LPFb2am],  Highpass_getSample(&v->hpf[HPFb2am], modMultA * s->PARS[parB2Am].val));
            v->b2pt = Lowpass_getSample(&v->lpf[LPFb2pt],  Highpass_getSample(&v->hpf[HPFb2pt], modMultA * s->PARS[parB2Pt].val));
            v->b2fr = Lowpass_getSample(&v->lpf[LPFb2fr],  Highpass_getSample(&v->hpf[HPFb2fr], modMultA * s->PARS[parB2Fr].val));
            v->b2ph = Lowpass_getSample(&v->lpf[LPFb2ph],  Highpass_getSample(&v->hpf[HPFb2ph], modMultA * s->PARS[parB2Ph].val));
            v->b2mp = Lowpass_getSample(&v->lpf[LPFb2mp],  Highpass_getSample(&v->hpf[HPFb2mp], modMultA * s->PARS[parB2Mp].val));

			v->b2ph += s->PARS[parPhaseOffsetB].val;
			v->b2fr += s->PARS[parFrequencyOffsetB].val;		
			
			updateOscBIncrement(s, v);

			double oscBval = PolyBLEP_getSample(&v->oscB);

			double outputB = oscBval * s->PARS[parAmplitudeB].val + oscBval * v->b2am;

			double modMultB = oscBval * s->PARS[parGlobalModB].val * s->PARS[parGlobalMod].val;

            v->a2am = Lowpass_getSample(&v->lpf[LPFa2am],  Highpass_getSample(&v->hpf[HPFa2am], modMultB * s->PARS[parA2Am].val));
            v->a2pt = Lowpass_getSample(&v->lpf[LPFa2pt],  Highpass_getSample(&v->hpf[HPFa2pt], modMultB * s->PARS[parA2Pt].val));
            v->a2fr = Lowpass_getSample(&v->lpf[LPFa2fr],  Highpass_getSample(&v->hpf[HPFa2fr], modMultB * s->PARS[parA2Fr].val));
            v->a2ph = Lowpass_getSample(&v->lpf[LPFa2ph],  Highpass_getSample(&v->hpf[HPFa2ph], modMultB * s->PARS[parA2Ph].val));
            v->a2mp = Lowpass_getSample(&v->lpf[LPFa2mp],  Highpass_getSample(&v->hpf[HPFa2mp], modMultB * s->PARS[parA2Mp].val));
            
			v->a2ph += s->PARS[parPhaseOffsetA].val;
			v->a2fr += s->PARS[parFrequencyOffsetA].val;


			switch((int)s->PARS[parStereoMode].val)
			{
				case CH_STEREO:
					*oleft  += outputA * s->PARS[parGlobalVolume].val;
					*oright += outputB * s->PARS[parGlobalVolume].val;
					break;
				case CH_MONO:
					*oleft  += (outputA + outputB) * .75 * s->PARS[parGlobalVolume].val;
					*oright = *oleft;
					break;
				case CH_LEFT:
					*oleft += outputA * s->PARS[parGlobalVolume].val;
					*oright = *oleft;
					break;	
				case CH_RIGHT:
					*oleft  += outputB * s->PARS[parGlobalVolume].val;
					*oright = *oleft;
					break;
			}

			v = v->Next;
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
		p->val = dbToAmpAccurate(map0to1(p->normVal, p->min, p->max));
		break;
	case parLPFAm:
		updateLPFIncrement(s, parLPFAm);
		break;
	case parLPFPt:
		updateLPFIncrement(s, parLPFPt);
		break;
	case parLPFFr:
		updateLPFIncrement(s, parLPFFr);
		break;
	case parLPFPh:
		updateLPFIncrement(s, parLPFPh);
		break;
	case parLPFMp:
		updateLPFIncrement(s, parLPFMp);
		break;
	case parHPFAm:
		updateHPFIncrement(s, parHPFAm);
		break;
	case parHPFPt:
		updateHPFIncrement(s, parHPFPt);
		break;
	case parHPFFr:
		updateHPFIncrement(s, parHPFFr);
		break;
	case parHPFPh:
		updateHPFIncrement(s, parHPFPh);
		break;
	case parHPFMp:
		updateHPFIncrement(s, parHPFMp);
		break;
    case parAllLPFpitch:
		updateLPFIncrement(s, parLPFAm);
		updateLPFIncrement(s, parLPFPt);
		updateLPFIncrement(s, parLPFFr);
		updateLPFIncrement(s, parLPFPh);
		updateLPFIncrement(s, parLPFMp);
		break;
	case parAllHPFpitch:
		updateHPFIncrement(s, parHPFAm);
		updateHPFIncrement(s, parHPFPt);
		updateHPFIncrement(s, parHPFFr);
		updateHPFIncrement(s, parHPFPh);
		updateHPFIncrement(s, parHPFMp);
		break;
	case parBothFilterpitch:
	case parGlobalPitchOffset:
		updateLPFIncrement(s, parLPFAm);
		updateLPFIncrement(s, parLPFPt);
		updateLPFIncrement(s, parLPFFr);
		updateLPFIncrement(s, parLPFPh);
		updateLPFIncrement(s, parLPFMp);
		updateHPFIncrement(s, parHPFAm);
		updateHPFIncrement(s, parHPFPt);
		updateHPFIncrement(s, parHPFFr);
		updateHPFIncrement(s, parHPFPh);
		updateHPFIncrement(s, parHPFMp);
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
			strcpy((char*)ptr, "SoEm Chaos v1.01");
			break;
		case effGetProductString:
			strcpy((char*)ptr, "A unique and straightforward dual-feedback-oscillator chaos generator.");
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
				strcpy((char*)ptr,(char*) &s->Presets[s->CurrentPreset].name);
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
	effect->uniqueID = 8224;
	effect->version = 1;
	effect->object = 0;
	effect->numPrograms = PRESETS_MAX;

	Synth* s = (Synth*)effect;
	s->Host = audioMaster;
	s->ActiveVoices = NULL;	

	int filtersweeprange = 128;

	s->PARS[parStereoMode]        = makePar("Stereo Mode", 0, 3, 0);

	s->PARS[parWaveformA]         = makePar("Waveform A", 0, POLYBLEP_NUM_WAVFORMS - 1, 0);
	s->PARS[parWaveformB]         = makePar("Waveform B", 0, POLYBLEP_NUM_WAVFORMS - 1, 0);
	s->PARS[parGlobalVolume]      = makePar("Volume", -70, 10, 0);
	s->PARS[parGlobalPitchOffset] = makePar("Pitch", -filtersweeprange, filtersweeprange, 0);

	s->PARS[parGlobalMod]         = makePar("Global Mod", 0, 10, 1);	
	s->PARS[parGlobalModA]        = makePar("Mod A", -10, 10, 1);
	s->PARS[parGlobalModB]        = makePar("Mod B", -10, 10, 1);

    s->PARS[parAmplitudeA]        = makePar("Amp A", -1, 1, .5);
	s->PARS[parAmplitudeB]        = makePar("Amp B", -1, 1, .5);

    s->PARS[parPitchOffsetA]      = makePar("Pitch A", -filtersweeprange, filtersweeprange, 0);
	s->PARS[parPitchOffsetB]      = makePar("Pitch B", -filtersweeprange, filtersweeprange, 0);

    s->PARS[parFrequencyOffsetA]  = makePar("Freq A", -.5, .5, 0);
	s->PARS[parFrequencyOffsetB]  = makePar("Freq B", -.5, .5, 0);	

    s->PARS[parPhaseOffsetA]      = makePar("Phase A", -1, 1, 0);
	s->PARS[parPhaseOffsetB]      = makePar("Phase B", -1, 1, 0.25);

    s->PARS[parMorphOffsetA]      = makePar("Morph A", -1, 1, 0);    
    s->PARS[parMorphOffsetB]      = makePar("Morph B", -1, 1, 0);

	s->PARS[parBothFilterpitch]   = makePar("Sweep Filters",   -filtersweeprange, filtersweeprange, 0);
    s->PARS[parAllLPFpitch]       = makePar("Sweep Lowpass",  -filtersweeprange, filtersweeprange, 0);
    s->PARS[parAllHPFpitch]       = makePar("Sweep Highpass", -filtersweeprange, filtersweeprange, 0);	

    s->PARS[parA2Am] = makePar("A Amp Mod", 0, 10, 0);
	s->PARS[parB2Am] = makePar("B Amp Mpd", 0, 10, 0);

    s->PARS[parA2Pt] = makePar("A Pitch Mod", 0, 10, 0);
	s->PARS[parB2Pt] = makePar("B Pitch Mod", 0, 10, 0);

    s->PARS[parA2Fr] = makePar("A Freq Mod", 0, 4, 0);
	s->PARS[parB2Fr] = makePar("B Freq Mod", 0, 4, 0);

    s->PARS[parA2Ph] = makePar("A Phase Mod", 0, 10, 0);
	s->PARS[parB2Ph] = makePar("B Phase Mod", 0, 10, 0);

    s->PARS[parA2Mp] = makePar("A Morph Mod", -30, 30, 0);
    s->PARS[parB2Mp] = makePar("B Morph Mod", -30, 30, 0);

    s->PARS[parLPFAm] = makePar("LPF Amp", -filtersweeprange, filtersweeprange, 0);
	s->PARS[parHPFAm] = makePar("HPF Amp", -filtersweeprange, filtersweeprange, 0);

    s->PARS[parLPFPt] = makePar("LPF Pitch", -filtersweeprange, filtersweeprange, 0);
	s->PARS[parHPFPt] = makePar("HPF Pitch", -filtersweeprange, filtersweeprange, 0);
	
    s->PARS[parLPFFr] = makePar("LPF Freq", -filtersweeprange, filtersweeprange, 0);
	s->PARS[parHPFFr] = makePar("HPF Freq", -filtersweeprange, filtersweeprange, 0);

    s->PARS[parLPFPh] = makePar("LPF Phase", -filtersweeprange, filtersweeprange, 0);
	s->PARS[parHPFPh] = makePar("HPF Phase", -filtersweeprange, filtersweeprange, 0);

    s->PARS[parLPFMp] = makePar("LPF Morph", -filtersweeprange, filtersweeprange, 0);
	s->PARS[parHPFMp] = makePar("HPF Morph", -filtersweeprange, filtersweeprange, 0);

	makeDefaultPatch(&s->Presets[0], (Par*)&s->PARS, "initialized");

	for (int i = 1; i < PRESETS_MAX; ++i)
	{
		sprintf((char*) &s->Presets[i].name, "Preset %i", i);
		makeDefaultPatch(&s->Presets[i], (Par*)&s->PARS, (char*) &s->Presets[i].name); 
	}

	for (int i = 0; i < 128; ++i)
    {
		s->Voices[i].pitch = i;

		PolyBLEP_init(&s->Voices[i].oscA);
		PolyBLEP_init(&s->Voices[i].oscB);

		for (int x = 0; x < 10; ++x)
		{
       		Lowpass_init(&s->Voices[i].lpf[x]);
			Highpass_init(&s->Voices[i].hpf[x]);
		}

		
		s->Voices[i].oscA.v[OSC_waveform].dr   = &s->PARS[parWaveformA].val;
		s->Voices[i].oscB.v[OSC_waveform].dr   = &s->PARS[parWaveformB].val;

		// amplitude is accounted for within the per-sample loop
		// pitch is accounted for within incrementA		
		// phase is accounted for within the per-sample loop
		s->Voices[i].oscA.v[OSC_morph].dr      = &s->Voices[i].morphA;
		s->Voices[i].oscA.v[OSC_increment].dr  = &s->Voices[i].incrementA;
		
		// amplitude modulation is accounted for within the per-sample loop
		// pitch modulation is accounted for within incrementA	
		s->Voices[i].oscA.v[OSC_phase].dmr     = &s->Voices[i].a2ph;
		// morph modulation is accounted for within incrementA
		s->Voices[i].oscA.v[OSC_increment].dmr = &s->Voices[i].a2fr;

		// amplitude is accounted for within the per-sample loop
		// pitch is accounted for within incrementB	
		// phase is accounted for within the per-sample loop
		s->Voices[i].oscB.v[OSC_morph].dr      = &s->Voices[i].morphB;
		s->Voices[i].oscB.v[OSC_increment].dr  = &s->Voices[i].incrementB;		

		// amplitude modulation is accounted for within the per-sample loop
		// pitch modulation is accounted for within incrementB
		
		s->Voices[i].oscB.v[OSC_phase].dmr     = &s->Voices[i].b2ph;
		// morph modulation is accounted for within incrementB
		s->Voices[i].oscB.v[OSC_increment].dmr = &s->Voices[i].b2fr;

		s->Voices[i].lpf[LPFa2am].v[FLT_increment].dr = &s->Voices[i].LowpassIncrements[0];
		s->Voices[i].lpf[LPFa2pt].v[FLT_increment].dr = &s->Voices[i].LowpassIncrements[1];
		s->Voices[i].lpf[LPFa2fr].v[FLT_increment].dr = &s->Voices[i].LowpassIncrements[2];
		s->Voices[i].lpf[LPFa2ph].v[FLT_increment].dr = &s->Voices[i].LowpassIncrements[3];
		s->Voices[i].lpf[LPFa2mp].v[FLT_increment].dr = &s->Voices[i].LowpassIncrements[4];

		s->Voices[i].lpf[LPFb2am].v[FLT_increment].dr = &s->Voices[i].LowpassIncrements[0];
		s->Voices[i].lpf[LPFb2pt].v[FLT_increment].dr = &s->Voices[i].LowpassIncrements[1];
		s->Voices[i].lpf[LPFb2fr].v[FLT_increment].dr = &s->Voices[i].LowpassIncrements[2];
		s->Voices[i].lpf[LPFb2ph].v[FLT_increment].dr = &s->Voices[i].LowpassIncrements[3];
		s->Voices[i].lpf[LPFb2mp].v[FLT_increment].dr = &s->Voices[i].LowpassIncrements[4];

 		s->Voices[i].hpf[HPFa2am].v[FLT_frequency].dr = &s->Voices[i].HighpassFrequencies[0];
		s->Voices[i].hpf[HPFa2pt].v[FLT_frequency].dr = &s->Voices[i].HighpassFrequencies[1];
		s->Voices[i].hpf[HPFa2fr].v[FLT_frequency].dr = &s->Voices[i].HighpassFrequencies[2];
		s->Voices[i].hpf[HPFa2ph].v[FLT_frequency].dr = &s->Voices[i].HighpassFrequencies[3];
		s->Voices[i].hpf[HPFa2mp].v[FLT_frequency].dr = &s->Voices[i].HighpassFrequencies[4];
		
		s->Voices[i].hpf[HPFb2am].v[FLT_frequency].dr = &s->Voices[i].HighpassFrequencies[0];
		s->Voices[i].hpf[HPFb2pt].v[FLT_frequency].dr = &s->Voices[i].HighpassFrequencies[1];
		s->Voices[i].hpf[HPFb2fr].v[FLT_frequency].dr = &s->Voices[i].HighpassFrequencies[2];
		s->Voices[i].hpf[HPFb2ph].v[FLT_frequency].dr = &s->Voices[i].HighpassFrequencies[3];
		s->Voices[i].hpf[HPFb2mp].v[FLT_frequency].dr = &s->Voices[i].HighpassFrequencies[4];
    }

	return effect;
}