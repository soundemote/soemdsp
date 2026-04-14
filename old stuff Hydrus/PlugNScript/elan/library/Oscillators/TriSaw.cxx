#include "../math.cxx"
#include "Ellipse.cxx"

double uniTriSawReaktor(double phase, double morph)
{
	phase = wrapPhaseTAU((phase - .25)*TAU);
	morph *= TAU;

	double sourceRangeMin, sourceRangeMax, targetRangeMin, targetRange, sourceRange;

	if (phase > morph)
	{
		sourceRangeMin = morph;
		sourceRangeMax = TAU;
		targetRangeMin = 1;
		targetRange = -1;
		sourceRange = TAU - morph;
	}
	else
	{
		sourceRangeMin = 0;
		sourceRangeMax = morph;
		targetRangeMin = 0;
		targetRange = 1;
		sourceRange = morph - 0;
	}

	if (sourceRangeMin == sourceRangeMax)
		return sourceRangeMin;

	return -(targetRangeMin + (targetRange * (phase - sourceRangeMin)) / sourceRange) + 1;
}

// phase and morph 0 to 1 values
// This trisaw starts at -1 given phase 0 and shape 0
double unitrisaw(double phase, double morph)
{	
	phase = wrapPhaseTAU(phase * TAU);
	morph *= TAU;

	double sourceRangeMin, sourceRangeMax, targetRangeMin, targetRange, sourceRange;

	if (phase > morph)
	{
		sourceRangeMin = morph;
		sourceRangeMax = TAU;
		targetRangeMin = 1;
		targetRange = -1;
		sourceRange = TAU - morph;
	}
	else
	{
		sourceRangeMin = 0;
		sourceRangeMax = morph;
		targetRangeMin = 0;
		targetRange = 1;
		sourceRange = morph - 0;
	}

	if (sourceRangeMin == sourceRangeMax)
		return sourceRangeMin;

	return targetRangeMin + (targetRange * (phase - sourceRangeMin)) / sourceRange;
}

// phase and morph 0 to 1 values
double trisaw(double phase, double morph)
{
	return unipolarToBipolar(unitrisaw(phase, morph));
}

class TriSaw
{
    void reset()
    {
        phase = phaseOffset;
    }

	void setSampleRate(double v)
    {
        sampleRate = v;
        update();
    }

    void setPhase(double v)
    {
        phase = v;
    }

	void setFrequency(double v)
    {
        frequency = v;
        update();
    }

    void setPhaseOffset(double v)
    {
    	phaseOffset = v;
    }

	// 0 to .5 to +1 saw -> tri -> ramp
	void setShape(double v)
	{		
		shape = v;		
	}

	double getSample()
	{
		phase += increment;
		phase = phase - floor(phase);
		return getValue(phase + phaseOffset, shape);
	}

	double getValue(double p, double shape)
	{
		return trisaw(p, shape);
	}

    protected void update()
    {
        increment = frequency / sampleRate;
    }

	Ellipse ellipse;

	protected double sampleRate = 0;
	protected double phase = 0;
	protected double frequency = 0;
    protected double phaseOffset = 0;
    protected double increment = 0;
	protected double shape = 0;
};