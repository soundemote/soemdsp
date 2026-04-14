#pragma once

#include "MaxMSPHelper.h"

class JerobeamBoing : public gen
{
public:

	JerobeamBoing() = default;
	virtual ~JerobeamBoing() = default;

	void getSampleFrame(double * outL, double * outR);

	void reset() { z_history = 0.0; phasor.reset(); }

	void setFrequency(double v) { freq = v; }
	void setSampleRate(double v) { phasor.setSampleRate(v); }

	void setDensity(double v) { density = v; }
	void setSharpness(double v) { tri = v/2.0+.5; }
	void setRotX(double v) { rot_x = (v+90.0)/360.0; }
	void setRotY(double v) { rot_y = v/360.0; }
	void setZDepth(double v) { zdepth = v; }
	void setZAmount(double v) { z_amount = v; }
	void setEnds(double v) { bright_dist = v; }
	void setBoing(double v) { boing = v; }
	void setBoingStrength(double v) { boing_strength = v; };
	void setDir(double v) { dir = v; };
	void setShape(double v) { shape = v; };
	void setVolume(double v) { volume = v; };
	void setVolumePreJump(bool v) { vol_prejump = v; }

	double getUnipolarTrianglePhasorValue() { return fphas_ends; }

	Phasor phasor;

protected:
	/* Core Functions */
	void sphere(double fphas, double dens, double shape, double * wave_x, double * wave_y, double * wave_z);
	void rotate(double in_x, double in_y, double in_z, double rot_x, double rot_y, double * out_x, double * out_y, double * out_z);
	void boingfunc(double in_x, double in_y, double in_z, double in_boing, double in_strength, double * out_x, double * out_y, double * out_z);
	void render(double in_x, double in_y, double in_z, double zdepth, double * out_l, double * out_r);

	/* Parameters */
	double freq{};

	double density{}; //Density 0 to 100
	double tri{}; //Sharpness -1 to 1

	double rot_x{}; //Rotate X -180 to +180
	double rot_y{}; //Rotate Y -180 to +180

	double zdepth{}; //Z-Depth 0 to 1
	double z_amount{}; //Z-Dark 0 to 1
	double bright_dist{}; //Ends -1 to 1

	double boing{}; //Boing 0 to 1
	double boing_strength{}; //Jump 0 to 1
	double dir{}; //Direction 0 to 1

	double shape{}; //Shape 0 to 1

	double volume{}; //Volume 0 to 2
	bool vol_prejump{};

	/* Internal States */
	double fphas_ends{};
	double z_history{};
};
