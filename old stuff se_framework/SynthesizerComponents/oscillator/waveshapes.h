#pragma once

extern const double phase45degrees;
extern const double phase90degrees;
extern const double phase45degrees_sin;
extern const double phase45degrees_cos;

/* shapes expect a phase value from 0 to TAU (value will wrap)
* and output a value from -1 to +1 or 0 to 1 for uni functions */
namespace waveshape {

	double uniramp(double phase);

	double ramp(double phase);

	double saw(double phase);

	double sine(double phase);

	double cosine(double phase);

	double tri(double phase);

	double square(double phase);

	/* Morph between saw, tri, and ramp
	 * morph of 0 is saw
	 * morph of .5 i tri
	 * morph of 1 is ramp */
	double trisaw(double phase, double morph);

	double unitrisaw(double phase, double morph);

	// multiply angle by half pi for normalized range
	//double ellipse(double phase, double offset = 1, double angle_sin = 1, double angle_cos = 0, double scale = 1)

	/* Morphs between sine, square, saw, and many shapes inbetweem
		A is -1 to +1, default 0
		B is -PI to +PI, default 0
		C is -inf to +inf, default 1, relevant values around 0. */
	double ellipse(double phase, double A = 0, double B_sin = 0, double B_cos = 1, double C = 1);

} // namespace waveshape

namespace waveshape2D {

	void circle(double phase, double *outX, double *outY);
	void diamond(double phase, double * outX, double * outY);
	void square(double phase, double *outX, double *outY);
	void squarepoint(double phase, double *outX, double *outY);

} // namespace waveshape2D
