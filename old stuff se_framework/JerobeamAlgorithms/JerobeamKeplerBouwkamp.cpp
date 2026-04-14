#include "JerobeamKeplerBouwkamp.h"

void JerobeamKeplerBouwkamp::getSampleFrame(double * outL, double * outR)
{
	double wave_x = 0;
	double wave_y = 0;

	double fphas = triangle(osc_phase, tri);
	double step_phas = fmod(fphas*n, 1); // step: polygon + circle
	double polygon_number = fphas*n - step_phas+first_polygon; // number of corners

	double polygon_phas = clip((step_phas-circleblend)/(1-circleblend), 0, 1);
	double circle_phas = clip(step_phas/circleblend, 0, 1);
	if (step_phas > circleblend)
		circle_phas = 0;

	double rad_in = cos(PI/polygon_number);
	double rad_in_prev = 1;
	if (polygon_number > first_polygon)
	{
		for (int i = (int)polygon_number; i > first_polygon; --i)
			rad_in_prev *= cos(PI/(i-1));
	}

	double rad_in_next = 1;
	for (int i = (int)polygon_number; i < first_polygon+n-1; ++i)
		rad_in_next *= cos(PI/(i+1));

	int first = 0;
	double f001 = .5/polygon_number; /*opt*/
	if (polygon_number == first_polygon)
		first = 1;
	else if (circle_phas > 1 - f001)
		circle_phas = triangle((circle_phas-(1-f001)) * 1/(f001), .5+.5*circleblend) * f001 + 1-f001;

	if (circle_phas)
	{
		double f003 = rad_in+zoom*(1-rad_in); /*opt*/
		double f002_sin, f002_cos;
		elan::sinCos((circle_phas + (!first)*(1-zoom) * .5/(polygon_number-1)-zoom * first * f001)*TAU, &f002_sin, &f002_cos); /*opt*/
		wave_x = -f002_sin * f003;
		wave_y = f002_cos * f003;
	}
	if (polygon_phas)
	{
		//double line_time = floor(polygon_phas*polygon_number)+1*(polygon_phas!=0); //unused
		polygon_phas = fmod(polygon_phas+1-(1-zoom)*.5/polygon_number, 1);
		double line_phas = fmod(polygon_phas*polygon_number, 1);
		double line_number = floor(polygon_phas*polygon_number)+1*(polygon_phas!=0);

		if (polygon_number != first_polygon+n-1 // not last polygon
			&& line_number == polygon_number // last line
			&& line_phas>.5*zoom && line_phas<.5+.5*zoom)
		{
			line_phas = triangle((line_phas-.5*zoom)*2, 1-circleblend)/2+.5*zoom;
		}

		double line = (line_phas*2-1)*sin(PI/polygon_number);

		double formula_001_sin, formula_001_cos;
		elan::sinCos(line_number/polygon_number*TAU, &formula_001_sin, &formula_001_cos); /*opt*/
		wave_x = line * formula_001_cos + rad_in * formula_001_sin;
		wave_y = rad_in * formula_001_cos - line * formula_001_sin;

	}

	wave_x *= zoom*rad_in_prev+(1-zoom)*rad_in_next;
	wave_y *= zoom*rad_in_prev+(1-zoom)*rad_in_next;

	double rot_sin, rot_cos;
	elan::sinCos(rotation * (polygon_number-first_polygon)*TAU, &rot_sin, &rot_cos); /*optimization*/
	*outL = wave_x*rot_cos + wave_y*rot_sin;
	*outR = wave_y*rot_cos - wave_x*rot_sin;

	phaseIncrementTAU(&osc_phase, phase_inc);
}
