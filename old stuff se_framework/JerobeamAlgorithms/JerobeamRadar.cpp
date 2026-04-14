#include "JerobeamRadar.h"

void JerobeamRadar::update_x_y()
{
	// x_, y_: x and y projected on a circular shape
	x_ = sin(x*(PI_z_4+(1-std::abs(y))*PI_z_4));
	y_ = y*cos(x*PI_z_4);
	// r, ph: polar coordinates (radius, angle)
	r = (sign(y_)+(y_==0))*sqrt(x_*x_+y_*y_);

	if (y_ != 0)
		ph = atan(x_/y_);
	else
		ph = PI_z_2*sign(x_);
}

void JerobeamRadar::render(double in_phas, double * out1, double * out2, double * out3)
{
	double phas = triphase = triangle(in_phas, tri1);
	if (phase_inv)
		phas = 1-triangle(in_phas, tri1);

	//phas = (pow(phas, 20)*pow1/10+phas*(1-pow1/10))*length; // different fade method
	if ((pow1_up && in_phas < tri1)	|| (pow1_down && in_phas >= tri1))
		phas = pow(phas, pow1);

	phas = phas*(dens+frontring/(tunnel_inv+(!tunnel_inv)*length))/dens; // frontring

	double sphas = phas;
	if (in_phas > tri1 && spiral_return)
		sphas = 2-phas;

	double sin_phas = clip(pow(triangle(sphas*length*dens, tri2), pow2), -1.e+100, +1.e+100); // clipping needed to avoid inf
	double f002_sin, f002_cos;
	rot = rotator.getSample();
	elan::sinCos((sin_phas-(tunnel_inv*frontring) - rot/lap - (!tunnel_inv*length*dens))*TAU*lap, &f002_sin, &f002_cos); /*optimization*/
	double lilsin = f002_cos*ration;
	double lilcos = f002_sin*ration;

	phas *= length;
	phas = (1-pow2_bend)*(floor(phas*dens)/dens + sin_phas/dens) + pow2_bend*phas;

	if (ringcut)
		phas = (floor(phas*dens+tunnel_inv*(1-frontring))+rot-tunnel_inv*(1-frontring))/dens; // ringcut

	if (!tunnel_inv)
		phas = 1-phas-(1-length)+frontring/dens;

	phas = clip(phas-frontring/dens, 0, 1);
	//phas = clip(phas*(dens+frontring)/(dens+1)-frontring/dens, 0, 1); // frontring

	double ph_sin_neg, ph_cos_neg;
	elan::sinCos(-ph, &ph_sin_neg, &ph_cos_neg); /*optimization*/
	double lilsin1 = lilsin*ph_sin_neg + lilcos*ph_cos_neg;
	double lilcos1 = lilcos*ph_sin_neg - lilsin*ph_cos_neg;

	double f003_sin, f003_cos;
	elan::sinCos(phas*std::abs(r)*TAU, &f003_sin, &f003_cos); /*optimization*/
	double bigsin = +f003_cos;
	double bigcos = -f003_sin;

	double lil_x = lilsin1*bigsin;
	double lil_y = lilcos1;
	double lil_z = lilsin1*bigcos*sign(r);

	//---------------- the real deal -----------------

	double big_x = 0;
	double big_y = 0;
	double big_z = -TAU*phas;
	if (r != 0)
	{
		big_z = bigcos/std::abs(r);
		big_x = (bigsin-1)/r;
	}

	double wave_x1 = big_x + lil_x;
	double wave_y1 = big_y + lil_y;
	double wave_z2 = big_z + lil_z;

	double ph_sin, ph_cos;
	elan::sinCos(ph, &ph_sin, &ph_cos); /*optimization*/
	double wave_x = wave_x1*ph_sin + wave_y1*ph_cos;
	double wave_y2 = wave_y1*ph_sin - wave_x1*ph_cos;

	double syz = 2*(size+.33)*(std::abs(x)*(1-y)+.5);
	wave_x = size * wave_x + (1-size)*(wave_x + x*(1-ratio) + x*ratio)*syz;
	wave_y2 = size * wave_y2 + (1-size)*(wave_y2-y)*syz;
	wave_z2 = size * wave_z2 + (1-size)*wave_z2*syz;

	double siz_sin, siz_cos;
	elan::sinCos((1-size)*PI_z_2, &siz_sin, &siz_cos); /*optimization*/
	double wave_y = wave_y2*siz_cos + wave_z2*siz_sin;
	double wave_z = wave_z2*siz_cos - wave_y2*siz_sin;

	*out1 = wave_x;
	*out2 = wave_y;
	*out3 = wave_z;
}

void JerobeamRadar::getSampleFrame(double & outL, double & outR)
{
	double wave_x, wave_y, wave_z;
	render(phasor.getUnipolarValue(), &wave_x, &wave_y, &wave_z);

	double depth = (1-z_depth) * (1 - std::abs(wave_z)*TAU_INV) + z_depth*(pow(z_depth*9 + 1, wave_z));
	double f001 = (depth*(1-z_end)+z_end)/((1-size)+size*ration); /*optimization*/
	outL = wave_x * f001;
	outR = wave_y * f001 + y_fix_for_zoom;

	phasor.increment();
}
