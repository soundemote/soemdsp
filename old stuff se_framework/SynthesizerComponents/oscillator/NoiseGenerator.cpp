#include "NoiseGenerator.h"

namespace elan
{
void NoiseGenerator::setSeed(int v)
{
	seed = v;
}

void NoiseGenerator::reset()
{
	state = seed;
}

double NoiseGenerator::getSampleBipolar()
{
	state = (1664525 * state + 1013904223) & 4294967295;
	return 2.0 / 4294967296.0 * state - 1;
}

double NoiseGenerator::getSampleUnpiolar()
{
	state = (1664525 * state + 1013904223) & 4294967295;
	return 1.0 / 4294967296.0 * state;
}
}//namespace elan
