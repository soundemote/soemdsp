#pragma once

typedef struct FlowerChildRev1
{
    FMD fmd;
    //FlowerChildRev1* slave;

    double feedback;
    double feedbackAmp;
    double fm_pm_crossfade;
    double fm;
    double pm;
    double currentOscVal;

    Lowpass LPF1;
    Lowpass LPF2;

    Graph fmpmNodeGraph;
    Graph resVsFreqGraph;

    double frequencyNormalized;
    double maxNormFreq;

} FlowerChildRev1;

void FlowerChildRev1_init(FlowerChildRev1* o)
{
    Graph_addNode(&o->fmpmNodeGraph, 0, 0.21, 0.00, LINEAR);
    Graph_addNode(&o->fmpmNodeGraph, 1, 0.00, 0.53, EXPONENTIAL);

    Graph_addNode(&o->resVsFreqGraph, 0, 0, 0, LINEAR);
    Graph_addNode(&o->resVsFreqGraph, 0, 0, 0, LINEAR);
    Graph_addNode(&o->resVsFreqGraph, 0, 0, -0.38, RATIONAL);
}

void FlowerChildRev1_setInputAmplitude(FlowerChildRev1* o, double v)
{
    o->fmd.inputAmp = v;
}

void FlowerChildRev1_resonanceChanged(FlowerChildRev1* o)
{
    Graph_moveNodeNoSort(&o->resVsFreqGraph, 1, 0.816054, o->fmd.resonance);
    Graph_moveNodeNoSort(&o->resVsFreqGraph, 2, 1.000, min(o->fmd.resonance, 0.818713));

    o->feedbackAmp = map0to1(rationalCurve(Graph_getValue(&o->resVsFreqGraph, o->fmd.resonance), .4), 0.0368, .6333);
}

void FlowerChildRev1_setCutoff(FlowerChildRev1* o, double v)
{
    double f = pitchToFrequency(map0to1(v, 3.0, 161.0));
    o->fm_pm_crossfade = Graph_getValue(&o->fmpmNodeGraph, v);

    double neg_TAU_x_f = -TAU * f;

    o->LPF1.v[FLT_coeff].dw = exp(neg_TAU_x_f * 0.164312 * sampleperiod);
    o->LPF2.v[FLT_coeff].dw = exp(neg_TAU_x_f * 0.366131 * sampleperiod);

    Lowpass_incrementChanged(&o->LPF1);
    Lowpass_incrementChanged(&o->LPF2);

    // TODO: adjust random walk frequency

    FlowerChildRev1_resonanceChanged(o);
}

void FlowerChildRev1_setFeedback(FlowerChildRev1* o, double v)
{
    o->fmd.resonance = v;
    FlowerChildRev1_resonanceChanged(o);
}

void FlowerChildRev1_setNoise(FlowerChildRev1* o, double v)
{
    o->fmd.noiseAmp = v;
}

double getSample(FlowerChildRev1* o, double input)
{
    // clamp input signal		
    double signal = clamp(o->fmd.inputAmp * 2.3 * -input, -1.0, +1.0);

    // TODO: add noise to signal
    //signal += Highpass_getSample(&o->fmd.noiseHPF, WalterRandomWalk_getSample(&o->fmd.random)) * o->fmd.noiseAmp;

    // establish feedback signal
    signal = o->feedback + 0.035848699999999845 * signal;

    // amplify signal and split input signal into phase and frequency modulation
    signal *= 1.4;
    double fm = signal * cos(PI_z_2 * o->fm_pm_crossfade);
    double pm = signal * sin(PI_z_2 * o->fm_pm_crossfade);

    // increment phasor
    o->fmd.p_freq = (o->fmd.cutoff * o->fm);
    o->fmd.p_inc = o->fmd.p_freq * sampleperiod;
    o->fmd.p_phase = wrapPhase(o->fmd.p_phase + o->fmd.p_inc);
    signal = wrapPhase(o->fmd.p_phase + o->pm);

    // get resonance and raise amplitude
    signal = sin(signal * TAU) * 1.3;

    // filter feedback signal
    signal = Lowpass_getSample(&o->LPF1, signal);
    signal = Lowpass_getSample(&o->LPF2, signal);

    // get feedback value
    o->feedback = signal * o->feedbackAmp;

    // amplify and final out
    return signal * 1.31;
}