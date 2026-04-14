#pragma once

/*
Taken from RS-MET library, this is a soft clipper.
*/
typedef struct SoftClipper
{
	double center;
	double width;
	double scaleX;
    double scaleY;  // scale factors for input and output
	double shiftX;
    double shiftY;  // shift offsets for input and output
} SoftClipper;

void SoftClipper_init(SoftClipper* o)
{
	o->center = 0;
	o->width  = 2;
	o->scaleX = 1;
	o->scaleY = 1;  // scale factors for input and output
	o->shiftX = 0;
	o->shiftY = 0;  // shift offsets for input and output
}

// Updates the coefficients from the desired center and width.
void SoftClipper_updateCoeffs(SoftClipper* o)
{
	o->scaleX =  2 / o->width;
	o->shiftX = -1 - (o->scaleX * (o->center - 0.5 * o->width));
	o->scaleY =  1 / o->scaleX;
	o->shiftY = -o->shiftX * o->scaleY;
}

// Sets the center value. This is the point where x = y = center (assuming the prototype 
// function goes through the origin).
void SoftClipper_setCenter(SoftClipper* o, double v)
{
	o->center = v;
	SoftClipper_updateCoeffs(o);
}

// Sets the width. The output values of the function will be bounded by center +- width/2. 
// newWidth should be > 0.
void SoftClipper_setWidth(SoftClipper* o, double v)
{
	o->width = v;
	SoftClipper_updateCoeffs(o);
}

// Computes the output value for the given input x.
double SoftClipper_getSample(SoftClipper* o, double v)
{
	return o->shiftY + o->scaleY * tanh(o->scaleX * v + o->shiftX);
}

