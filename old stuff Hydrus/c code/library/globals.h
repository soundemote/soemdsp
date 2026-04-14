#pragma once

const double _1_z_127 = 1.0 / 127.0;
double _440_z_sampleRate = 0;
double samplerate = 0;
double halfsamplerate = 0;
double _7z8xSampleRate = 0;
double TAU_z_samplerate = 0;
double sampleperiod = 0;
int blockSize = 0;

void boilerplate_samplerate_stuff(double v) {
    samplerate = v;
    halfsamplerate = v * 0.5;
    sampleperiod = 1.0 / v;
    _440_z_sampleRate = 440 * v;
    TAU_z_samplerate = TAU / v;
}

enum ENVSTATE {
    ENV_OFF,
    ENV_DELAY,
    ENV_ATTACK,
    ENV_DECAY,
    ENV_SUSTAIN,
    ENV_RELEASE
};

enum WAVEFORM {
    SIN,
    COS,
    SQUARE,
    PULSE,
    PULSE_CENTER,
    SAW,
    RAMP,
    TRI,
    TRISAW,
    TRISQUARE,
    TRIPULSE,
    RECTIFIED_SIN_HALF,
    RECTIFIED_SIN_FULL,
    TRAPEZOID,

    POLYBLEP_NUM_WAVFORMS
};