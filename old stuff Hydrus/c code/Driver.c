// tcc -Os -ffunction-sections -fdata-sections
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <windows.h>

#define __GNUC__
#define LOGFILE "Plugin_Driver.log"

#include "aeffect.h"
#include "aeffectx.h"

__declspec(dllexport) struct AEffect* VSTPluginMain(audioMasterCallback audioMaster);

#ifdef doDBG
bool LogCreated = false;

void LogB(char *message)
{
	FILE *file; if (!LogCreated) { 
		file = fopen(LOGFILE, "w"); 
		LogCreated = true; 
	} else 
	file = fopen(LOGFILE, "a"); 
	if (file == NULL) { 
		if (LogCreated) 
			LogCreated = false; 
		return; 
	} 
	else { 
		fputs(message, file); 
		fclose(file); 
	} 
	if (file) fclose(file); 
}

void Log (char *message) {LogB(message);}

void LogLn (char *message) { 
	LogB(message);
	LogB("\n");
} 

void LogInt(int x)
{
	char str[255];
	sprintf(str, "%d", x);
	Log(str);
}

void LogFl(float x)
{
	char str[255];
	sprintf(str, "%f", x);
	Log(str);
}

void LogMsg(char *message, int x)
{
	LogB(message);
	LogInt(x);
	LogB("\n");
}

void LogMsgF(char *message, float x)
{
	LogB(message);
	LogFl(x);
	LogB("\n");
}
#endif

typedef struct ERect ERect;
typedef struct VstEvents VstEvents;
typedef struct VstEvent VstEvent;
typedef struct VstMidiEvent VstMidiEvent;
typedef struct VstParameterProperties VstParameterProperties;
typedef struct VstTimeInfo VstTimeInfo; 

typedef struct Driver
{
	struct AEffect instance;
	audioMasterCallback Host;
	int sampleStart;
	VstEvents midiOutEvents;
	VstMidiEvent midiEvent;
	float speedMult;
	float param_OnOff;
	float param_Speed;
	float param_BPM;  // if 0 then use host bpm, and send global reset
	// also global reset can be received by this plugin and then forwarded
} Driver;

const int channels = 2;
const int NumParams = 3;
float sampleRate;
int blockSize;

void int2string(VstInt32 value, char *string) { sprintf(string, "%d", value); }
void float2string(float value, char *string) { sprintf(string, "%.2f", value); }
VstInt32 float2int(float number) { return (VstInt32)(1000.0f * number); }

void setSampleRate(float sampleRateIn) 
{ 
	sampleRate = sampleRateIn; 
	#ifdef doDBG
	LogMsgF("setSampleRate : ", sampleRateIn );
	LogMsg("setSampleRate (int) : ", (int) sampleRateIn );
	#endif
}
void setBlockSize(VstInt32 blockSize) { blockSize = blockSize; }

VstInt32 processEvents(VstEvents* ev) 
{
	// for (int i = 0; i < ev->numEvents; i++) 
	// {
	// 	VstEvent* event = ev->events[i];
	// 	if (event->type == kVstMidiType) 
	// 	{
	// 		VstMidiEvent* midiEvent = (VstMidiEvent*) event;
	// 		VstInt8 status = midiEvent->midiData[0];
	// 		VstInt8 data1 = midiEvent->midiData[0];
	// 		VstInt8 data2 = midiEvent->midiData[0];
	// 		// printf("MIDI: %02x %02x %02x\n", status, data1, data2); 
	// 	} 
	// }
	return 0; 
}

enum ParamNames { parameter1, parameter2, parameter3 };

bool getVendorString(char* ptr) { strncpy((char*)ptr, "Walter Hackett", kVstMaxVendorStrLen); return true; }

void getParameterName(VstInt32 index, char* text) 
{
	switch (index) 
	{
		case parameter1:
			strncpy((char*)text, "Power", kVstMaxParamStrLen);
			break;
		case parameter2:
			strncpy((char*)text, "Interval", kVstMaxParamStrLen);
			break;
		case parameter3:
			strncpy((char*)text, "BPM", kVstMaxParamStrLen);
			break; 
	} 
}

