#include "Hypersaw.h"

void HypersawUnit_init(HypersawUnit* o, int oscillatorIdx, int totalOscillators) {
    //SineSaw_setMorph(&o->osc, 1);
    //osc.setWaveform(WAVEFORM::SAWTOOTH);

    double lpInc = exp(-TAU * 20000 * sampleperiod);

    o->lpf.v[FLT_coeff].dw = lpInc;
    Lowpass_incrementChanged(&o->lpf);

    Highpass_incrementChanged(&o->hpf, 0);

    Lowpass_incrementChanged(&o->smoother, lpInc);

    LinearEnvelope_setSustainAmplitude(&o->env, 1);

    o->idx = oscillatorIdx;
    o->div = oscillatorIdx/totalOscillators;
    o->phaseDistribution = o->idx * o->div;

    o->walkSignal = 0;
    o->filteredWalkSignal = 0;
    o->vibratoSignal = 0;
    o->phaseDistribution = 0;
    o->envelopeSignal = 0;    
    o->targetPitch = 69;
    o->pitchOffset = 0;
    o->pitchDetuneAmount = 0;
    o->pitchDetuneCurve = 0;
    o->pitchOffsetFromDetune = 0.01;
    o->currentPitch = 69;
    o->currentFrequency = 440;
    o->lastPitchValue = 69;
    o->phaseOffset = 0;
    o->randomPhaseOffset = 0;
    o->randomPhaseOffsetRange = 0.1;
    o->PMDriftAmount = 0;
    o->PMDriftDetail = 0;
    o->targetFrequency = 440;
    o->portamentoSeconds = 0;
    o->portamentoRandomValue = 0;
    o->portamentoRandomMult = 0;
    o->delaySecondsRandomRange = 0;
    o->attackSeconds = 0;
    o->attackSecondsRandomMult = 0;
    o->releaseSeconds = 0;
    o->releaseSecondsRandomMult = 0;
}

double HypersawUnit_getSample(HypersawUnit* o) { 
    o->vibratoSignal = 0;
    o->filteredWalkSignal = 0;

    o->currentPitch = Lowpass_getSample(&o->smoother, o->targetPitch) + o->pitchOffset;
    o->currentFrequency = pitchToFrequency(o->currentPitch + o->pitchOffsetFromDetune);

    /* HYPERSAW */
    if (o->PMDriftAmount > 0)
    {	
        //RandomWalk_setDetail(&o->walkPM, map0to1(rationalCurve(o->PMDriftDetail, -.9), 0, o->currentFrequency*2));
        o->walkSignal = RandomWalk_getSample(&o->walkPM) * o->PMDriftAmount;
        o->filteredWalkSignal = Lowpass_getSample(&o->lpf, Highpass_getSample(&o->hpf, o->walkSignal));
    }
    
    /* SYMHPONIC SAW */		
    if (o->vibrato.range > 0)
        o->vibratoSignal = VibratoGenerator_getSample(&o->vibrato);

    o->envelopeSignal = LinearEnvelope_getSample(&o->env);

    o->osc.v[OSC_phase].dmw = wrapPhaseExtreme(o->filteredWalkSignal + o->vibratoSignal + o->phaseDistribution + o->randomPhaseOffset);

    SineSaw_setFrequency(&o->osc, o->currentFrequency);

    return SineSaw_getSample(&o->osc) * o->envelopeSignal;
}

void HypersawUnit_updateDetune(HypersawUnit* o) {        
    if (o->idx % 2 == 0) //posivie
        o->pitchOffsetFromDetune = rationalCurve(o->div * o->idx, o->pitchDetuneCurve) * 12 * (o->pitchDetuneAmount + o->pitchDetuneAmount);
    else                 // negative
        o->pitchOffsetFromDetune = rationalCurve(o->div * o->idx, o->pitchDetuneCurve) * 12 * -(o->pitchDetuneAmount + o->pitchDetuneAmount);
}

void HypersawUnit_setPitchDetuneAmount(HypersawUnit* o, double v) {
    o->pitchDetuneAmount = v;
    HypersawUnit_updateDetune(o);
}
void HypersawUnit_setPitchDetuneCurve(HypersawUnit* o, double v) {
    o->pitchDetuneCurve = clamp(v, -0.999999, 0.99999);
    HypersawUnit_updateDetune(o);
}

void HypersawUnit_updateAttack(HypersawUnit* o) {
    double randomAttack = NoiseGenerator_getSampleUni(&o->r);
    LinearEnvelope_setAttackTime(&o->env, o->attackSeconds + randomAttack * o->attackSecondsRandomMult);
}

// seed for A: random phase B: random frequency C: random vibrato frequency d: random vibrato amplitude
void HypersawUnit_setSeed(HypersawUnit* o, int v) {
    RandomWalk_setSeed(&o->walkPM, MASTER_SEED + 1 + v);
    RandomWalk_setSeed(&o->walkFM, MASTER_SEED + 2 + v);
    VibratoGenerator_setSeed(&o->vibrato, MASTER_SEED + 3 + v, MASTER_SEED + 4 + v);
    NoiseGenerator_setSeed(&o->r, MASTER_SEED + 5 + v);

    o->portamentoRandomValue = NoiseGenerator_getSampleUni(&o->r);

    HypersawUnit_updateAttack(o);
    HypersawUnit_updateRelease(o);
}

void HypersawUnit_setSampleRate(HypersawUnit* o) {
    SineSaw_setSampleRate(&o->osc);
    RandomWalk_setSampleRate(&o->walkPM);
    RandomWalk_setSampleRate(&o->walkFM);
    //Lowpass_setSampleRate(&o->lpf);
    Highpass_setSampleRate(&o->hpf);
    VibratoGenerator_setSampleRate(&o->vibrato);
    LinearSmoother_setSampleRate(&o->smoother);
    LinearEnvelope_setSampleRate(&o->env);
}

