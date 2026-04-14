/* 1. BOILER PLATE */

// tcc -Os -ffunction-sections -fdata-sections
#define _CRT_SECURE_NO_WARNINGS
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

//#define __GNUC__
#define LOGFILE "Plugin_Synth.log"

#include "aeffect.h"
#include "aeffectx.h"

#include "../library/DBG.h"
#include "../library/semath.h"

//__attribute__((dllexport)) struct AEffect* VSTPluginMain(audioMasterCallback audioMaster); // linux, mac, tcc
__declspec(dllexport) struct AEffect* VSTPluginMain(audioMasterCallback audioMaster); // windows, visual studio


typedef struct VstEvents VstEvents;
typedef struct VstEvent VstEvent;
typedef struct VstMidiEvent VstMidiEvent;
typedef struct VstParameterProperties VstParameterProperties;
typedef struct VstTimeInfo VstTimeInfo;

/* 2. #DEFINE CONSTANTS, COMPILED CONSTANTS, GLOBALS, AND PARAMETER ENUMS */

#define UNIQUE_ID 6253
#define VERSION_NUMBER 1000
#define MASTER_SEED 2433003
#define PRESETS_MAX 128
#define VOICES_MAX 128
#define NUM_INPUT_CHANNELS 2
#define NUM_OUTPUT_CHANNELS 2
const char VENDOR_STRING[]  = "Soundemote";
const char EFFECT_NAME[]    = "SoEm Tone v1.0";
const char PRODUCT_STRING[] = "Simple tone generator activated via midi.";

enum ParameterIndexes {
    parGlobalVolume,
    parPitchOffset,
    parWaveform,
    parMorph,

    NUM_PARAMS
};

/* 3. INCLUDE DSP LIBRARY AND CUSTOM DSP INCLUDES */

#include "../soundemote_dsp_library.h"

/* 4. START OF IMPLEMENTATION CODE */

typedef struct Voice {
    struct Voice* Prev;
    struct Voice* Next;
    bool Active;
    double pitch;
    double increment;
    double frequency;
    double velocityNormalized;

    bool needsIncrementUpdate;
    bool needsMorphUpdate;

    SineSaw saw;
    SineSquare square;
} Voice;

typedef struct Synth {
    struct AEffect instance;
    audioMasterCallback Host;
    struct Voice* ActiveVoices;
    struct Voice Voices[VOICES_MAX];

    struct Preset Presets[PRESETS_MAX];
    int CurrentPreset;

    Par PARS[NUM_PARAMS];

} Synth;

void setSampleRate(Synth* o, float sampleRateIn) {
    boilerplate_samplerate_stuff(sampleRateIn);
}

void setBlockSize(VstInt32 blockSize) {
    blockSize = blockSize;
}

void updateSingleVoiceIncrement(Synth* s, Voice* v) {
    v->needsIncrementUpdate = false;
    v->frequency            = pitchToFrequency(v->pitch + s->PARS[parPitchOffset].val + 12);
    v->increment            = sampleperiod * v->frequency;

    double square_max_partials = 0.5 * (v->frequency == 0.0 ? 2.0 : halfsamplerate / v->frequency);
    SineSquare_incrementChanged(&v->square, square_max_partials);
    SineSaw_incrementChanged(&v->saw);
}

void updateSingleVoiceMorph(Synth* s, Voice* v) {
    v->needsMorphUpdate = false;

    SineSquare_morphChanged(&v->square);
    SineSaw_morphChanged(&v->saw);
}

void updateActiveVoicesIncrement(Synth* s) {
    Voice* iterVoice = s->ActiveVoices;
    while (iterVoice != NULL) {
        updateSingleVoiceIncrement(s, iterVoice);
        iterVoice = iterVoice->Next;
    }
}

void updateActiveVoicesMorph(Synth* s) {
    Voice* iterVoice = s->ActiveVoices;
    while (iterVoice != NULL) {
        updateSingleVoiceMorph(s, iterVoice);
        iterVoice = iterVoice->Next;
    }
}

// enum MidiEvents {
// };

