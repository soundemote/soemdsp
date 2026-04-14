#pragma once

#include <math.h>

#include "../../DBG.h"
#include "../../math.h"
#include "../../variable.h"

#include "../Highpass.h"
#include "../Lowpass.h"
#include "../../Dynamics/SoftClipper.h"
#include "../../Modulators/WalterRandomWalk.h"

typedef struct FMD
{
    double cutoff;
    double resonance;

    double inputAmp;
    double outputAmp;

    double noiseAmp;
    Highpass noiseHPF;
    WalterRandomWalk random; // a better noise source than just straight white noise

    double bleed;      // input signal filtered by lp/hp and sent to modulate filter cutoff
    Lowpass bleedLPF;  // lowpass filter for bleed signal
    Highpass bleedHPF; // highpass filter for bleed signal

    SoftClipper clipper;

    double p_phase; //phasor phase
    double p_freq;  //phasor frequency
    double p_inc;   //phasor phase increment amount

    // need curve envelope
} FMD;

#include "FlowerChildRev1.h"