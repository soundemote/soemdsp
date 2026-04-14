#include "XoxosOscillator.h"

double XoxosOscillator::getSample()
{
	double x, y, t = s0;

	s0 = s0 * w0 + s1 * w1;
	s1 = s1 * w0 - t * w1;

	x = s0 * scale;
	t = s1 + offset;

	y = t * m0 - x * m1;
	x = x * m0 + t * m1;

	return y / sqrt(x * x + y * y);
}
