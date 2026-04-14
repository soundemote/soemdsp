/*
FEATURES TODO:
- user-controllable limit harmonics via frequency limit OR number of harmonics.
- per-voice rather than per-harmonic randomwalk
- make sure phase controls are working
- proper filters and delay-style filters (more about bringing in harmonics at a certian speed from high low or low high)
- filters that are infinitely continuous, russet
- TILT FILTER!
- pseudo-resonance where a sinewave is dipped and increased as it passes through harmonics
- one curve to go from no harmonic amplitude tapering to sawtooth to filtered sawtooth
- sub-harmonics
- sub-harmonics amplitude tapering
- option to reduce randomwalks either on the highs to low, or lows to high
- ui, envelopes, LFOs
- general volume, pitch, frequency controls good for modulation
- random amplitude 
- random harmonic removal with percentage control
- converge effect for harmonics, 2nd harmonic moves to 1st, 3rd and 5th converge to 4th, etc.
- fm random walk if pm can't do slow and subtle
- per voice morph random walk especially on square pulse width
*/

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

#define MASTER_SEED 2433003
#define HYPERSAW_MAX_OSCILLATORS 32
#define ADDITIVE_MAX_HARMONICS 512
#define ADDITIVE_MAX_FREQUENCY 20000
#define PRESETS_MAX 128

enum ENVSTATE { OFF, DELAY, ATTACK, DECAY, SUSTAIN, RELEASE };

enum ParameterIndexes
{
	parStereoMode,
	parNumHarmonics,
	parAmpCurve,

	parDriftColor,
	parDriftDetail,
	parDriftCurve,
	parDriftAmplitude,	

	parRandomPhase,
	parSkewPhase,
	parSkewPhaseCurve,

	parResetPhaseOnAttack,
	parRandomizePhaseOnAttack,

	parPitchSkew,
	parPitchSkewCurve,
	parPitchRandom,

	parFreqSkew,
	parFreqSkewCurve,
	parFreqRandom,

	parPitchSeed,
	parFreqSeed,

	parWaveform,
	parMorph,
	parAllHarmonicsSameAmp,
	parGlobalVolume,
	parPitchOffset,
	parStereoPhaseOffset,
	parSeed,	

	NUM_PARAMS
};

const int channels = 2;
const double _1_z_127 = 1.0 / 127.0;
float sampleRate;
float halfSampleRate;
double samplePeriod = 0;
int blockSize;
int initTimes;

#include "library/Filters/Lowpass.h"
#include "library/Filters/Highpass.h"
#include "library/Filters/Bandpass.h"

#include "library/Utility/LinearSmoother.h"
#include "library/Modulators/PluckEnvelope.h"
#include "library/Modulators/LinearEnvelope.h"
#include "library/Oscillators/NoiseGenerator.h"
#include "library/Modulators/RandomWalk.h"
#include "library/Modulators/SampleAndHold.h"

#include "library/Oscillators/SineSaw.h"
#include "library/Oscillators/SineSquare.h"
#include "library/Oscillators/PolyBLEP.h"
#include "library/Oscillators/Ellipse.h"

#include "library/Modulators/VibratoGenerator.h"

#include "library/Oscillators/Hypersaw.c"
#include "library/Oscillators/Additive.c"

#include "library/Parameters.h"

struct Voice;
typedef struct Voice
{
	struct Voice* Prev;
	struct Voice* Next;
	bool Active;
	float PhaseAccumulator;
	float Frequency;
	float Pitch;
	double velocityNormalized;

	AdditiveMaster additiveMasterL;
	AdditiveMaster additiveMasterR;

} Voice;

typedef struct Synth
{
	struct AEffect instance;
	audioMasterCallback Host;
	struct Voice *ActiveVoices;
	struct Voice Voices[128];

	struct Preset Presets[PRESETS_MAX];
	int CurrentPreset;

	NoiseGenerator rnd;
	NoiseGenerator rnd2;

	Par PARS[NUM_PARAMS];

	/* ADDITIVE GLOBAL VARIABLES */
	int stereoMode;
	bool randomizePhaseOnAttack;	
	bool resetPhaseOnAttack;
	double randomPhase;
	double randomPhaseAmp;
	double skewPhase;
	double skewPhaseCurve;
	double allHarmonicsSameAmp;
	double globalAmplitude;
	enum WAVEFORM waveform;

} Synth;

void int2string(VstInt32 value, char *string) { sprintf(string, "%d", value); }
void float2string(float value, char *string) { sprintf(string, "%.2f", value); }
VstInt32 float2int(float number) { return (VstInt32)(1000.0f * number); }