VstInt32 processEvents(Synth* s, VstEvents* ev) {
    for (int i = 0; i < ev->numEvents; i++) {
        VstEvent* event = ev->events[i];

        if (event->type == kVstMidiType) {
            VstMidiEvent* midiEvent = (VstMidiEvent*)event;
            switch (midiEvent->midiData[0] & 0xF0) {
            case 0x80: { // type=kMidiNoteOff
                Voice* selectedVoice              = &(s->Voices[(int)(midiEvent->midiData[1] & 0x7F)]);
                selectedVoice->velocityNormalized = 0;

                if (selectedVoice->Active) {
                    selectedVoice->Active = false;
                    if (selectedVoice->Next != NULL)
                        selectedVoice->Next->Prev = selectedVoice->Prev;
                    if (selectedVoice->Prev != NULL)
                        selectedVoice->Prev->Next = selectedVoice->Next;
                    else
                        s->ActiveVoices = selectedVoice->Next;
                    selectedVoice->Next = NULL;
                    selectedVoice->Prev = NULL;
                }
            } break;

            case 0x90: { // type=kMidiNoteOn;
                Voice* selectedVoice              = &(s->Voices[(int)(midiEvent->midiData[1] & 0x7F)]);
                selectedVoice->velocityNormalized = (midiEvent->midiData[2] & 0x7F) * _1_z_127;

                if (!selectedVoice->Active) {
                    selectedVoice->Active = true;
                    selectedVoice->Next   = s->ActiveVoices;
                    selectedVoice->Prev   = NULL;

                    if (s->ActiveVoices != NULL)
                        s->ActiveVoices->Prev = selectedVoice;

                    if (selectedVoice->needsIncrementUpdate) {
                        selectedVoice->needsIncrementUpdate = false;
                        updateSingleVoiceIncrement(s, selectedVoice);
                    }

                    if (selectedVoice->needsMorphUpdate) {
                        selectedVoice->needsMorphUpdate = false;
                        updateSingleVoiceMorph(s, selectedVoice);
                    }

                    s->ActiveVoices = selectedVoice;
                }
            } break;
            case 0xB0: // type=kMidiControlChange;
                break;
            case 0xC0: // type=kMidiProgramChange;
                break;
            case 0xE0: // type=kMidiPitchWheel;
                break;
            case 0xA0: // type=kMidiNoteAfterTouch;
                break;
            case 0xD0: // type=kMidiChannelAfterTouch;
                break;
            }
        }
    }

    return 0;
}

bool getVendorString(char* ptr) {
    strncpy((char*)ptr, VENDOR_STRING, kVstMaxVendorStrLen);
    return true;
}

void getParameterName(Synth* s, VstInt32 index, char* text) {
    strncpy((char*)text, (char*)&s->PARS[index].name, kVstMaxLabelLen);
}

void getParameterLabel(Synth* s, VstInt32 index, char* label) {
    Par* p = &s->PARS[index];

    float val = map0to1(p->normVal, p->min, p->max);

    switch (index) {
    default:
        sprintf(label, "%.2f", val);
        return;
    }
}

void getParameterDisplay(Synth* s, VstInt32 index, char* text) {
    switch (index) {
    case parGlobalVolume:
        strcpy((char*)text, "dB");
    case parWaveform:
        switch ((int)s->PARS[parWaveform].storedVal) {
        case 0:
            strcpy((char*)text, "Saw");
        case 1:
            strcpy((char*)text, "Square");
        }
    default:
        *text = 0;
        return;
    }
}

bool getParameterProperties(VstInt32 index, VstParameterProperties* p) {
    return false;
}

void sendVstEventsToHost(Synth* effect, VstEvents* events) {
    if (effect->Host)
        (effect->Host)(effect, audioMasterProcessEvents, 0, 0, events, 0);
}

VstTimeInfo* getTimeInfo(Synth* effect, VstInt32 filter) {
    if (effect->Host)
        return (VstTimeInfo*)(effect->Host)(effect, audioMasterGetTime, 0, filter, 0, 0);

    return 0;
}

void process32(Synth* s, float** inputs, float** outputs, VstInt32 sampleframes) {
    float* oleft  = outputs[0];
    float* oright = outputs[1];

    while (--sampleframes >= 0) {
        Voice* iterVoice = s->ActiveVoices;

        while (iterVoice != NULL) {
            double out = 0.0;

            switch ((int)s->PARS[parWaveform].storedVal) {
            case 0:
                out = SineSaw_getSample(&iterVoice->saw) * s->PARS[parGlobalVolume].storedVal;
                break;
            case 1:
                out = SineSquare_getSample(&iterVoice->square) * s->PARS[parGlobalVolume].storedVal;
                break;
            }

            *oleft += out;
            *oright += out;

            iterVoice = iterVoice->Next;
        }

        ++oleft;
        ++oright;
    }
}

