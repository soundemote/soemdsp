#pragma once

#include "../library/oscillators/NoiseGenerator.h"
#include "../library/filters/Lowpass.h"

typedef struct WalterRandomWalk
{
	double value;
	double stepSize;

	NoiseGenerator noise;

} WalterRandomWalk;

//void WalterRandomWalk_DBG(WalterRandomWalk* o)
//{
//	FUNC
//	LOGF(value)
//	LOGF(stepSize)
//}

void WalterRandomWalk_init(WalterRandomWalk* o, int seed)
{
	o->value = 0;
	o->stepSize = 0;
	NoiseGenerator_setSeed(o->noise.state, seed);
}

void WalterRandomWalk_reset(WalterRandomWalk* o)
{
	WalterRandomWalk_init(o, o->noise.seed);
}

void WalterRandomWalk_setSeed(WalterRandomWalk* o, int v)
{
	o->noise.state = o->noise.seed = v;
}

// bipolar values
double WalterRandomWalk_getSample(WalterRandomWalk* o)
{
	double r = NoiseGenerator_getSample(&o->noise);
	o->value = o->value + (r - o->value * o->stepSize) * o->stepSize;

	return o->value;
}

typedef struct DualWalterRandomWalk
{
	WalterRandomWalk walkA, walkB;
	Lowpass lpf;
	Highpass hpf;

	double ampA;
	double ampB;
	double stepSizeA;
	double stepSizeB;

} DualWalterRandomWalk;


//void DualWalterRandomWalk_DBG(DualWalterRandomWalk* o)
//{
//	LOGF(o->ampA)
//	LOGF(o->ampB)
//	LOGF(o->stepSizeA)
//	LOGF(o->stepSizeB)
//	LOGF(o->lpf.frequency)
//	LOGF(o->hpf.frequency)
//	WalterRandomWalk_DBG(&o->walkA);
//	WalterRandomWalk_DBG(&o->walkB);
//}

void DualWalterRandomWalk_init(DualWalterRandomWalk* o, int seed)
{
	o->ampA = 0.5;
	o->ampB = 0.5;
	o->stepSizeA = 0.5;
	o->stepSizeB = 0.5;
	WalterRandomWalk_init(&o->walkA, seed);
	WalterRandomWalk_init(&o->walkB, seed+97816);
	Lowpass_init(&o->lpf);
	Highpass_init(&o->hpf);
}

void DualWalterRandomWalk_setSampleRate(DualWalterRandomWalk* o)
{
	o->lpf.v[FLT_coeff].dw = Lowpass_calculate_b1(samplerate * sampleperiod);
	Lowpass_incrementChanged(&o->lpf);
	Highpass_setSampleRate(&o->hpf);
}

void DualWalterRandomWalk_reset(DualWalterRandomWalk* o)
{
	WalterRandomWalk_reset(&o->walkA);
	WalterRandomWalk_reset(&o->walkB);
}

void DualWalterRandomWalk_setSeed(DualWalterRandomWalk* o, int v)
{
	WalterRandomWalk_setSeed(&o->walkA, v+3948);
	WalterRandomWalk_setSeed(&o->walkB, v+23568);
}

void DualWalterRandomWalk_setStepSizeA(DualWalterRandomWalk* o, double v)
{
	o->walkA.stepSize = v;
}

void DualWalterRandomWalk_setStepSizeB(DualWalterRandomWalk* o, double v)
{
	o->walkB.stepSize = v;
}

// bipolar values
double DualWalterRandomWalk_getSample(DualWalterRandomWalk* o)
{
	double a = WalterRandomWalk_getSample(&o->walkA);
	double b = WalterRandomWalk_getSample(&o->walkB);

	return Lowpass_getSample(&o->lpf, a) * o->ampA + Highpass_getSample(&o->hpf, b) * o->ampB;
	//return a * ampA + b * ampB;
	//return a * o->mix + b * (1.0 - o->mix);
}