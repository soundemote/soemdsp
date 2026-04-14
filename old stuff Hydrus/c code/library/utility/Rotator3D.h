#pragma once

#include "../math.h"

typedef struct Rotator3D {
	double rx, ry, rz; // rotation values from 0 to 1, angles around x, y and z-axis
	double xx, xy, xz, yx, yy, yz, zx, zy, zz; // matrix coeffs
} Rotator3D;

/** Updates the rotation matrix coefficients. */
void Rotator3D_updateCoeffs(Rotator3D* o) {
	// sines/cosines:
	double sx = sin(o->rx); 
	double cx = cos(o->rx);

	double sy = sin(o->ry); 
	double cy = cos(o->ry);

	double sz = sin(o->rz); 
	double cz = cos(o->rz);

	// rotation matrix coeffs:
	o->xx =  cz * cy;
	o->xy = -sz * cx + cz * sy * sx;
	o->xz =  sz * sx + cz * sy * cx;
	o->yx =  sz * cy;
	o->yy =  cz * cx + sz * sy * sx;
	o->yz = -cz * sx + sz * sy * cx;
	o->zx = -sy;
	o->zy =  cy * sx;
	o->zz =  cy * cx;
}

void Rotator3D_init(Rotator3D* o) {
	o->rx = 0;
	o->ry = 0;
	o->rz = 0;
	Rotator3D_updateCoeffs(o);
}

// call Rotator3D_updateCoeffs afterwards
void Rotator3D_setAngleX(Rotator3D* o, double X) {
	o->rx = X * TAU;
}

// call Rotator3D_updateCoeffs afterwards
void Rotator3D_setAngleY(Rotator3D* o, double Y) {
	o->ry = Y * TAU;
}

// call Rotator3D_updateCoeffs afterwards
void Rotator3D_setAngleZ(Rotator3D* o, double Z) {
	o->rz = Z * TAU;
}

// Rotator3D_updateCoeffs is called
void Rotator3D_setAngles(Rotator3D* o, double X, double Y, double Z) {
	o->rx = X * TAU;
	o->ry = Y * TAU;
	o->rz = Z * TAU;
	Rotator3D_updateCoeffs(o);
}

/** Applies the rotation matrix to the coordinate values. */
void Rotator3D_getSample(Rotator3D* o, double* x, double* y, double* z) {
	// temporaries:
	double X = *x;
	double Y = *y;
	double Z = *z;

	// new vector is given by matrix-vector product:
	*x = o->xx * X + o->xy * Y + o->xz * Z;    // |x|   |xx xy xz|   |X|
	*y = o->yx * X + o->yy * Y + o->yz * Z;    // |y| = |yx yy yz| * |Y|
	*z = o->zx * X + o->zy * Y + o->zz * Z;    // |z|   |zx zy zz|   |Z|
}