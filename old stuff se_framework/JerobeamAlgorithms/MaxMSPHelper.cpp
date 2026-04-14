#include "MaxMSPHelper.h"

double gen::clip(double in, double min, double max)
{
	return RAPT::rsClip(in, min, max);
}

double gen::sign(double in)
{
	return RAPT::rsSign(in);
}

double gen::phasor(double f)
{
	jassert(curPhasorIdx < phasors.size());
	/* You forgot to call run_gen() or you 
	didn't instantiate enough phasors!! DOOD. 
	srrisly. */

	phasors[curPhasorIdx].setFrequency(f);
	double ret = phasors[curPhasorIdx].getCurrentValue();
	++curPhasorIdx;
	return ret;
}

void gen::run_gen()
{
	for (auto & p : phasors) 
		p.incrementSample();

	curPhasorIdx = 0;
	curChangeIdx = 0;
}

double gen::triangle(double x, double y)
{
	return waveshape::unitrisaw(x, y);
}

int gen::change(double v)
{
	jassert(curChangeIdx < last_change_value.size());
	/* You ran out of change() calls. How? You probably forgot
	to call run_gen(). Or you'll have to edit the class and
	increase the number of last_change_value indexes. */

	int ret = 0;

	if (last_change_value[curChangeIdx] > v)
		ret = 1;
	else if (last_change_value[curChangeIdx] < v)
		ret = -1;

	last_change_value[curChangeIdx] = v;
	
	++curChangeIdx;

	return ret;
}