void HypersawUnit_reset(HypersawUnit* o) {
    SineSaw_reset(&o->osc);
    RandomWalk_reset(&o->walkPM);
    RandomWalk_reset(&o->walkFM);
    Lowpass_reset(&o->lpf);
    Highpass_reset(&o->hpf);
    VibratoGenerator_reset(&o->vibrato);
    LinearSmoother_reset(&o->smoother);
    LinearEnvelope_reset(&o->env);

    o->randomPhaseOffset = NoiseGenerator_getSampleUni(&o->walkPM.noiseGenerator) * 0.1;
}

void HypersawUnit_setOscMorph(HypersawUnit* o, double v) {
    //SineSaw_setMorph(&o->osc, v);
}
void HypersawUnit_setPitch(HypersawUnit* o, double v) {
    o->targetPitch = v;
}
void HypersawUnit_setPitchOffset(HypersawUnit* o, double v) {
    o->pitchOffset = v;
}

void HypersawUnit_setPortamentoSeconds(HypersawUnit* o, double v) {
    o->portamentoSeconds = max(v, 1.e-4);
    HypersawUnit_updatePortamento(o);
}
void HypersawUnit_setPortamentoRandomMult(HypersawUnit* o, double v) {
    o->portamentoRandomMult = v;
    HypersawUnit_updatePortamento(o);
}
void HypersawUnit_updatePortamento(HypersawUnit* o) {
    LinearSmoother_setFrequency(&o->smoother, 1.0/max(o->portamentoSeconds + o->portamentoRandomValue * o->portamentoRandomMult, 1.e-4));
}

void HypersawUnit_setPMDriftAmount(HypersawUnit* o, double v) {
    o->PMDriftAmount = v;
}
void HypersawUnit_setPMDriftColor(HypersawUnit* o, double v) {
    RandomWalk_setColor(&o->walkPM, v);
}
void HypersawUnit_setPMDriftDetail(HypersawUnit* o, double v) {		
    o->PMDriftDetail = v;
    //walkPM.setDetail(min(v,currentFrequency*2));
    //walkPM.setDetail(v);
}
void HypersawUnit_setDriftHPF(HypersawUnit* o, double v) {
    Highpass_setFrequency(&o->hpf, v);
}
void HypersawUnit_setDriftLPF(HypersawUnit* o, double v) {
    //Lowpass_setFrequency(&o->lpf, v);
}

void HypersawUnit_setVibratoSpeed(HypersawUnit* o, double v) {
    VibratoGenerator_setFrequency(&o->vibrato, v);
}
void HypersawUnit_setVibratoRandomSpeed(HypersawUnit* o, double v) {
    VibratoGenerator_setRandomSpeed(&o->vibrato, v);
}
void HypersawUnit_setVibratoRandomSpeedOffset(HypersawUnit* o, double v) {
    VibratoGenerator_setRandomSpeedOffset(&o->vibrato, v);
}
void HypersawUnit_setVibratoRange(HypersawUnit* o, double v) {
    VibratoGenerator_setRange(&o->vibrato, v);
}
void HypersawUnit_setVibratoRandomRange(HypersawUnit* o, double v) {
    VibratoGenerator_setRandomRange(&o->vibrato, v);
}
void HypersawUnit_setVibratoRandomRangeOffset(HypersawUnit* o, double v) {
    VibratoGenerator_setRandomRangeOffset(&o->vibrato, v);
}
void HypersawUnit_setVibratoDesync(HypersawUnit* o, double v) {
    VibratoGenerator_setPhaseDesync(&o->vibrato, v);
}

void HypersawUnit_triggerAttack(HypersawUnit* o) {
    HypersawUnit_updateAttack(o);
    HypersawUnit_updateDelay(o);

    //if (env.isIdle())
        //randomPhaseOffset = rand(-randomPhaseOffsetRange,randomPhaseOffsetRange);

    LinearEnvelope_triggerAttack(&o->env, 1, 1);
}

void HypersawUnit_triggerRelease(HypersawUnit* o) {
    HypersawUnit_updateRelease(o);
    LinearEnvelope_triggerRelease(&o->env);
}

void HypersawUnit_setRandomDelayRange(HypersawUnit* o, double v) {
    o->delaySecondsRandomRange = v;
    HypersawUnit_updateDelay(o);
}
void HypersawUnit_setAttackSeconds(HypersawUnit* o, double v) {
    o->attackSeconds = v;
    HypersawUnit_updateAttack(o);
}
void HypersawUnit_setAttackSecondsRandomMult(HypersawUnit* o, double v) {
    o->attackSecondsRandomMult = v;
    HypersawUnit_updateAttack(o);
}
void HypersawUnit_setReleaseSeconds(HypersawUnit* o, double v) {
    o->releaseSeconds = v;		
    HypersawUnit_updateRelease(o);
}
void HypersawUnit_setReleaseSecondsRandomMult(HypersawUnit* o, double v) {
    o->releaseSecondsRandomMult = v;
    HypersawUnit_updateRelease(o);
}

void HypersawUnit_updateDelay(HypersawUnit* o) {
    double randomDelay = NoiseGenerator_getSampleUni(&o->r);
    LinearEnvelope_setDelayTime(&o->env, randomDelay * o->idx * o->div * o->delaySecondsRandomRange);
}

void HypersawUnit_updateRelease(HypersawUnit* o) {
    double randomRelease = NoiseGenerator_getSampleUni(&o->r);
    double sec = o->releaseSeconds + randomRelease * o->releaseSecondsRandomMult;
    LinearEnvelope_setReleaseTime(&o->env, sec);
    LinearEnvelope_setDecayTime(&o->env, sec);
}
