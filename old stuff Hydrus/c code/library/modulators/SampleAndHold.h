/* Linearly smoothed S&H that outputs bipolar values. */

//#include "../utility/LinearSmoother.h"

typedef struct SampleAndHold
{	
	NoiseGenerator noise;
	LinearSmoother smoother;

	double noiseValue;
	double smoothedValue;
	double sampledValue;
	double lastSampledValue;
    double phase;
    double increment;
    double frequency;
} SampleAndHold;

//SampleAndHold_DBG(SampleAndHold* o)
//{
//    LogFunc(__func__);
//
//    LogMsgF("o->noiseValue", o->noiseValue);
//    LogLine();
//    LogMsgF("o->smoothedValue", o->smoothedValue);
//    LogLine();
//    LogMsgF("o->sampledValue", o->sampledValue);
//    LogLine();
//    LogMsgF("o->phase", o->phase);
//    LogLine();
//    LogMsgF("o->increment", o->increment);
//    LogLine();
//    LogMsgF("o->frequency", o->frequency);
//    LogLine();
//}

SampleAndHold_init(SampleAndHold* o)
{
    NoiseGenerator_init(&o->noise);
    LinearSmoother_init(&o->smoother);

    o->noiseValue = 0;
    o->smoothedValue = 0;
    o->sampledValue = 0;
    o->phase = 1;
    o->increment = 0;
    o->frequency = 0;
}

void SampleAndHold_update(SampleAndHold* o)
{
    o->increment = o->frequency / samplerate;
}

void SampleAndHold_setSampleRate(SampleAndHold* o)
{
    LinearSmoother_setSampleRate(&o->smoother);
    SampleAndHold_update(o);
}

void SampleAndHold_reset(SampleAndHold* o)
{
    NoiseGenerator_reset(&o->noise);
    LinearSmoother_setInternalValue(&o->smoother, o->sampledValue);    
    o->phase = 1;
}

void SampleAndHold_trigger(SampleAndHold* o)
{
    o->phase = wrapPhase(o->phase);
    o->sampledValue = NoiseGenerator_getSample(&o->noise);
}

void SampleAndHold_increment(SampleAndHold* o)
{
    if (o->phase >= 1)
        SampleAndHold_trigger(o);

    o->phase += o->increment;
    
    o->smoothedValue = LinearSmoother_getSample(&o->smoother, o->sampledValue);
}

double SampleAndHold_getSampledValue(SampleAndHold* o)  
{
    return o->sampledValue;  
}
double SampleAndHold_getSmoothedValue(SampleAndHold* o) 
{
    return o->smoothedValue; 
}
double SampleAndHold_getNoiseValue(SampleAndHold* o) 
{
    return o->noiseValue = NoiseGenerator_getSample(&o->noise); 
}

void SampleAndHold_setSeed(SampleAndHold* o, int v)
{
    NoiseGenerator_setSeed(&o->noise, v);
}

void SampleAndHold_setFrequency(SampleAndHold* o, double v)
{
    o->frequency = v;
    LinearSmoother_setFrequency(&o->smoother, v);
    SampleAndHold_update(o);
}

