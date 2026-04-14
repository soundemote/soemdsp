#pragma once

class PolyBLEP : public Phasor
{
public:
	enum Waveform
	{
		SINE,
		COSINE,
		TRIANGLE,
		SQUARE,
		RECTANGLE,
		SAWTOOTH,
		RAMP,
		MODIFIED_TRIANGLE,
		MODIFIED_SQUARE,
		HALF_WAVE_RECTIFIED_SINE,
		FULL_WAVE_RECTIFIED_SINE,
		TRIANGULAR_PULSE,
		TRAPEZOID_FIXED,
		TRAPEZOID_VARIABLE
	};

	PolyBLEP() = default;
	virtual ~PolyBLEP() = default;

	void setWaveform(Waveform waveform);

	void setPulseWidth(double pw);

	virtual double getSample();

	//void sync(double phase);

protected:
	Waveform waveform = Waveform::SINE;	
	double amplitude = 1.0; // Frequency dependent gain 0 to 1
	double pulseWidth = 0.5; // 0 to 1

	double get();

	double tri(double t);
	double saw(double t);
	double ramp(double t);
	double sqr(double t);
	double half(double t);
	double full(double t);
	double tri2(double t);
	double trip(double t);
	double trap(double t);
	double trap2(double t);
	double sqr2(double t);
	double rect(double t);

	double blep(double t, double dt);
	double blamp(double t, double dt);
};
