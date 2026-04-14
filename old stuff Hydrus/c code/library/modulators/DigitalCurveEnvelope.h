typedef struct DigitalCurveEnvelope {
    double /* Parameters */
        sustainAmp,
        delayTime,
        attackTime,
        decayTime,
        releaseTime,
        attackcurve,
        decaycurve,
        releasecurve;

	double /* Internal Values*/
		attackIncrement,
		decayDecrement,
		releaseDecrement;

	double /* Counters */
		currentValue,
		secondsPassed;

	bool isLooping;

	enum ENVSTATE envState;
} DigitalCurveEnvelope;

void DigitalCurveEnvelope_init(DigitalCurveEnvelope* o) {
    o->delayTime = 0;

    o->attackTime = .1;
    o->decayTime = .5;
    o->releaseTime = .5;

    o->sustainAmp = .5;

    o->attackcurve = .5;
    o->decaycurve = .5;
    o->releasecurve = .5;

    o->attackIncrement = 0;
    o->decayDecrement = 0;
    o->releaseDecrement = 0;

    o->currentValue = 0;
    o->secondsPassed = 0;

    o->isLooping = false;

    o->envState = ENV_OFF;
}

bool DigitalCurveEnvelope_isIdle(DigitalCurveEnvelope* o) {
    return o->envState == ENV_OFF; 
}

int DigitalCurveEnvelope_getEnvState(DigitalCurveEnvelope* o) { 
    return o->envState; 
}

// 1.e-6 to inf exponential
void DigitalCurveEnvelope_setDelayTime(DigitalCurveEnvelope* o, double v) {
    o->delayTime = v;
}

// 1.e-6 to inf exponential
void DigitalCurveEnvelope_setAttackTime(DigitalCurveEnvelope* o, double v) {
    o->attackTime = v;
    o->attackIncrement = min(sampleperiod / o->attackTime, 1.0);
}

// 1.e-6 to inf exponential
void DigitalCurveEnvelope_setDecayTime(DigitalCurveEnvelope* o, double v) {
    o->decayTime = v;
    o->decayDecrement = (1 - o->sustainAmp) * sampleperiod / o->decayTime;
}

// 1.e-6 to inf exponential
void DigitalCurveEnvelope_setReleaseTime(DigitalCurveEnvelope* o, double v) {
    o->releaseTime = v;
    o->releaseDecrement = o->currentValue * sampleperiod / o->releaseTime;
}

void DigitalCurveEnvelope_triggerRelease(DigitalCurveEnvelope* o) {
    o->envState = ENV_RELEASE;
    DigitalCurveEnvelope_setReleaseTime(o, o->releaseTime);
}

// 0 to 1
void DigitalCurveEnvelope_setSustainAmplitude(DigitalCurveEnvelope* o, double v) {
    o->sustainAmp = v;
    DigitalCurveEnvelope_setDecayTime(o, o->decayTime);
    DigitalCurveEnvelope_setReleaseTime(o, o->releaseTime);
}

void DigitalCurveEnvelope_setSampleRate(DigitalCurveEnvelope* o) {
    DigitalCurveEnvelope_setDelayTime(o, o->delayTime);
    DigitalCurveEnvelope_setAttackTime(o, o->attackTime);
    DigitalCurveEnvelope_setSustainAmplitude(o, o->sustainAmp);
}
void DigitalCurveEnvelope_reset(DigitalCurveEnvelope* o) {
    o->currentValue = 0;
    o->envState = ENV_OFF;
    o->secondsPassed = 0;
}

void DigitalCurveEnvelope_triggerAttack(DigitalCurveEnvelope* o, double velocity, bool updateVelScale) {
    /* skip delay stage is delay time is negligible
    * and skip attack stage if attack time is
    * negliglbe. */

    if (o->delayTime < sampleperiod)
    {
        if (o->attackTime <= sampleperiod)
        {
            o->envState = ENV_DECAY;
            o->currentValue = 1.0;						
        }
        else
        {
            o->envState = ENV_ATTACK;
        }
    }
    else
    {
        o->envState = ENV_DELAY;
    }
}

double DigitalCurveEnvelope_getSample(DigitalCurveEnvelope* o) {
    switch (o->envState)
    {
    case ENV_OFF:
        break;

    case ENV_DELAY:
        o->secondsPassed += sampleperiod;
        if (o->secondsPassed >= o->delayTime)
            o->envState = ENV_ATTACK;
        break;

    case ENV_ATTACK:
        o->currentValue += o->attackIncrement;
        
        if (o->currentValue >= 1)
        {
            o->currentValue = 1;
            o->envState = ENV_DECAY;
        }
        break;

    case ENV_DECAY:
        o->currentValue -= o->decayDecrement;
        if (o->currentValue <= o->sustainAmp)
        {
            o->currentValue = o->sustainAmp;
            o->envState = ENV_SUSTAIN;
        }
        break;

    case ENV_SUSTAIN:
        if (o->isLooping)
            o->envState = ENV_ATTACK;
        o->currentValue = o->sustainAmp;
        break;

    case ENV_RELEASE:
        o->currentValue -= o->releaseDecrement;
        if (o->currentValue <= 0)
            DigitalCurveEnvelope_reset(o);
        break;
    }

    return o->currentValue;
}

void DigitalCurveEnvelope_setLooping(DigitalCurveEnvelope* o, bool v) { 
    o->isLooping = v; 
    if (o->isLooping && (o->envState != ENV_ATTACK || o->envState != ENV_DECAY))
        DigitalCurveEnvelope_triggerAttack(o, 1, 1);
}