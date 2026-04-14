#pragma once

#include "../semath.h"

double ellipsoidSineToSquare(double x, double sineToSquare)
{
	double TAU_x_X = TAU * x;
	double s = sin(TAU_x_X);
	double c = cos(TAU_x_X);

	x = pow(c, 2) + pow(s * sineToSquare, 2);

	return (c / sqrt(x));
}

typedef struct 
{
	double samplerate;
	double frequency;
	double scale;
	double offset;
	double phase;
	double phaseOffset;
	double increment;
	double shape;

	double B_sin;
	double B_cos;
} Ellipsoid;

void Ellipsoid_init(Ellipsoid* o)
{
	o->samplerate = 44100;
	o->frequency = 1;
	o->scale = 0;
	o->offset = 1;
	o->phase = 0;
	o->phaseOffset = 0;
	o->increment = 0;
	o->shape = 0;

	o->B_sin = 0;
	o->B_cos = 1;
}

void Ellipsoid_update(Ellipsoid* o)
{
	o->increment = o->frequency / samplerate;
}

// A = Offset, B = angle, C = scale
double Ellipsoid_getValue(double phase, double A, double B_sin, double B_cos, double C)
{
	double sin_x, cos_x;
	sin_x = sin(phase*TAU);
	cos_x = cos(phase*TAU);

	double sqrt_val = sqrt(pow(A + cos_x, 2) + pow(C*sin_x, 2));
	double output = ((A + cos_x)*B_cos + (C*sin_x)*B_sin) / sqrt_val;

	return output;
}

// shape is 0 to 1, 0 being sin, 1 being square
double Ellipsoid_getValueSinToSquare(double phase, double shape)
{
	shape = 1 - shape;
	return -Ellipsoid_getValue(phase, 0, 0, 1, shape);
}

// shape is -01 to 1, 0 being sin, 1 being ramp, -1 being saw
double Ellipsoid_getValueSinToSaw(double phase, double shape)
{
	phase = map0to1(phase, -.75, .25);
	return Ellipsoid_getValue(phase, -shape, sin_pi_x_1_p_5, cos_pi_x_1_p_5, 1);
}

double Ellipsoid_getSampleSinToSquare(Ellipsoid* o)
{
	o->phase += o->increment;
	o->phase = o->phase - floor(o->phase);
	return Ellipsoid_getValueSinToSquare(o->phase + o->phaseOffset, o->shape);
}

double Ellipsoid_getSampleSinToSaw(Ellipsoid* o)
{
	o->phase += o->increment;
	o->phase = o->phase - floor(o->phase);
	return Ellipsoid_getValueSinToSaw(o->phase + o->phaseOffset, o->shape);
}

void Ellipsoid_setFrequency(Ellipsoid* o, double v) 
{ 
	o->frequency = v; 
	Ellipsoid_update(o);
}

void Ellipsoid_setPhaseOffset(Ellipsoid* o, double v)
{
	o->phaseOffset = v;
}

void Ellipsoid_setSampleRate(Ellipsoid* o) 
{ 
	Ellipsoid_update(o); 
}

// -1 to +1
void Ellipsoid_setOffset(Ellipsoid* o, double v) 
{
	o->offset = v; 
}

// 0 to 1
void Ellipsoid_setAngle(Ellipsoid* o, double v) 
{
	o->B_sin = sin(v*PI);
	o->B_cos = cos(v*PI);
}

void Ellipsoid_setShape(Ellipsoid* o, double v)
{
	o->shape = v;
}

// 0 to inf
void Ellipsoid_setScale(Ellipsoid* o, double v)
{
	o->scale = v;
}

double Ellipsoid_getSample(Ellipsoid* o)
{
	o->phase += o->increment;
	o->phase = o->phase - floor(o->phase);
	return Ellipsoid_getValue(o->phase + o->phaseOffset, o->offset, o->B_sin, o->B_cos, o->scale);
}