void getParameterLabel(Driver* effect, VstInt32 index, char *label) 
{
	switch (index) 
	{
		case parameter1: // On or Off
			if (effect->param_OnOff < 0.5) strncpy((char*)label, "Off", kVstMaxParamStrLen);
			else strncpy((char*)label, "On", kVstMaxParamStrLen);
			break;
		case parameter2: // Speed
			switch((int) (effect->param_Speed * 8.0))
			{
				case 0:
					strncpy((char*)label, "Off", kVstMaxParamStrLen);
					break;
				case 1:
					strncpy((char*)label, "1/2", kVstMaxParamStrLen);
					break;
				case 2:
					strncpy((char*)label, "1/4", kVstMaxParamStrLen);
					break;
				case 3:
					strncpy((char*)label, "1/8", kVstMaxParamStrLen);
					break;
				case 4:
					strncpy((char*)label, "1/16", kVstMaxParamStrLen);
					break;
				case 5:
					strncpy((char*)label, "1/32", kVstMaxParamStrLen);
					break;
				case 6:
					strncpy((char*)label, "1/64", kVstMaxParamStrLen);
					break;
				case 7:
					strncpy((char*)label, "1/128", kVstMaxParamStrLen);
					break;
				case 8:
					strncpy((char*)label, "1/256", kVstMaxParamStrLen);
					break;
			}
			break;
			//switch (float2int(effect->mode_value * 2.99 / 1000.0)) 
		case parameter3: // BPM
			if (effect->param_BPM < 0.00001) strncpy((char*)label, "Host BPM", kVstMaxParamStrLen);
			else float2string(effect->param_BPM * 1000.0, label);
			break;
	} 
}

void getParameterDisplay (VstInt32 index, char* text) { *text = 0; }

bool getParameterProperties(VstInt32 index, VstParameterProperties* p) { return false; }

VstIntPtr Dispatcher(AEffect* effect, VstInt32 opcode, VstInt32 index, VstIntPtr value, void* ptr, float opt)
{
	VstIntPtr result = 0;
	switch (opcode)
	{
		case effSetSampleRate:
			setSampleRate(opt);
			break;
		case effSetBlockSize:
			setBlockSize((VstInt32)value);
			break;
		case effProcessEvents:
			result = processEvents((VstEvents*)ptr);	
			break;
		case effGetPlugCategory:
			result = kPlugCategEffect | kPlugCategAnalysis;
			break;
		case effGetVendorString:
			result = getVendorString(ptr);
			break;
		case effGetEffectName:
			strncpy((char*)ptr, "WH Driver", kVstMaxEffectNameLen);
			break;
		case effGetProductString:
			strncpy((char*)ptr, "magic plugin", kVstMaxEffectNameLen);
			break;
		case effGetParamName:
			getParameterName(index, (char*)ptr);
			break;
		case effGetParamDisplay:
			getParameterDisplay(index, (char*)ptr);
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
		default:
			break;
	}
	return result;
}

void sendVstEventsToHost(Driver* effect, VstEvents* events)
{
	if (effect->Host) (effect->Host) (effect, audioMasterProcessEvents, 0, 0, events, 0);
}

VstTimeInfo* getTimeInfo(Driver* effect, VstInt32 filter)
{
	if (effect->Host) return (VstTimeInfo*) (effect->Host) (effect, audioMasterGetTime, 0, filter, 0, 0);
	return 0;
}

void process32(Driver* effect, float** inputs, float** outputs, VstInt32 sampleframes)
{
	double tempo = effect->param_BPM * 1000.0;

	if (effect->param_BPM < 0.00001)
	{
	    VstTimeInfo * info = getTimeInfo( effect, kVstTempoValid | kVstPpqPosValid | kVstTransportPlaying |
	      kVstTransportChanged | kVstTransportCycleActive | kVstTransportRecording | 
	      kVstAutomationWriting | kVstAutomationReading);

	    if (!(info && (kVstTempoValid & info->flags) && (kVstPpqPosValid  & info->flags))) return;
	    
	    tempo = info->tempo;
    }

    tempo = (sampleRate * 60.0) / tempo * effect->speedMult; //  / the send rate

    int sampleEnd = effect->sampleStart + sampleframes;

    // if  TimeInfo available, set sampleStart to CurrentSample
    // if  TimeInfo available, send global reset on daw restart

    int multipleCount = (int)(effect->sampleStart / tempo);
    int start = (int)(multipleCount * tempo);
    if (effect->sampleStart > start) start = (int)((++multipleCount) * tempo);

    while (sampleEnd >= start)
    {
      // Send CC (Inc)
      effect->midiEvent.midiData[0] = 0xB0 & 0xf0;
      effect->midiEvent.midiData[1] = 111 & 0x7F; // control number (here is 65)
      effect->midiEvent.midiData[2] = 111 & 0x7F; // cc value (here is 127) 
      effect->midiEvent.deltaFrames = start - effect->sampleStart;

      // Note On
      // effect->midiEvent.midiData[0] = 0x90 & 0xf0;
      // effect->midiEvent.midiData[1] = 65 & 0x7F; // note number (here is 65)
      // effect->midiEvent.midiData[2] = 127; // velocity
      // effect->midiEvent.deltaFrames = start - effect->sampleStart;

      // Note Off
      // effect->midiEvent.midiData[0] = 0x80 & 0xf0;
      // effect->midiEvent.midiData[1] = 65 & 0x7F;  // note number (here is 65)
      // effect->midiEvent.midiData[2] = 127; // velocity
      // effect->midiEvent.deltaFrames = start - effect->sampleStart;
      if (effect->param_OnOff >= 0.5) sendVstEventsToHost(effect, &effect->midiOutEvents);

      start = (int)((++multipleCount) * tempo);
    }
    effect->sampleStart = sampleEnd;
}

