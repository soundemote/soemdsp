#include "JerobeamWirdoSpiral.h"

void JerobeamWirdoSpiral::getSampleFrame(double * outL, double * outR)
{
	phas = phasor(freq);
	if (cut<1000)
		phas = int(phas*cut)/cut; // turns ramp into stair -> sample&hold

	double cross_rot = (phas>tri)*cross_spread*TAU-cross_spread*pi;
	double cross_phas = triangle(phas, tri);
	if (sync_cut<1)
		cross_phas = int(cross_phas*clip(std::abs(dens)*sync_cut, 1, 1000))/clip(std::abs(dens)*sync_cut, 1, 1000);
	double crossbow = cross_phas*length-clip(length-1, 0, 1);

	double cross_x = crossbow*cos(cross_rot);
	double cross_y = crossbow*sin(cross_rot);

	double spirot = crossbow*dens;
	double spirot_x = cross_x*cos(spirot) + cross_y*sin(spirot);
	double spirot_y = cross_y*cos(spirot) - cross_x*sin(spirot);
	double splash = sin((triangle(phas*splash_dens+phasor(splash_speed), 1))*TAU*scrap);

	if (scrap<.25)
		splash /= sin(scrap*TAU);
	if (scrap<.5)
		splash = splash*2-1;
	else if (scrap<.75) // not working yet
		splash = splash*(2+sin(scrap*TAU))-(sin(scrap*TAU)+1)*(1+sin(scrap*TAU));

	if (splash_cut<10)
		splash = int(splash*splash_cut)/splash_cut;

	*outL = spirot_x;
	*outR = spirot_y*cos(z_rot*pi/2)+splash*splash_depth;//*sin(z_rot*pi/2);

	run_gen();
}