#pragma once

#include "MaxMSPHelper.h"

class JerobeamRadar : public gen
{
public:
  JerobeamRadar() = default;
  virtual ~JerobeamRadar() = default;
  
	void getSampleFrame(double & outL, double & outR);

	void reset() override { phasor.reset(); rotator.reset(); }

	void setFrequency(double v) { phasor.setFrequency(v); }
	void setPhaseOffset(double v) { phasor.setPhaseOffset(v); }
	void setSampleRate(double v) { phasor.setSampleRate(v); rotator.setSampleRate(v); }

	void setDensity(double v) { density = v; update_dens(); }
	void setSharp(double v) { tri1 = v*.5+.5; }
	void setFade(double v) { pow1 = v; }
	void setRotation(double v) { rotator.setPhaseOffset(v); }

	void setDirection(double v) { tri2 = v; }
	void setShade(double v) { pow2 = clip(v, -80.0, 80.0); }
	void setLap(double v) { lap = jmax(1.e-6,v+1); }

	void setRingCut(bool v) {ringcut = v; update_dens(); }
	void setPow1Up(bool v) { pow1_up = v; }
	void setPow1Down(bool v) { pow1_down = v; }
	void setPow2Bend(bool v) { pow2_bend = v; }
	void setPhaseInv(bool v) { phase_inv = v; }
	void setTunnelInv(bool v) { tunnel_inv = v; }
	void setSpiralReturn(bool v) { spiral_return = v; }

	void setLength(double v) { length = v; }
	void setRatio(double v) { ratio = v; ration = ratio+.1; }
	void setFrontRing(double v) { frontring = v; }
	void setZoom(double v)
	{
		size = v;
		double x = 1-v;
		y_fix_for_zoom = x + (x - pow(x,6));
	}

	void setZDepth(double v) { z_depth = v; }
	void setInner(double v) { z_end = v; }

	void setX(double v) { x = -v; update_x_y(); }
	void setY(double v) { y = v; update_x_y(); }

	double getUnipolarTrianglePhasorValue() { return triphase; }

	/* Core Objects */
	Phasor phasor;
	Phasor rotator;

protected:
	/* Core Functions */
	void render(double in_phas, double * out_x, double * out_y, double * out_z);
	void update_dens()
	{
		dens = (ringcut ? floor(density) : density) + 1.e-6;
		dens = jmin(dens, 1.e+6);
	}
	void update_x_y();

	/* Parameters */
	double frequency = 0;

	double density = 1; // Density 0 to 100
	double tri1 = 0.5; // Sharp 0 to 1
	double pow1 = 0; // Fade .1 to 10
	double rot = 0.5; // Rotation -1 to +1

	double tri2 = 0; // Direction 0 to 1
	double pow2 = 0; // Shade .1 to 10
	double lap = 0; // Lap -1 to +1

	bool ringcut = false; // Ring Cut
	bool pow1_up = false; // Up
	bool pow1_down = false; // Down
	bool pow2_bend = false; // Bend
	bool phase_inv = false; // -Phase or PhaseInv1
	bool tunnel_inv = false; // unlabeled or PhaseInv2
	bool spiral_return = false; // Return

	double length = 1; // Length .0001 to 1
	double ratio = 0; // Ratio 0 to 1
	double frontring = 0; // Front Ring 0 to 1
	double size = 0; // Zoom 0 to 1
	double y_fix_for_zoom = 1;

	double x = 0; // X -1 to +1
	double y = 0; // Y -1 to +1
	double z_depth = 0; // ZDepth 0 to 1
	double z_end = 0; // Inner 0 to 1

	/* Internal States */
	double dens = 1;
	double ration = 0.1;
	double triphase = 0;

	/* `update x y` variables */
	double x_ = 0, y_ = 0, ph = 0, r = 0;
};