void setparam(Driver* effect, VstInt32 index, float value)
{
	switch (index) 
	{
		case parameter1:
			effect->param_OnOff = value;
			break;
		case parameter2:
			effect->param_Speed = value;
			switch((int) (value * 8.0))
			{
				case 0:
					effect->speedMult = 1.0;
					break;
				case 1:
					effect->speedMult = 1.0 / 2.0;
					break;
				case 2:
					effect->speedMult = 1.0 / 4.0;
					break;
				case 3:
					effect->speedMult = 1.0 / 8.0;
					break;
				case 4:
					effect->speedMult = 1.0 / 16.0;
					break;
				case 5:
					effect->speedMult = 1.0 / 32.0;
					break;
				case 6:
					effect->speedMult = 1.0 / 64.0;
					break;
				case 7:
					effect->speedMult = 1.0 / 128.0;
					break;
				case 8:
					effect->speedMult = 1.0 / 256.0;
					break;
			}
			break;
		case parameter3:
			effect->param_BPM = value;
			break;
	} 
}

float getparam(Driver* effect, VstInt32 index)
{
	float result = 0;
	switch (index) 
	{
		case parameter1:
			result = effect->param_OnOff;
			break;
		case parameter2:
			result = effect->param_Speed;
			break;
		case parameter3:
			result = effect->param_BPM;
			break;
	}
	return result;
}

AEffect* VSTPluginMain(audioMasterCallback audioMaster)
{
	AEffect* effect = (AEffect*) malloc(sizeof(Driver));
	memset(effect, 0, sizeof(Driver));
	effect->magic = kEffectMagic;
	effect->dispatcher = &Dispatcher;
	effect->setParameter = &setparam;
	effect->getParameter = &getparam;
	effect->numParams = NumParams;
	effect->numInputs = channels;
	effect->numOutputs = channels;
	effect->flags = effFlagsCanReplacing;
	effect->processReplacing = &process32;
	effect->uniqueID = 1987;
	effect->version = 1;
	effect->object = 0;
	Driver* effectX = (Driver*)effect;
	effectX->sampleStart = 0;
	effectX->Host = audioMaster;
	effectX->midiOutEvents.numEvents = 1;
	effectX->param_OnOff = 1.0;
	effectX->speedMult = 1.0;
	effectX->midiOutEvents.events[0] = (VstEvent*)&effectX->midiEvent;
	effectX->midiEvent.type = kVstMidiType;
	return effect;
}

	// LogMsg("effectX->sampleStart : ",(int) effectX->sampleStart);
	// LogMsg("effectX->Host : ",(int) effectX->Host);
	// LogMsg("effectX->midiOutEvents.numEvents : ",(int) effectX->midiOutEvents.numEvents);
	// LogMsg("effectX->midiEvent.type : ",(int) effectX->midiEvent.type);

// #ifdef doDBG
//     if (inCase > 9) LogLn("inCase Exceeded");

// // int dbgInc = 101;
// 	dbgInc++;
// 	if (dbgInc > 100)
// 	{
// 		LogMsg("process32 inCase : ",(int) inCase);
// 		LogMsg("process32 start : ",(int) start);
// 		LogMsg("process32 multipleCount : ",(int) multipleCount);
// 		LogMsg("process32 effect->sampleStart : ",(int) effect->sampleStart);
// 		LogMsg("process32 sampleframes : ",(int) sampleframes);
// 		LogMsgF("process32 tempo : ",(float)tempo);
// 		LogMsgF("process32 sampleRate : ",sampleRate);

// 		LogLn("============================");

// 		dbgInc = 0;
// 	}
// #endif