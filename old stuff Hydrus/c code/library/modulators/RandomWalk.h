#pragma once

//#include "../oscillators/NoiseGenerator.h"

typedef struct RandomWalk
{
	double output;
	double stepSize;
    double jitter;
    double frequency;
    int    method;

    double whiteNoiseMix;
    double randomMix;
    double frequencyNormalized;

    double stepSizeTaper;
    double frequencyTaper;

	NoiseGenerator noiseGenerator;
    Lowpass lpf;    

} RandomWalk;

void RandomWalk_DBG(RandomWalk* o)
{
	LOGF(o->output)
	LOGF(o->stepSize)
    LOGF(o->jitter)
    LOGF(o->frequency)
    LOGI(o->method)
    LOGF(o->whiteNoiseMix)
    LOGF(o->randomMix)
    LOGF(o->frequencyNormalized)
    LOGF(o->stepSizeTaper)
    LOGF(o->frequencyTaper)
}

void RandomWalk_init(RandomWalk* o)
{
	o->output = 0;
	o->stepSize = 0;
    o->method = 0;
    o->frequency = 0;
    o->frequencyNormalized = 0;
    o->jitter = 0;

    o->frequencyTaper = -0.996;
    o->stepSizeTaper = -0.638;

    NoiseGenerator_init(&o->noiseGenerator);
    Lowpass_init(&o->lpf);
}

void RandomWalk_reset(RandomWalk* o)
{
    o->output = 0;
    NoiseGenerator_reset(&o->noiseGenerator);
    Lowpass_reset(&o->lpf);
}

void RandomWalk_updateStepSize(RandomWalk* o)
{  
    o->stepSize  = rationalCurve(clamp(o->frequencyNormalized, 0, 1), o->stepSizeTaper);

    o->stepSize += map0to1(rationalCurve(o->jitter, -.99), -o->stepSize, 1 - o->stepSize);    
}

void RandomWalk_updateWhiteNoiseMix(RandomWalk* o)
{
	double colorJitterAvg = (o->jitter + o->frequencyNormalized) * .5;

	double startWhiteNoiseMix = 0.9;  

	o->whiteNoiseMix = 0;
    if (colorJitterAvg >= startWhiteNoiseMix)  
        o->whiteNoiseMix = rationalCurve(map(colorJitterAvg, startWhiteNoiseMix, 1, 0, 1), -.7);        

    o->randomMix = 1 - o->whiteNoiseMix;
}

void RandomWalk_updateIncrement()
{
    
}

void RandomWalk_updateFrequency(RandomWalk* o)
{
    o->frequency = map0to1(rationalCurve(o->frequencyNormalized, o->frequencyTaper), 0, samplerate);

   o->lpf.v[FLT_coeff].dw = exp(-TAU * *o->lpf.v[FLT_increment].dr * samplerate);

    Lowpass_incrementChanged(&o->lpf);

    RandomWalk_updateStepSize(o);	
}

void RandomWalk_setSampleRate(RandomWalk* o)
{
    RandomWalk_updateFrequency(o);
}

void RandomWalk_setSeed(RandomWalk* o, int v)
{
    NoiseGenerator_setSeed(&o->noiseGenerator, v);
}

// speed of value changes  from 0 to 1, but that is converted to 0 to samplerate
void RandomWalk_setColor(RandomWalk* o, double v)
{
    o->frequencyNormalized = pow(v, 2);

	RandomWalk_updateWhiteNoiseMix(o);

    RandomWalk_updateFrequency(o);
}

// depth of value changes, can be from 0 to 1, negative values are ok
void RandomWalk_setJitter(RandomWalk* o, double v)
{
    o->jitter = v;

    RandomWalk_updateWhiteNoiseMix(o);

    RandomWalk_updateStepSize(o);
}

// bipolar values
double RandomWalk_getSample(RandomWalk* o)
{
    double n = NoiseGenerator_getSample(&o->noiseGenerator);
    double r = 0;

    switch((int)o->method)
    {
    default:
    case 0: // random steps
        r = n * o->stepSize;
        break;
    case 1: // fixed steps
        r = n > 0 ? o->stepSize : -o->stepSize;
        break;
    }
   
    o->output = clamp(o->output + r, -1.0, 1.0);

    return Lowpass_getSample(&o->lpf, o->output * o->randomMix + n * o->whiteNoiseMix);
}