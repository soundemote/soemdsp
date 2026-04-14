typedef struct {
	double /* Parameters */
		sustainAmp,
		delayTime,
		attackTime,
		decayTime,
		releaseTime;

	double /* Internal Values*/
		attackIncrement,
		decayDecrement,
		releaseDecrement;

	double /* Counters */
		currentValue,
		secondsPassed;

	bool isLooping;

	enum ENVSTATE envState;
} LinearEnvelope;

void LinearEnvelope_init(LinearEnvelope* o) {
    o->sustainAmp = 1;
    o->delayTime = 0;
    o->attackTime = 0;
    o->decayTime = 1;
    o->releaseTime = 1;

    o->attackIncrement = 0;
    o->decayDecrement = 0;
    o->releaseDecrement = 0;

    o->currentValue = 0;
    o->secondsPassed = 0;

    o->isLooping = false;

    o->envState = ENV_OFF;
}

bool LinearEnvelope_isIdle(LinearEnvelope* o) {
    return o->envState == ENV_OFF; 
}

int LinearEnvelope_getEnvState(LinearEnvelope* o) { 
    return o->envState; 
}

// 1.e-6 to inf exponential
void LinearEnvelope_setDelayTime(LinearEnvelope* o, double v) {
    o->delayTime = v;
}

// 1.e-6 to inf exponential
void LinearEnvelope_setAttackTime(LinearEnvelope* o, double v) {
    o->attackTime = v;
    o->attackIncrement = min(sampleperiod / o->attackTime, 1.0);
}

// 1.e-6 to inf exponential
void LinearEnvelope_setDecayTime(LinearEnvelope* o, double v) {
    o->decayTime = v;
    o->decayDecrement = (1 - o->sustainAmp) * sampleperiod / o->decayTime;
}

// 1.e-6 to inf exponential
void LinearEnvelope_setReleaseTime(LinearEnvelope* o, double v) {
    o->releaseTime = v;
    o->releaseDecrement = o->currentValue * sampleperiod / o->releaseTime;
}

void LinearEnvelope_triggerRelease(LinearEnvelope* o) {
    o->envState = ENV_RELEASE;
    LinearEnvelope_setReleaseTime(o, o->releaseTime);
}

// 0 to 1
void LinearEnvelope_setSustainAmplitude(LinearEnvelope* o, double v) {
    o->sustainAmp = v;
    LinearEnvelope_setDecayTime(o, o->decayTime);
    LinearEnvelope_setReleaseTime(o, o->releaseTime);
}

void LinearEnvelope_setSampleRate(LinearEnvelope* o) {
    LinearEnvelope_setDelayTime(o, o->delayTime);
    LinearEnvelope_setAttackTime(o, o->attackTime);
    LinearEnvelope_setSustainAmplitude(o, o->sustainAmp);
}
void LinearEnvelope_reset(LinearEnvelope* o) {
    o->currentValue = 0;
    o->envState = ENV_OFF;
    o->secondsPassed = 0;
}

void LinearEnvelope_triggerAttack(LinearEnvelope* o, double velocity, bool updateVelScale) {
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

double LinearEnvelope_getSample(LinearEnvelope* o) {
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
            LinearEnvelope_reset(o);
        break;
    }

    return o->currentValue;
}

void LinearEnvelope_setLooping(LinearEnvelope* o, bool v) { 
    o->isLooping = v; 
    if (o->isLooping && (o->envState != ENV_ATTACK || o->envState != ENV_DECAY))
        LinearEnvelope_triggerAttack(o, 1, 1);
}