void setparam(Synth* s, VstInt32 index, float value) {
    Voice* iterVoice = s->ActiveVoices;
    Par* p           = &s->PARS[index];
    p->normVal       = value;
    p->val           = map0to1(p->normVal, p->min, p->max);

    s->Presets[s->CurrentPreset].value[index] = value;

    switch (index) {
    case parGlobalVolume:
        p->storedVal = dbToAmpAccurate(p->val);
        break;
    case parPitchOffset:
        for (int i = 0; i < VOICES_MAX; ++i)
            s->Voices[i].needsIncrementUpdate = true;
        updateActiveVoicesIncrement(s);
        break;
    case parMorph:
        p->storedVal = wrapPhaseBidirectional(p->val);
        for (int i = 0; i < VOICES_MAX; ++i)
            s->Voices[i].needsMorphUpdate = true;
        updateActiveVoicesMorph(s);
        break;
    case parWaveform:
        p->storedVal = round(p->val);
    default:
        break;
    }
}

float getparam(Synth* s, VstInt32 index) {
    Par* p = &s->PARS[index];

    switch (index) {
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
    switch (opcode) {
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
        strcpy_s((char*)ptr, kVstMaxEffectNameLen, EFFECT_NAME);
        break;
    case effGetProductString:
        strcpy_s((char*)ptr, kVstMaxProductStrLen, PRODUCT_STRING);
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
    case effClose: {
        Synth* s = (Synth*)effect;
        free(s);
        // free(ptr);
        result = 1;
        break;
    }
    case effGetProgramName: {
        Synth* s = (Synth*)effect;
        strncpy((char*)ptr, (char*)&s->Presets[s->CurrentPreset].name, kVstMaxProgNameLen);
    } break;
    case effGetProgramNameIndexed: {
        if (index < PRESETS_MAX) {
            Synth* s = (Synth*)effect;
            strncpy((char*)ptr, (char*)&s->Presets[index].name, kVstMaxProgNameLen);
            result = 0;
        }
    } break;
    case effGetProgram: {
        Synth* s = (Synth*)effect;
        return s->CurrentPreset;
    } break;
    case effSetProgram: {
        Synth* s = (Synth*)effect;
        setProgram(s, value);
    } break;
    default:
        break;
    }

    return result;
}

AEffect* VSTPluginMain(audioMasterCallback audioMaster) {
    AEffect* effect = (AEffect*)malloc(sizeof(Synth));
    memset(effect, 0, sizeof(Synth));
    effect->magic            = kEffectMagic;
    effect->dispatcher       = &Dispatcher;
    effect->setParameter     = &setparam;
    effect->getParameter     = &getparam;
    effect->numParams        = NUM_PARAMS;
    effect->numInputs        = NUM_INPUT_CHANNELS;
    effect->numOutputs       = NUM_OUTPUT_CHANNELS;
    effect->flags            = effFlagsCanReplacing | effFlagsIsSynth;
    effect->processReplacing = &process32;
    effect->uniqueID         = UNIQUE_ID;
    effect->version          = VERSION_NUMBER;
    effect->object           = 0;
    effect->numPrograms      = PRESETS_MAX;

    Synth* s        = (Synth*)effect;
    s->Host         = audioMaster;
    s->ActiveVoices = NULL;

    s->PARS[parGlobalVolume] = makePar("Volume", -70, 0, 0);
    s->PARS[parPitchOffset]  = makePar("Transpose", -128, 128, 0);
    s->PARS[parWaveform]     = makePar("Waveform", 0, 1, 0);
    s->PARS[parMorph]        = makePar("Morph", -10, 10, 0);

    makeDefaultPatch(&s->Presets[0], (Par*)&s->PARS, "initialized");

    for (int i = 1; i < PRESETS_MAX; ++i) {
        sprintf((char*)&s->Presets[i].name, "Preset %i", i);
        makeDefaultPatch(&s->Presets[i], (Par*)&s->PARS, (char*)&s->Presets[i].name);
    }

    for (int i = 0; i < 128; ++i) {
        s->Voices[i].pitch = i;

        SineSquare_init(&s->Voices[i].square);
        s->Voices[i].square.v[OSC_frequency].vr = &s->Voices[i].frequency;
        s->Voices[i].square.v[OSC_increment].vr = &s->Voices[i].increment;
        s->Voices[i].square.v[OSC_morph].vr     = &s->PARS[parMorph].storedVal;

        SineSaw_init(&s->Voices[i].saw);
        s->Voices[i].saw.v[OSC_frequency].vr = &s->Voices[i].frequency;
        s->Voices[i].saw.v[OSC_increment].vr = &s->Voices[i].increment;
        s->Voices[i].saw.v[OSC_morph].vr     = &s->PARS[parMorph].storedVal;
    }

    return effect;
}