void setSampleRate(Synth* o, float sampleRateIn) 
{
	sampleRate = sampleRateIn;
	halfSampleRate = sampleRate * 0.5;
	samplePeriod = 1.0/sampleRateIn;

	for (int i = 0; i < 128; ++i)
	{
		AdditiveMaster_setSampleRate(&o->Voices[i].additiveMasterL);
		AdditiveMaster_setSampleRate(&o->Voices[i].additiveMasterR);
	}
}

void setBlockSize(VstInt32 blockSize) { blockSize = blockSize; }


void Synth_UpdateSingleVoiceIncrement(Synth* s, Voice* v)
{
	int MP = v->additiveMasterL.pitch;
	int n = v->additiveMasterL.numHarmonics;

	double multiplier = 2;
	
	for (int h = 0; h < n; ++h)
	{			
		double p = MP;
		
		p += rationalCurve((double)h/(double)n, s->PARS[parPitchSkewCurve].val) * s->PARS[parPitchSkew].val * 12 * n;
		
		p += v->additiveMasterL.harmonics[h].randomPitch * s->PARS[parPitchRandom].val;

		p += s->PARS[parPitchOffset].val;
		
		double p2f = pitchToFrequency(p);
		
		double f = p2f;
		double h2 = h+1;

		if (n > 1)
		{			
			h2 = map(h2, 1, n, 0, 1);	
			h2 = rationalCurve(h2, s->PARS[parFreqSkewCurve].val);
			f *= map0to1(h2, 1, n);
			f = map0to1(s->PARS[parFreqSkew].val, p2f, f);	
		}

		if (fabs(f) > 22050.0 || f != f)
		{
			f = sampleRate; // will not render
		}
		else
			f += v->additiveMasterL.harmonics[h].randomFreq * s->PARS[parFreqRandom].val;

		v->additiveMasterL.harmonics[h].frequency = f;
		v->additiveMasterR.harmonics[h].frequency = f;

		double inc = f * samplePeriod;

		v->additiveMasterL.harmonics[h].increment = inc;
		v->additiveMasterR.harmonics[h].increment = inc;
	}

	AdditiveMaster_update(&v->additiveMasterL);
	AdditiveMaster_update(&v->additiveMasterR);
}

