#pragma once

// Linear Feedback Envelope

typedef struct PluckEnvelope
{
	double decayIncrement;
	double releaseIncrement;

	double fbAttack;
	double fbDecay;
    double decay;

    double finalDecayMod;
    double decayModStart;
    double decayModEnd;
    double endingDecay;
    double decayModCurve;

	double fbRelease;

	double currentValue;
	double secondsPassed;

	double phasor;
	double phasorIncrement;

	int envState;
} PluckEnvelope;

void PluckEnvelope_init(PluckEnvelope* o) {
	o->decayIncrement = 0;
	o->releaseIncrement = 0;
	o->fbAttack = 0;
	o->fbDecay = 0;
	o->decay = 0;
	o->finalDecayMod = 0;
	o->decayModStart = 0;
	o->decayModEnd = 0;
	o->endingDecay = 0;
	o->decayModCurve = 0;
	o->fbRelease = 0;
	o->currentValue = 0;
	o->secondsPassed = 0;
	o->phasor = 0;
	o->phasorIncrement = 0;
    o->envState = ENV_OFF;
}

void PluckEnvelope_updateDecayIncrement(PluckEnvelope* o) {
    o->decayIncrement = (o->currentValue-1) * sampleperiod / 50;
}

void PluckEnvelope_updateReleaseIncrement(PluckEnvelope* o) {
    o->releaseIncrement = o->currentValue * sampleperiod / 50;
}

void PluckEnvelope_updateDecayFeedback(PluckEnvelope* o) {
    o->fbDecay = min(.999999, exp(-(o->finalDecayMod)*10));
}

void PluckEnvelope_setRelease(PluckEnvelope* o, double v) { 
    o->fbRelease = min(.999999, exp(-v*10));
}

bool PluckEnvelope_isIdle(PluckEnvelope* o) {
    return o->envState == ENV_OFF;
}

void PluckEnvelope_setSampleRate(PluckEnvelope* o) {
    PluckEnvelope_updateDecayIncrement(o);
    PluckEnvelope_updateReleaseIncrement(o);        
}

void PluckEnvelope_reset(PluckEnvelope* o) {
    o->currentValue = 0;
    o->envState = ENV_OFF;
    o->secondsPassed = 0;
    o->phasor = 0;
}

void PluckEnvelope_triggerAttack(PluckEnvelope* o, double velocity) {
    o->currentValue = velocity;
    o->envState = ENV_DECAY;
    o->phasor = 0;
    PluckEnvelope_updateDecayIncrement(o);   
}

void PluckEnvelope_triggerRelease(PluckEnvelope* o) {
    o->envState = ENV_RELEASE;
    PluckEnvelope_updateReleaseIncrement(o);
}

double PluckEnvelope_getSample(PluckEnvelope* o) {
    if (o->phasor >= 1)
    {
        o->finalDecayMod = o->endingDecay;
        PluckEnvelope_updateDecayFeedback(o);
    }
    else
	{
        o->finalDecayMod = o->decay + map0to1(rationalCurve(o->phasor, o->decayModCurve), o->decayModStart, o->decayModEnd);
        PluckEnvelope_updateDecayFeedback(o);
    }

    switch (o->envState)
    {
    case ENV_OFF:
        break;

    case ENV_DECAY:
        o->currentValue -= o->decayIncrement + o->currentValue*o->currentValue*o->fbDecay;
        if (o->currentValue < 0)
            PluckEnvelope_reset(o);            
        break;

    case ENV_RELEASE:
        o->currentValue -= o->releaseIncrement + o->currentValue*o->currentValue*o->fbRelease;
        if (o->currentValue <= 0)
            PluckEnvelope_reset(o);
        break;
    }

    o->phasor += o->phasorIncrement;

    return o->currentValue;
}

void PluckEnvelope_setDecay(PluckEnvelope* o, double v) { 
    o->decay = v;
}
void PluckEnvelope_setDecayModStart(PluckEnvelope* o, double v) {
    o->decayModStart = v;
}
void PluckEnvelope_setDecayModEnd(PluckEnvelope* o, double v) {
    o->decayModEnd = v;
}
void PluckEnvelope_setDecayModFrequency(PluckEnvelope* o, double v) {
    o->phasorIncrement = v / samplerate;
}
void PluckEnvelope_setDecayModCurve(PluckEnvelope* o, double v) {
   o->decayModCurve = v;
}
void PluckEnvelope_setFinalDecay(PluckEnvelope* o, double v) {
    o->endingDecay = v;
}
