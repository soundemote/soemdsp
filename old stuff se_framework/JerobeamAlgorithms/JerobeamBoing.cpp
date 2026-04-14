#include "JerobeamBoing.h"

void JerobeamBoing::sphere(double fphas, double dens, double shape, double * wave_x, double * wave_y, double * wave_z)
{
	double formula_001 = dens*PI_x_2*fphas-3*dens;
	double formulua_001_sin, formula_001_cos;
	elan::sinCos(formula_001, &formulua_001_sin, &formula_001_cos); /*optimization*/
	double formula_002 = shape+(1-shape)*sin(PI*(fphas+1)); /*optimization*/
	*wave_y = formulua_001_sin * formula_002;
	*wave_x = -cos(PI*fphas);
	*wave_z = formula_001_cos * formula_002;
}

void JerobeamBoing::rotate(double in_x, double in_y, double in_z, double rot_x, double rot_y, double * out_x, double * out_y, double * out_z)
{
	double rot_x_x_PI_x_2_sin, rot_x_x_PI_x_2_cos;
	elan::sinCos(rot_x*PI_x_2, &rot_x_x_PI_x_2_sin, &rot_x_x_PI_x_2_cos); /*optimization*/
	double help_11 = in_x*rot_x_x_PI_x_2_cos-in_y*rot_x_x_PI_x_2_sin;
	double help_12 = in_x*rot_x_x_PI_x_2_sin+in_y*rot_x_x_PI_x_2_cos;

	double rot_y_x_PI_x_2_sin, rot_y_x_PI_x_2_cos;
	elan::sinCos(rot_y*PI_x_2, &rot_y_x_PI_x_2_sin, &rot_y_x_PI_x_2_cos); /*optimization*/
	double help_21 = help_11*rot_y_x_PI_x_2_cos-in_z*rot_y_x_PI_x_2_sin;
	double help_22 = help_11*rot_y_x_PI_x_2_sin+in_z*rot_y_x_PI_x_2_cos;

	*out_x = help_21;
	*out_y = help_12;
	*out_z = help_22;
}

void JerobeamBoing::boingfunc(double in_x, double in_y, double in_z, double in_boing, double in_strength,
	double * out_x, double * out_y, double * out_z)
{
	//double jump = pow(in_boing, 2); //unused
	double formula_001 = 1-pow(in_boing, 2)*in_strength;
	*out_x = in_x * formula_001;
	*out_y = in_y * formula_001 * (1-pow(1-boing, 4)*in_strength)
		+ (pow(in_boing, .8)*2-1)*in_strength;
	*out_z = in_z;//*(1-boing*strength); // bad for z-dark frequency stability
}

void JerobeamBoing::render(double in_x, double in_y, double in_z, double zdepth,
	double * out_l, double * out_r)
{
	zdepth = pow(zdepth, 2)+1;
	*out_l = in_x*pow(zdepth, -in_z-zdepth*.2);
	*out_r = in_y*pow(zdepth, -in_z-zdepth*.2);
}

void JerobeamBoing::getSampleFrame(double * outL, double * outR)
{
	double z_darkness = pow(z_amount*z_amount*5+1, z_history)+pow(z_amount, 1.5)*.22;

	fphas_ends = triangle(phasor.getSample(), tri);
	double fphas_mids = asin((asin(fphas_ends*2-1)/PI+.5)*2-1)/PI+.5;
	double fphas = bright_dist*fphas_mids+(1-bright_dist)*fphas_ends;

	double wave_x, wave_y, wave_z;
	sphere(fphas, density, shape, &wave_x, &wave_y, &wave_z);
	rotate(wave_x, wave_y, wave_z, rot_x, rot_y, &wave_x, &wave_y, &wave_z);
	rotate(wave_x, wave_y, wave_z, -dir, 0, &wave_x, &wave_y, &wave_z);

	if (vol_prejump)
	{
		wave_x *= volume;
		wave_y *= volume;
		//wave_z *= volume; // bad for z-dark frequency stability 
	}

	boingfunc(wave_x, wave_y, wave_z, boing, boing_strength, &wave_x, &wave_y, &wave_z);

	// corner boing
	wave_y *= 1 - boing_strength*(.5 + volume/2)*(-cos(dir*8*PI)/2+.5)*std::abs(pow(wave_x*.75, 2))*pow(1-boing, 5);

	rotate(wave_x, wave_y, wave_z, dir, 0, &wave_x, &wave_y, &wave_z);
	render(wave_x, wave_y, wave_z, zdepth, outL, outR);

	if (!vol_prejump)
	{
		*outL *= volume;
		*outR *= volume;
	}

	z_history = wave_z;
	//phaseIncrementTAU(&osc_phase, phase_inc * z_darkness);
	phasor.setFrequency(freq * z_darkness);
}