void Synth_UpdateIncrement(Synth* s)
{		
	Voice *iterVoice= s->ActiveVoices;

	while (iterVoice != NULL)
	{		
		Synth_UpdateSingleVoiceIncrement(s, iterVoice);
		iterVoice = iterVoice->Next;
	}
}

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

						float p = (float) midiEvent->midiData[1];
						selectedVoice->Pitch = p;

						if (s->ActiveVoices != NULL) 
							s->ActiveVoices->Prev = selectedVoice;

						s->ActiveVoices = selectedVoice;

						selectedVoice->additiveMasterL.pitch = p;
						selectedVoice->additiveMasterR.pitch = p;

						int n = howManyHarmonics(pitchToFrequency(p), s->PARS[parNumHarmonics].val, sampleRate, ADDITIVE_MAX_FREQUENCY, ADDITIVE_MAX_HARMONICS);

						FUNC
						LOGF(n);

						AdditiveMaster_setNumHarmonics(&s->ActiveVoices->additiveMasterL, n);
						AdditiveMaster_setNumHarmonics(&s->ActiveVoices->additiveMasterR, n);

						selectedVoice->additiveMasterL.randomPhaseAmp = 
						selectedVoice->additiveMasterR.randomPhaseAmp = s->PARS[parRandomPhase].val;

						selectedVoice->additiveMasterL.ampCurve = 
						selectedVoice->additiveMasterR.ampCurve = s->PARS[parAmpCurve].val;

						selectedVoice->additiveMasterL.waveform = 
						selectedVoice->additiveMasterR.waveform = (int)s->PARS[parWaveform].val;

						selectedVoice->additiveMasterL.morph = 
						selectedVoice->additiveMasterR.morph = s->PARS[parMorph].val;

						selectedVoice->additiveMasterL.allHarmonicsSameAmp = 
						selectedVoice->additiveMasterR.allHarmonicsSameAmp = s->PARS[parAllHarmonicsSameAmp].val >= 1.0 ? true : false;

						Synth_UpdateSingleVoiceIncrement(s, selectedVoice);
						
						if (s->resetPhaseOnAttack)
						{
							for (int i = 0; i < n; ++i)
							{
								selectedVoice->additiveMasterL.harmonics[i].phase =
								selectedVoice->additiveMasterR.harmonics[i].phase = 0;
							}
						}
						
						if (s->randomizePhaseOnAttack)
						{
							for (int i = 0; i < n; ++i)
							{
								selectedVoice->additiveMasterL.harmonics[i].randomPhase = NoiseGenerator_getSample(&s->rnd, 1);
								selectedVoice->additiveMasterR.harmonics[i].randomPhase = NoiseGenerator_getSample(&s->rnd, 1);
							}
						}	
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
	case parStereoMode:
		switch((int)val)
		{
		case MONO:
			strncpy((char*)label, "Mono", kVstMaxLabelLen);
			break;
		case PSEUDOSTEREO:
			strncpy((char*)label, "Pseudostereo", kVstMaxLabelLen);
			break;
		case DOUBLEMONO:
			strncpy((char*)label, "DoubleMono", kVstMaxLabelLen);
			break;
		}
		break;
	case parResetPhaseOnAttack:
		strncpy((char*)label, val < 1 ? "off" : "RESET ON ATTACK", kVstMaxLabelLen);
		break;
	case parRandomizePhaseOnAttack:
		strncpy((char*)label, val < 1 ? "off" : "RANDOMIZE ON ATTACK", kVstMaxLabelLen);
		break;
	case parNumHarmonics:
	 	int2string(val, label);
		return;
	case parWaveform:
		switch((int)val)
			{
			case 0:
				strncpy((char*)label, "SIN", kVstMaxLabelLen);
				break;
			case 1:
				strncpy((char*)label, "SQUARE", kVstMaxLabelLen);
				break;
			case 2:
				strncpy((char*)label, "PULSE", kVstMaxLabelLen);
				break;
			case 3:
				strncpy((char*)label, "PULSE_CENTER", kVstMaxLabelLen);
				break;
			case 4:
				strncpy((char*)label, "SAW", kVstMaxLabelLen);
				break;
			case 5:
				strncpy((char*)label, "RAMP", kVstMaxLabelLen);
				break;
			case 6:
				strncpy((char*)label, "TRI", kVstMaxLabelLen);
				break;
			case 7:
				strncpy((char*)label, "TRISAW", kVstMaxLabelLen);
				break;
			case 8:
				strncpy((char*)label, "TRISQUARE", kVstMaxLabelLen);
				break;
			case 9:
				strncpy((char*)label, "TRIPULSE", kVstMaxLabelLen);
				break;
			case 10:
				strncpy((char*)label, "RECTIFIED_SIN_FULL", kVstMaxLabelLen);
				break;
			}
			break;
	case parAllHarmonicsSameAmp:
		strncpy((char*)label, val >= 1.0 ? "true" : "false", kVstMaxLabelLen);
		break;
	case parGlobalVolume:
		sprintf((char*)label, "%.2f", val);
		break;
	default:
	 	float2string(val, label);
		return;
	}
}

