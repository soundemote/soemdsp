#pragma once

#undef min
#undef max

#include "MaxMSPHelper.h"

class JerobeamSpiral
{
public:
	JerobeamSpiral() = default;
	~JerobeamSpiral() = default;

	void setSampleRate(double v) 
	{ 
		phasor.setSampleRate(v); 
		phasor_rotX.setSampleRate(v);
		phasor_rotY.setSampleRate(v);
		phasor_morph.setSampleRate(v);
		phasor_position.setSampleRate(v);
		bpfilter.setSampleRate(v);
	}

	void setFrequency(double v) { frequency = v; }

	void getSampleFrame(double *outL, double *outR);
	double getUnipolarTrianglePhasorValue() { return fphas_ends; }

	void reset() {
		z_history = 0.0; 
		phasor.reset(); 
		phasor_rotX.reset();
		phasor_rotY.reset();
		phasor_morph.reset();
		phasor_position.reset();
		bpfilter.reset();
	}

	// number of spirals
	void setDensity(double v) 
	{
		// must be positive and not 0
    v = jmax(std::fabs(v), 1.e-6);
		dense = v;
		log_dense = log(v);
	}

	// initial position of algorithm
	void setOscPhaseOffset(double v) { phasor.setPhaseOffset(v); }

	// size of spiral or brightness of timbre or spiral width
	void setSize(double v) { div = std::max(v, 0.1); }
	void setSharp(double v) { sharp = v; }
	void setSharpCurve(double v) { bright_dist = v; }
	void setSharpCrvMult(double v) { crvmult = v; }

	// position of spiral
	void setPosition(double v) { phasor_position.setPhaseOffset( v); }
	void setPositionSpeed(double v) { phasor_position.setFrequency(v); }

	void setMorph(double v)  { phasor_morph.setPhaseOffset(v); };
	void setMorphSpeed(double v) { phasor_morph.setFrequency(v); }

	void setRotX(double v) { phasor_rotX.setPhaseOffset(v); };
	void setRotXSpeed(double v) { phasor_rotX.setFrequency(v); }

	void setRotY(double v) { phasor_rotY.setPhaseOffset(v); };
	void setRotYSpeed(double v) { phasor_rotY.setFrequency(v); }

	void setZDepth(double v) { zdepth = v; }
	void setZAmount(double v) { z_amount = v; }

	Phasor phasor;
	Phasor phasor_rotX, phasor_rotY, phasor_morph, phasor_position;
	MonoTwoPoleBandpass bpfilter;

protected:
	/* Core Functions */
	void rotate(double in_x, double in_y, double in_z, double rot_x, double rot_y, double * out_x, double * out_y, double * out_z);
	void spiral(double lophas, double phasor, double dense, double div, double morph, double * x_out, double * y_out, double * z_out);
	void render(double in_x, double in_y, double in_z, double zdepth, double * out_l, double * out_r);

	/* Parameters */
	double frequency = 440;
	double dense = 1;
	double div = 0.5;
	double sharp = 0.5;
	double bright_dist = 0;
	double crvmult = 1;
	double zdepth = 0;
	double z_amount = 0;

	/* Internal States */
	double fphas_ends = 0;
	double log_dense = 0;
	double z_history = 0;
};
