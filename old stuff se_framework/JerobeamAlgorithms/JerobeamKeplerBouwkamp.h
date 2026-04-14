#pragma once

#include "MaxMSPHelper.h"

class JerobeamKeplerBouwkamp : public gen
{
public:
  JerobeamKeplerBouwkamp() = default;
  ~JerobeamKeplerBouwkamp() = default;
  
	void setFrequency(double v) { freq = v; recalculatePhaseInc(); }
	void getSampleFrame(double *outL, double *outR);
	void setSampleRate(double v) { sampleRate = v; recalculatePhaseInc(); }
	void reset() { osc_phase = 0.0; }

	void setStart(double v) { first_polygon = v; }
	void setLength(double v) { n = v; }
	void setCircles(double v) { circleblend = v; }
	void setZoom(double v) { zoom = v; }
	void setRotation(double v) { rotation = v; }
	void setTri(double v) { tri = v; }

protected:
	void recalculatePhaseInc() { phase_inc = TAU * freq / sampleRate; }

	/* Parameters */
	double freq = 440.0;

	double first_polygon = 3; //Start 3 to 20 integer
	double n = 1; //Length 1 to 20 integer
	double circleblend = 0; //Circles 0 to 1
	double zoom = 0; //Zoom 0 to 1
	double rotation = 0; //Rotation -1 to +1
	double tri = 0; //Tri 0 to 1;

									/* Internal States */
	double sampleRate = 44100.0;
	double phase_inc = 0.0;
	double osc_phase = 0.0;
};