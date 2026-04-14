#pragma once

// use read pointers to point to a master value when needed

typedef struct variable {
    double vw;  // value WRITE
    double* vr; // value pointer READ

    double vmw;  // value MOD WRITE
    double* vmr; // value pointer MOD READ
} variable;

void variable_init(variable* o) {
    o->vw  = 0;
    o->vmw = 0;

    o->vr  = &o->vw;
    o->vmr = &o->vmw;
};

double variable_getWithMod(variable* o) {
    return *o->vr + *o->vmr;
}

double variable_getNoMod(variable* o) {
    return *o->vr;
}

enum OSC_VARIABLES {
    OSC_increment,
    OSC_frequency,
    OSC_phase,
    OSC_morph,
    OSC_waveform,
    OSC_amplitude,

    OSC_NUMVARS
};

enum FILTER_VARIABLES {
    FLT_increment,
    FLT_frequency,
    FLT_resonance,
    FLT_coeff,

    FLT_NUMVARS
};
