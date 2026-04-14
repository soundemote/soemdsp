#include "JerobeamTorus.h"

void JerobeamTorus::setSampleRate(double v)
{
	phasor.setSampleRate(v);
	phasorWander.setSampleRate(v);
	phasorDarkAngle.setSampleRate(v);
	phasorX.setSampleRate(v);
	phasorY.setSampleRate(v);
	phasorZ.setSampleRate(v);
}

void JerobeamTorus::reset()
{
	phasor.reset();
	phasorWander.reset();
	phasorDarkAngle.reset();
	phasorX.reset();
	phasorY.reset();
	phasorZ.reset();
}

void JerobeamTorus::rotate(double in_x, double in_y, double in_z, double rot_x, double rot_y, double rot_z,
	double * out_x, double * out_y, double * out_z)
{
	double sin_rotx, cos_rotx, sin_roty, cos_roty, sin_rotz, cos_rotz;
	elan::sinCos(rot_x, &sin_rotx, &cos_rotx); /*optimization*/
	double help_11 = in_x*cos_rotx-in_y*sin_rotx;
	double help_12 = in_x*sin_rotx+in_y*cos_rotx;
	elan::sinCos(rot_y, &sin_roty, &cos_roty); /*optimization*/
	double help_21 = help_11*cos_roty-in_z*sin_roty;
	double help_22 = help_11*sin_roty+in_z*cos_roty;
	elan::sinCos(rot_z, &sin_rotz, &cos_rotz); /*optimization*/
	double help_31 = help_21*cos_rotz-help_12*sin_rotz;
	double help_32 = help_21*sin_rotz+help_12*cos_rotz;

	*out_x = help_31;
	*out_y = help_32;
	*out_z = help_22;
}

void JerobeamTorus::render(double in_x, double in_y, double in_z, double zaspx, double zaspy, double zdepth,
	double * out_l, double * out_r)
{
	double formula_001 = zdepth*(in_z/2.0+0.5); /*optimization*/
	double zero_p_5_x_zaspx_x_zdepth = 0.5 * zaspx*zdepth; /*optimization*/
	*out_l = in_x - formula_001 * (in_x-zaspx) - zero_p_5_x_zaspx_x_zdepth;
	*out_r = in_y - formula_001 * (in_y+zaspy) + zero_p_5_x_zaspx_x_zdepth;
}

void JerobeamTorus::getSampleFrame(double * outL, double * outR)
{
	dangle = phasorDarkAngle.getSample() + .5;

	rot_x = -TAU * (phasorX.getSample()+1);
	rot_y = TAU * phasorY.getSample() - PI_z_2;
	rot_z = PI_z_2 - TAU * phasorZ.getSample();	

	triphase = waveshape::unitrisaw(phasor.getSample(), sharp);
	phas = fmod(triphase * len - rot_x / PI_x_2, 1.0);

	double blend = sin(rot_y);
	double norm_phas = phas * (1.0 - .5 * std::abs(blend));
	double phas_bipolar = phas*2-1;
	double danked_pos = .5*(clip(blend, 0.0, 1.0)) * (pow(phas_bipolar, dank)+1.0)/2.0;
	double danked_neg = .5*(clip(-blend, 0.0, 1.0)) * (.5*pow((fmod(phas+.5, 1.0))*2.0-1.0, dank) + (sign(phas_bipolar)+1) / 2.0);
	double _phasor = (norm_phas + danked_pos + danked_neg + .25 + rot_x/(PI_x_2) + dangle);

	double sp0sin, sp0cos;
	elan::sinCos(PI_x_2*_phasor, &sp0sin, &sp0cos); /*optimization*/
	double spiral0_x = sp0sin;
	double spiral0_y = sp0cos;
	double spiral0_z = 0.0;

	double sp1sin, sp1cos;
	elan::sinCos(dense*PI_x_2*_phasor, &sp1sin, &sp1cos); /*optimization*/
	double formula_001 = (1.0-bal)/div; /*optimization*/
	double formula_002 = formula_001 * sp1sin; /*optimization*/
	double spiral1_x = formula_002 * sp0sin;
	double spiral1_y = formula_002 * sp0cos;
	double spiral1_z = formula_001 * sp1cos;

	double sp2sin, sp2cos;
	elan::sinCos(sdens * (_phasor + phasorWander.getSample()), &sp2sin, &sp2cos); /*optimization*/
	double bal_z_div_x_div = bal/(div*div); /*optimization*/
	double spiral2_x = bal_z_div_x_div * (sp2cos*sp0cos + sp2sin*sp1sin*sp0sin);
	double spiral2_y = bal_z_div_x_div * (sp2cos*-sp0sin + sp2sin*sp1sin*sp0cos);
	double spiral2_z = bal_z_div_x_div * sp2sin*sp1cos;

	double formula_003 = vol_correct + zdepth_z_2 - vol_correct * zdepth_z_2; /*optimization*/
	double wave_x = (spiral0_x + spiral1_x + spiral2_x) * formula_003;
	double wave_y = (spiral0_y + spiral1_y + spiral2_y) * formula_003;
	double wave_z = (spiral0_z + spiral1_z + spiral2_z) * formula_003;

	rotate(wave_x, wave_y, wave_z, rot_x, rot_y, rot_z, &wave_x, &wave_y, &wave_z);
	render(wave_x, wave_y, wave_z, zaspx, zaspy, zdepth, outL, outR);
}
