//#include "../oscillators/Ellipse.h"
//#include "../modulators/SampleAndHold.h"

typedef struct
{
    Ellipsoid ellipsoid;
    SampleAndHold SnHSpeed;
    SampleAndHold SnHRange;

    // 0 being zero phase, 1 being random phase
    double phaseSpreadValue;
    double phaseSpreadMult;
    double phaseDesyncValue;
    double phaseDesyncMult;
    double finalPhaseSpreadDesync; 

    // ph is phase, set shapeA and shapeB before calling
    double lastVal;

	double samplerate;
	double phase;
	double frequency;
    double fm;
    double pm;
    double increment;
    double range;
    double finalPhaseOffsetFromRandom;

    double shapeA;
    double shapeB;
    double shapeBSin;
    double shapeBCos;
    double shapeC;

    double randomSpeedMult;
    double randomRangeMult;

    double rangeRandomOffset;
    double speedRandomOffset;
    double randomSpeedOffsetMult;
    double randomRangeOffsetMult;
}  VibratoGenerator;

void VibratoGenerator_init(VibratoGenerator* o)
{
    Ellipsoid_init(&o->ellipsoid);
    SampleAndHold_init(&o->SnHSpeed);
    SampleAndHold_init(&o->SnHRange);

    o->phaseSpreadValue = 0;
    o->phaseSpreadMult = 0;
    o->phaseDesyncValue = 0;
    o->phaseDesyncMult = 0;
    o->finalPhaseSpreadDesync = 0;
    o->lastVal = 0;

    o->samplerate = 44100;
    o->phase = 0;
    o->frequency = 0;
    o->fm = 0;
    o->pm = 0;
    o->increment = 0;
    o->range = 0;
    o->finalPhaseOffsetFromRandom = 0;

    o->shapeA = 0;
    o->shapeB = 0;
    o->shapeBSin = 0;
    o->shapeBCos = 1;
    o->shapeC = 1;

    o->randomSpeedMult = 0;
    o->randomRangeMult = 0;

    o->rangeRandomOffset = 0;
    o->speedRandomOffset = 0;
    o->randomSpeedOffsetMult = 0;
    o->randomRangeOffsetMult = 0;
}

void VibratoGenerator_update(VibratoGenerator* o)
{          
    o->increment = (o->frequency + o->fm) / samplerate;
}

double VibratoGenerator_getValue(VibratoGenerator* o, double phase)
{
    // return Ellipsoid_getValue(&o->ellipsoid, o->ellipsoid.ph, o->ellipsoid.shapeA, 
    //     o->ellipsoid.shapeBSin, o->ellipsoid.shapeBCos, o->ellipsoid.shapeC);
    return sin(phase * TAU);
}

// set the seed for both freq and amp randomizers
void VibratoGenerator_setSeed(VibratoGenerator* o, int a, int b)
{
    SampleAndHold_setSeed(&o->SnHSpeed, a);
    SampleAndHold_setSeed(&o->SnHRange, b);

    NoiseGenerator r;
    NoiseGenerator_setSeed(&r, a);

    o->speedRandomOffset = NoiseGenerator_getSample(&r);
    o->rangeRandomOffset = NoiseGenerator_getSample(&r);
    o->phaseDesyncValue = NoiseGenerator_getSampleUni(&r);
}

void VibratoGenerator_reset(VibratoGenerator* o)
{
    o->phase = o->pm;
}

void VibratoGenerator_updateSpreadDesyncPhase(VibratoGenerator* o)
{
    o->finalPhaseSpreadDesync = o->phaseSpreadValue * o->phaseSpreadMult + o->phaseDesyncValue * o->phaseDesyncMult;
}
void VibratoGenerator_setPhaseSpread(VibratoGenerator* o, double v)
{
    o->phaseSpreadMult = v;
    VibratoGenerator_updateSpreadDesyncPhase(o);
}
void VibratoGenerator_setPhaseDesync(VibratoGenerator* o, double v)
{
    o->phaseDesyncMult = v;
    VibratoGenerator_updateSpreadDesyncPhase(o);
}

void VibratoGenerator_setSampleRate(VibratoGenerator* o)
{
    SampleAndHold_setSampleRate(&o->SnHSpeed);
    SampleAndHold_setSampleRate(&o->SnHRange);
    VibratoGenerator_update(o);
}

void VibratoGenerator_setPhase(VibratoGenerator* o, double v)
{
    o->phase = v;
}

void VibratoGenerator_setPM(VibratoGenerator* o, double v)
{
    o->pm = v;
}

void VibratoGenerator_setFrequency(VibratoGenerator* o, double v)
{        
    o->frequency = v;
    SampleAndHold_setFrequency(&o->SnHSpeed, v);
    SampleAndHold_setFrequency(&o->SnHRange, v);
    VibratoGenerator_update(o);
}

void VibratoGenerator_setFM(VibratoGenerator* o, double v)
{
    o->fm = v;
    VibratoGenerator_update(o);
}

void VibratoGenerator_setRandomSpeedOffset(VibratoGenerator* o, double v)
{
    o->randomSpeedOffsetMult = v;
}
void VibratoGenerator_setRandomRangeOffset(VibratoGenerator* o, double v)
{
    o->randomRangeOffsetMult = v;
}

// a is -1 to +1, b is -1 to +1
void VibratoGenerator_setShapeA(VibratoGenerator* o, double v)
{
    o->shapeA = v;
}

void VibratoGenerator_setShapeB(VibratoGenerator* o, double v)
{
    o->shapeB = v * TAU;

    o->shapeBSin = sin(o->shapeB);
    o->shapeBCos = cos(o->shapeB);
}

void VibratoGenerator_setShapeC(VibratoGenerator* o, double v)
{
    o->shapeC = v;
}

void VibratoGenerator_setRange(VibratoGenerator* o, double v)
{
    o->range = v;
}

void VibratoGenerator_setRandomSpeed(VibratoGenerator* o, double v)
{
    o->randomSpeedMult = v;
}

void VibratoGenerator_setRandomRange(VibratoGenerator* o, double v)
{
    o->randomRangeMult = v;
}

double VibratoGenerator_getSample(VibratoGenerator* o)
{   
    o->phase += o->increment;
    o->phase = o->phase - floor(o->phase);

    SampleAndHold_increment(&o->SnHSpeed);

    double spdSmoothVal =  SampleAndHold_getSmoothedValue(&o->SnHSpeed) * o->randomSpeedMult;
    double spdOffsetVal =  o->speedRandomOffset * o->randomSpeedOffsetMult;

    VibratoGenerator_setPM(o, o->finalPhaseOffsetFromRandom + spdSmoothVal + spdOffsetVal);
    SampleAndHold_increment(&o->SnHRange);

    return VibratoGenerator_getValue(o, o->phase + o->pm + o->finalPhaseSpreadDesync) * o->range + (SampleAndHold_getSmoothedValue(&o->SnHRange) * o->randomRangeMult * o->range + o->rangeRandomOffset * o->randomRangeOffsetMult * o->range);
}



