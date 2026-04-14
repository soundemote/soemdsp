#pragma once

/*
history: https://docs.cycling74.com/max7/genobject/gen_dsp_history
triangle: https://docs.cycling74.com/max7/vignettes/gen~_operators
triangle: https://docs.cycling74.com/max7/genobject/gen_dsp_triangle
phasor: https://docs.cycling74.com/max7/maxobject/phasor~
phasor: https://docs.cycling74.com/max7/genobject/gen_dsp_phasor
exp: https://docs.cycling74.com/max7/genobject/gen_common_exp
ln: https://docs.cycling74.com/max7/genobject/gen_common_ln

USE THIS SYNTAX:
tri = in2; // Sharp 0 to 1, 0

USE THIS REGEX:
(\s*)(\w+)\s*=\s*(.*);\s*\/\/\s*(\w+)\s*([\d-.+]+)\s*\w*\s*([\d-.+]+)

USE THESE VARIOUS SUBSTITUTIONS: (regex followed by example output)
$1double $2 = $7;
double tri = 0;

void set$4(double v) { $2 = v; } //$3\n
void setSharp(double v) { tri = v; } //in2

par$4,\n
parSharp,

par$4.init(i++, SLIDER, "$4", $5, $6, $7),\n
parSharp.init(i++, SLIDER, "Sharp", 0, 1, 0),

par$4.setCallback([this](double v) { jbWirdoCores.jbWirdo.set$4(v); });\n
parSharp.setCallback([this](double v) { jbWirdoCores.jbWirdo.setSharp(v); });
*/

/* It is a goal to not use this class. Use it only in
* helping convert Gen to C++, then factor it out of
* the code. Call run_gen at the end of your sample
* process block. */

#include <vector>

/*Generates a ramp waveform from 0 to 1*/
class genPhasor
{
public:
	void incrementSample() { phaseIncrementTAU(&phase, phase_inc); }
	double getCurrentValue() { return phase * TAU_INV; }

	void setSampleRate(double v) { sampleRate = v; calculateIncrement(); }
	void setFrequency(double v) { frequency = v; calculateIncrement(); }

	void calculateIncrement() { phase_inc = TAU * frequency / sampleRate; }
	void reset() { phase = 0.0; }

protected:
	double current_value = 0.0;
	double sampleRate = 44100.0;
	double frequency = 1.0;
	double phase = 0.0;
	double phase_inc = TAU*1.0/44100.0;
};

class gen
{
public:
	gen(int numPhasorsToInstantiate = 10) { phasors = std::vector<genPhasor>(numPhasorsToInstantiate); }
  virtual ~gen() = default;

	void setSampleRate(double v) { for (auto & p : phasors) p.setSampleRate(v); }

	virtual void reset() { for (auto & p : phasors) p.reset(); }

protected:
	/* Call this function at the end of your sample process block. */
	void run_gen();

	/* gen to C++ functions */
	double triangle(double x, double y);
	int change(double v);
	double phasor(double freq);
	double clip(double in, double min, double max);
	double sign(double in);

	/* gen to C++ variables */
	double pi = PI;

	size_t curChangeIdx = 0;
	std::vector<double> last_change_value{ 1000, 0 };

	size_t curPhasorIdx = 0;

	std::vector<genPhasor> phasors;
};
