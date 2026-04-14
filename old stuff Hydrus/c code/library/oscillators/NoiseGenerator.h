#pragma once

#include <stdint.h>

/*
Deterministic seed-based noise generator.
*/

const double _2_div_4294967296 = 2.0 / 4294967296.0;
const double _1_div_4294967296 = 1.0 / 4294967296.0;

typedef struct NoiseGenerator
{
	int seed;
	uint64_t state;
	double value;
} NoiseGenerator;

void NoiseGenerator_DBG(NoiseGenerator* o)
{
	FUNC
	LOGF(o->seed)
	LOGF(o->state)
	LOGF(o->value)
}

void NoiseGenerator_init(NoiseGenerator* o)
{
	o->seed = 39047;
	o->state = 0;
	o->value = 0;
}

void NoiseGenerator_setSeed(NoiseGenerator* o, int v)
{
	o->state = o->seed = v;
}

void NoiseGenerator_reset(NoiseGenerator* o)
{
	o->state = o->seed;
}

double NoiseGenerator_getSample(NoiseGenerator* o)
{
	o->state = (1664525 * o->state + 1013904223) & 4294967295;

	return o->value = _2_div_4294967296 * o->state - 1;		
}

double NoiseGenerator_getSampleUni(NoiseGenerator* o)
{
	o->state = (1664525 * o->state + 1013904223) & 4294967295;

	return o->value = _1_div_4294967296 * o->state;
}