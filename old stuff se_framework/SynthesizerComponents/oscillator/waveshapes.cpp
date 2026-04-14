#include "waveshapes.h"

const double phase90degrees = degreesToPhase(90);

const double phase45degrees = degreesToPhase(45);
const double phase45degrees_sin = sin(phase45degrees);
const double phase45degrees_cos = cos(phase45degrees);

const double phase135degrees = degreesToPhase(135);
const double phase135degrees_sin = sin(phase135degrees);
const double phase135degrees_cos = cos(phase135degrees);

namespace waveshape {

	double uniramp(double phase)
	{
		return wrapPhase(phase);
	}

	double ramp(double phase)
	{
		return unipolarToBipolar(wrapPhase(phase));
	}

	double sine(double phase)
	{
		return sin(phase * TAU);
	}

	double cosine(double phase)
	{
		return cos(phase * TAU);
	}

	double tri(double phase)
	{
		double x = wrapPhaseTAU(phase*TAU);

		if (x < PI_z_2)
			return x*PI_INV_x_2;
		else if (x < PI_x_1p5)
			return 1.0 - (x-PI_z_2)*PI_INV_x_2;
		else
			return -1.0 + (x-PI_x_1p5)*PI_INV_x_2;

		//// another method:
		//double out = -1.0 + (2.0 * phase * TAU_INV);
		//return 2.0 * (fabs(out) - 0.5);
	}

	double saw(double phase)
	{
		return -ramp(phase);
	}

	double square(double phase)
	{
		double x = wrapPhase(phase);

		if (x < 0.5)
			return -1.0;
		else
			return +1.0;
	}

	double trisaw(double phase, double morph)
	{
		return unipolarToBipolar(unitrisaw(phase, morph));
	}

	double unitrisaw(double phase, double morph)
	{
		phase = wrapPhaseTAU(phase*TAU);
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


	//// multiply angle by half pi for normalized range
	//double ellipse(double phase, double offset, double angle_sin, double angle_cos, double scale)
	//{
	//	double s0, s1;
	//	elan::sinCos(phase*TAU, &s0, &s1);

	//	double x = s0 * scale;
	//	double t = s1 * offset;

	//	double out = (t * angle_sin + scale * angle_cos) / sqrt(t * t + x * x);

	//	return out;
	//}

	double ellipse(double phase, double A, double B_sin, double B_cos, double C)
	{
		double sin_x, cos_x;
		elan::sinCos(phase*TAU, &sin_x, &cos_x);

		double sqrt_val = sqrt(pow(A + cos_x, 2) + pow(C*sin_x, 2));
		double out = ((A + cos_x)*B_cos + (C*sin_x)*B_sin) / sqrt_val;

		return out;
	}

} //namespace waveshape

namespace waveshape2D {

	void circle(double phase, double * outX, double * outY)
	{
		elan::sinCos(phase, outX, outY);
	}

	void diamond(double phase, double * outX, double * outY)
	{
		*outX = waveshape::tri(phase);
		*outY = waveshape::tri(phase+phase90degrees);
	}

	void square(double phase, double * outX, double * outY)
	{
		diamond(phase, outX, outY);
		rotate2D(phase45degrees_sin, phase45degrees_cos, outX, outY);
	}

	void squarepoint(double phase, double* outX, double* outY)
	{
		*outX = waveshape::square(phase);
		*outY = waveshape::square(phase+phase90degrees);
	}

} //namespace waveshape2D