void getParameterDisplay (Synth* effect, VstInt32 index, char* text) 
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
	float toOut;

	while (--sampleframes >= 0)
	{
		iterVoice = s->ActiveVoices;

		while (iterVoice != NULL)
		{					
			switch (s->stereoMode)
			{
			case DOUBLEMONO:		
				*oleft += AdditiveMaster_getSample(&iterVoice->additiveMasterL) * s->globalAmplitude * iterVoice->velocityNormalized;
				*oright += AdditiveMaster_getSample(&iterVoice->additiveMasterR) * s->globalAmplitude * iterVoice->velocityNormalized;
				break;
			default:
				AdditiveMaster_getSample(&iterVoice->additiveMasterL);
				*oleft += iterVoice->additiveMasterL.signalL * s->globalAmplitude;
				*oright += iterVoice->additiveMasterL.signalR * s->globalAmplitude;
				break;
			}
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
	case parNumHarmonics:	
		for (int i = 0; i < 128; ++i)
		{			
			int n = howManyHarmonics(pitchToFrequency(s->Voices[i].Pitch), s->PARS[parNumHarmonics].val, sampleRate, ADDITIVE_MAX_FREQUENCY, ADDITIVE_MAX_HARMONICS);
			AdditiveMaster_setNumHarmonics(&s->Voices[i].additiveMasterL, n);
			AdditiveMaster_setNumHarmonics(&s->Voices[i].additiveMasterR, n);
		}
		Synth_UpdateIncrement(s);
		break;
	case parDriftColor:
		for (int i = 0; i < 128; ++i)
		{
			AdditiveFxDrift_setDriftColor(&s->Voices[i].additiveMasterL.driftFx, p->val);
			AdditiveFxDrift_setDriftColor(&s->Voices[i].additiveMasterR.driftFx, p->val);
		}
		break;
	case parDriftDetail:
		for (int i = 0; i < 128; ++i)
		{
			AdditiveFxDrift_setDriftDetail(&s->Voices[i].additiveMasterL.driftFx, p->val);
			AdditiveFxDrift_setDriftDetail(&s->Voices[i].additiveMasterR.driftFx, p->val);
		}
		break;
	case parDriftCurve:
		for (int i = 0; i < 128; ++i)
		{	
			AdditiveFxDrift_setDriftCurve(&s->Voices[i].additiveMasterL.driftFx, p->val);
			AdditiveFxDrift_setDriftCurve(&s->Voices[i].additiveMasterR.driftFx, p->val);
		}
		break;
	case parDriftAmplitude:
		for (int i = 0; i < 128; ++i)
		{	
			AdditiveFxDrift_setDriftAmplitude(&s->Voices[i].additiveMasterL.driftFx, p->val);
			AdditiveFxDrift_setDriftAmplitude(&s->Voices[i].additiveMasterR.driftFx, p->val);
		}
		break;
	case parStereoMode:
		for (int i = 0; i < 128; ++i)
		{	
			AdditiveMaster_setStereoMode(&s->Voices[i].additiveMasterL, (int)p->val);
			AdditiveMaster_setStereoMode(&s->Voices[i].additiveMasterR, (int)p->val);		
		}
		s->stereoMode = (int)p->val;
		break;
	case parRandomPhase:
		s->randomPhaseAmp = p->val;
		break;
	case parSkewPhaseCurve:
		s->skewPhaseCurve = p->val;
		for (int i = 0; i < 128; ++i)
		{			
			for (int h = 0; h <  s->Voices[i].additiveMasterL.numHarmonics; ++h)
			{
				s->Voices[i].additiveMasterL.harmonics[h].skewPhase = 
				s->Voices[i].additiveMasterR.harmonics[h].skewPhase = rationalCurve((double)h/(double)s->Voices[i].additiveMasterL.numHarmonics, s->skewPhaseCurve) * s->skewPhase;
			}						
		}
		break;
	case parSkewPhase:
		s->skewPhase = p->val;
		for (int i = 0; i < 128; ++i)
		{			
			for (int h = 0; h < ADDITIVE_MAX_HARMONICS; ++h)
			{	
				s->Voices[i].additiveMasterL.harmonics[h].skewPhase = 
				s->Voices[i].additiveMasterR.harmonics[h].skewPhase = rationalCurve((double)h/(double)s->Voices[i].additiveMasterL.numHarmonics, s->skewPhaseCurve) * s->skewPhase;
			}
		}
		break;
	case parResetPhaseOnAttack:
		s->resetPhaseOnAttack = (int)p->val < 1 ? false : true;
		break;
	case parRandomizePhaseOnAttack:
		s->randomizePhaseOnAttack = (int)p->val < 1 ? false : true;
		break;
	case parPitchSkew:		
	case parPitchSkewCurve:
	case parPitchRandom:
	case parFreqRandom:
	case parFreqSkew:
	case parFreqSkewCurve:
		Synth_UpdateIncrement(s);
		break;
	case parAmpCurve:
		for (int i = 0; i < 128; ++i)
		{			
			s->Voices[i].additiveMasterL.ampCurve = 
			s->Voices[i].additiveMasterR.ampCurve = p->val;
		}
		break;
	case parPitchSeed:
		NoiseGenerator_setSeed(&s->rnd2, p->val);
		NoiseGenerator_reset(&s->rnd2);
		for (int i = 0; i < 128; ++i)
		{		
			for (int h = 0; h < ADDITIVE_MAX_HARMONICS; ++h)
			{
				s->Voices[i].additiveMasterL.harmonics[h].randomPitch = 
				s->Voices[i].additiveMasterR.harmonics[h].randomPitch = NoiseGenerator_getSample(&s->rnd2, 1);
			}					
		}
		Synth_UpdateIncrement(s);
		break;
	case parFreqSeed:
		NoiseGenerator_setSeed(&s->rnd2, p->val);
		NoiseGenerator_reset(&s->rnd2);
		for (int i = 0; i < 128; ++i)
		{		
			for (int h = 0; h < ADDITIVE_MAX_HARMONICS; ++h)
			{
				s->Voices[i].additiveMasterL.harmonics[h].randomFreq = 
				s->Voices[i].additiveMasterR.harmonics[h].randomFreq = NoiseGenerator_getSample(&s->rnd2, 1);
			}					
		}
		Synth_UpdateIncrement(s);
		break;
	case parMorph:
		while (iterVoice != NULL)
		{	
			iterVoice->additiveMasterL.morph = 
			iterVoice->additiveMasterR.morph = p->val;
			iterVoice = iterVoice->Next;
		}
		break;
	case parGlobalVolume:
		s->globalAmplitude = dbToAmpAccurate(p->val);
		break;
	case parPitchOffset:
		Synth_UpdateIncrement(s);
		break;
	case parStereoPhaseOffset:
		for (int i = 0; i < 128; ++i)
		{			
			s->Voices[i].additiveMasterL.globalPhaseOffset = p->val;
			s->Voices[i].additiveMasterR.globalPhaseOffset = -p->val;
		}
		break;
	case parSeed:
		s->rnd.state = s->rnd.seed = p->val+49572;
		s->rnd2.state = s->rnd2.seed = p->val+245606;
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
			strcpy((char*)ptr, "SoEm Additive v0.3");
			break;
		case effGetProductString:
			strcpy((char*)ptr, "Additive component to the the Hypersaw Symphony");
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

	for (int i = 0; i < 128; ++i)
	{
		AdditiveMaster_init(&s->Voices[i].additiveMasterL);
		AdditiveMaster_init(&s->Voices[i].additiveMasterR);
	}

	makeSinWavetable();

	s->PARS[parStereoMode]     = makePar("StereoMode", 0, 2, 0);	
	s->PARS[parNumHarmonics]   = makePar("NumHarmonics", 1, ADDITIVE_MAX_HARMONICS, 1);
	s->PARS[parAmpCurve]       = makePar("Amp Curve", .01, 1, 1);

	s->PARS[parDriftColor]     = makePar("DriftColor", 0, 10, 0);
	s->PARS[parDriftDetail]    = makePar("DriftDetail", 0, 10, 0);
	s->PARS[parDriftCurve]     = makePar("DriftCurve", -.9999, .9999, 0);
	s->PARS[parDriftAmplitude] = makePar("DriftAmplitude", 0, 10, 0);	

	s->PARS[parRandomPhase]    = makePar("Random Phase", 0, 2, 0);	
	s->PARS[parSkewPhase]      = makePar("Phase Skew", 0, 1000, 0);
	s->PARS[parSkewPhaseCurve] = makePar("Phase Skew Curve", -.9999, .9999, 0);

	s->PARS[parResetPhaseOnAttack]     = makePar("resetPhaseOnAttack", 0, 1, 1);	
	s->PARS[parRandomizePhaseOnAttack] = makePar("randomizePhaseOnAttack", 0, 1, 0);	

	s->PARS[parPitchSkew]      = makePar("Pitch Skew", -2, 2, 0);	
	s->PARS[parPitchSkewCurve] = makePar("Pitch Skew Curve", -.9999, .9999, 0);
	s->PARS[parPitchRandom]    = makePar("Pitch Random", 0, 72, 0);

	s->PARS[parFreqSkew]      = makePar("Frq Skew", 0, 12, 1);
	s->PARS[parFreqSkewCurve] = makePar("Frq Skew Curve", -.9999, .9999, 0);
	s->PARS[parFreqRandom]    = makePar("Frq Random", 0, 5000, 0);

	s->PARS[parPitchSeed]     = makePar("Pitch seed", 0, 9999, 0);
	s->PARS[parFreqSeed]      = makePar("Freq seed", 0, 9999, 0);

	s->PARS[parWaveform]            = makePar("Waveshape", 0, 10, 0);
	s->PARS[parMorph]               = makePar("Morph", 0, 1, .5);
	s->PARS[parAllHarmonicsSameAmp] = makePar("AllHarmonicsSameAmp", 0, 1, 0);
	s->PARS[parGlobalVolume]        = makePar("Global Volume", -100, 0, -12);
	s->PARS[parPitchOffset]         = makePar("Pitch Offset", -72, 72, 0);
	s->PARS[parStereoPhaseOffset]   = makePar("Stereo Phase Offset", -1, 1, 0);
	s->PARS[parSeed]                = makePar("Seed", 0, 9999, 0);	


	makeDefaultPatch(&s->Presets[0], (Par*)&s->PARS, "initialized");

	for (int i = 1; i < PRESETS_MAX; ++i)
	{
		sprintf((char*) &s->Presets[i].name, "Preset %i", i);
		makeDefaultPatch(&s->Presets[i], (Par*)&s->PARS, (char*) &s->Presets[i].name); 
	}

	return effect;
}