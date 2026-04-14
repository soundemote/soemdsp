#include "JerobeamBlubb.h"

void JerobeamBlubb::rotate(double in_x, double in_y, double in_z, double rot_x, double rot_y, double rot_z, double * out_l, double * out_r)
{
	double sin_rotx, cos_rotx, sin_roty, cos_roty, sin_rotz, cos_rotz;
	elan::sinCos(rot_x, &sin_rotx, &cos_rotx); /*optimization*/
	double help_11 = in_x*cos_rotx-in_y*sin_rotx;
	double help_12 = in_x*sin_rotx+in_y*cos_rotx;
	elan::sinCos(rot_y, &sin_roty, &cos_roty); /*optimization*/
	double help_21 = help_11*cos_roty-in_z*sin_roty;
	//double help_22 = help_11*sin_roty+in_z*cos_roty; //unused
	elan::sinCos(rot_z, &sin_rotz, &cos_rotz); /*optimization*/
	double help_31 = help_21*cos_rotz-help_12*sin_rotz;
	double help_32 = help_21*sin_rotz+help_12*cos_rotz;

	double x = help_31;
	double y = help_32;
	//double z = help_22; //unused

	//double rot_x_x_PI_x_2_sin, rot_x_x_PI_x_2_cos;
	//elan::sinCos(rot_x*PI_x_2, &rot_x_x_PI_x_2_sin, &rot_x_x_PI_x_2_cos); /*optimization*/
	//double help_11 = in_x*rot_x_x_PI_x_2_cos-in_y*rot_x_x_PI_x_2_sin;
	//double help_12 = in_x*rot_x_x_PI_x_2_sin+in_y*rot_x_x_PI_x_2_cos;

	//double rot_y_x_PI_x_2_sin, rot_y_x_PI_x_2_cos;
	//elan::sinCos(rot_y*PI_x_2, &rot_y_x_PI_x_2_sin, &rot_y_x_PI_x_2_cos); /*optimization*/
	//double help_21 = help_11*rot_y_x_PI_x_2_cos-in_z*rot_y_x_PI_x_2_sin;
	//double help_22 = help_11*rot_y_x_PI_x_2_sin+in_z*rot_y_x_PI_x_2_cos;

	//double x = help_21;
	//double y = help_12;
	//double z = help_22;

	double formula = zDepth*1.25*(in_z *.05+ 0.5);  /*optimization*/
	double L = x - formula*x;
	double R = y - formula*y;

	double m = 1 + zDepth;

	*out_l = L*m;
	*out_r = R*m;
}


void JerobeamBlubb::getSampleFrame(double* outL, double * outR)
{
	double phase_sin, phase_cos;

	phasor.increment();

	double chLeft, chRight;
	switch (shape)
	{
	case Shape::CIRCLE:
		elan::sinCos(phase, &phase_sin, &phase_cos);
		chLeft = phase_sin;
		chRight = phase_cos;
		break;
	case Shape::SQUARE: default:
		chLeft = waveshape::tri(phase+phase45degrees);
		chRight = waveshape::tri(phase+phase135degrees);
	}

	*outL = chLeft;
	*outR = chRight;
}
