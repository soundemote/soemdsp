
typedef struct
{
    double currentValue;
	double targetValue;
	double increment;
	double frequency;
	int periodInSamples;

} LinearSmoother;

//void LinearSmoother_DBG(LinearSmoother* o)
//{
//    LOGF(o->currentValue)
//    LOGF(o->targetValue)
//    LOGF(o->increment)
//    LOGF(o->frequency)
//    LOGF(o->numSamplesToTarget)
//}

void LinearSmoother_init(LinearSmoother* o)
{
    o->currentValue = 0;
	o->targetValue = 0;
	o->increment = 0;
	o->frequency = 50;
	o->periodInSamples = 0;    
}

bool LinearSmoother_needsSmoothing(LinearSmoother* o)
{
    return fabs(o->targetValue - o->currentValue) > 1.e-6; 
}

void LinearSmoother_updateNumSamplesToTarget(LinearSmoother* o)
{
    o->periodInSamples = floor(samplerate / o->frequency);
}

void LinearSmoother_updateIncrement(LinearSmoother* o)
{
    if (o->periodInSamples == 0)
        return;
    o->increment = (o->targetValue - o->currentValue) / (double)o->periodInSamples;
}

void  LinearSmoother_reset(LinearSmoother* o)
{
    o->currentValue = o->targetValue;
}

void  LinearSmoother_setInternalValue(LinearSmoother* o, double v)
{
    o->currentValue = o->targetValue = v;
}

void LinearSmoother_setSampleRate(LinearSmoother* o)
{
    LinearSmoother_updateNumSamplesToTarget(o);
    LinearSmoother_updateIncrement(o);		
}

void LinearSmoother_setFrequency(LinearSmoother* o, double v)
{
    o->frequency = v;

    LinearSmoother_updateNumSamplesToTarget(o);
    LinearSmoother_updateIncrement(o);
}

double LinearSmoother_getSample(LinearSmoother* o, double v)
{
    if (o->frequency <= 0)
        return o->currentValue;

    if (o->targetValue != v)
    {
        o->targetValue = v;
        LinearSmoother_updateIncrement(o);
    }

    if (!LinearSmoother_needsSmoothing(o))
    {
        return o->currentValue;
    }
    
    o->currentValue += o->increment;

    // if (o->frequency >= _3z4xSampleRate)
    //     o->currentValue = o->targetValue;

    return o->currentValue;
}