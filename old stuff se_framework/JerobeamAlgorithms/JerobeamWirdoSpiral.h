#pragma once

#include "MaxMSPHelper.h"

class JerobeamWirdoSpiral : public gen
{
public:

	void getSampleFrame(double *outL, double *outR);

	void setFrequency(double v) { freq = v; }
	void setSharp(double v) { tri = v; } 
	void setCross(double v) { cross_spread = v; }
	void setDensity(double v) { dens = v*TAU; }
	void setLength(double v) { length = v; }
	void setRotate(double v) { z_rot = v; }
	void setSplash(double v) { splash_depth = v; }
	void setSplashDensity(double v) { splash_dens = v; }
	void setCut(double v) { cut = floor(v+.5); }
	void setScrap(double v) { scrap = clip(v, .0001, 1); }
	void setRingCut(double v) { splash_cut = v; }
	void setSplashSpeed(double v) { splash_speed = v; }
	void setSyncCut(double v) { sync_cut = v; }

protected:

	/* Parameters */
	double freq = 440.0;
	double tri = 0;
	double cross_spread = 0;
	double dens = 5;
	double length = 1;
	double z_rot = 0;
	double splash_depth = 0;
	double splash_dens = 0;
	double cut = 1000;
	double scrap = 1;
	double splash_cut = 10;
	double splash_speed = 0;
	double sync_cut = 0;

	/* Internal States */
	double sampleRate;
	double phase_inc = 0.0;
	double phas = 0.0;
};