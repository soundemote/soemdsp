//double const PI = 3.1415926535897932384626433832795;
double const TAU = PI * 2.0;
double const TAU_INV = 1.0 / (PI * 2.0);
double const PI_INV_x_2 = (1.0 / PI)*2.0;
double const PI_z_2 = PI / 2.0;
double const PI_INV_z_2 = PI_INV / 2.0;
double const PI_z_4 = PI / 4.0;
double const PI_z_6 = PI / 6.0;
double const PI_x_2 = PI * 2.0;
double const PI_x_1p5 = PI * 1.5;
double const PI_x_4 = PI * 4.0;
double const INV_180 = 1.0 / 180.0;
double const TWOTHIRDS = 2.0 / 3.0;
double const ONETHIRD = 1.0 / 3.0;
double const INV_127 = 1.0 / 127.0;
double const INV_8192 = 1.0 / 8192.0;
double const ONETWELVTH = 1.0 / 12.0;

double bipolarToUnipolar(double v) { return v * .5 + .5; }
double unipolarToBipolar(double v) { return v * 2 - 1; }

void wrapPhase(double * phase) 
{
	*phase -= floor(*phase);
}

double min(double a, double b)
{
	return a < b ? a : b;
}

double max(double a, double b)
{
	return a < b ? b : a;
}

double wrapPhase(double phase)
{
	return phase - floor(phase);
}

void wrapPhaseTAU(double * phase)
{
	//method 1:
	*phase -= TAU * floor(*phase * TAU_INV);

	////method 2 for negative phase
	//while (*phase > TAU)
	//	*phase -= TAU;
	//while (*phase < 0.0)
	//	*phase += TAU;
}

double wrapPhaseTAU(double phase)
{
	return phase - TAU * floor(phase * TAU_INV);

	//while (phase > TAU)
	//	phase -= TAU;
	//while (phase < 0.0)
	//	phase += TAU;
	//return phase;
}

double degreesToPhase(double degrees)
{
	return PI * degrees * INV_180;
}

void rotate2D(double sin, double cos, double * X, double * Y)
{
	double x = *X, y = *Y;
	*X = x * cos - y * sin;
	*Y = x * sin + y * cos;
}

template <typename Type>
constexpr Type jmap (Type value0To1, Type targetRangeMin, Type targetRangeMax)
{
    return targetRangeMin + value0To1 * (targetRangeMax - targetRangeMin);
}

template <typename Type>
constexpr Type jmap(Type sourceValue, Type sourceRangeMin, Type sourceRangeMax, Type targetRangeMin, Type targetRangeMax)
{
	return targetRangeMin + ((targetRangeMax - targetRangeMin) * (sourceValue - sourceRangeMin)) / (sourceRangeMax - sourceRangeMin);
}

namespace elan
{
	void sinCos(double v, double * sinOut, double * cosOut)
	{
		RAPT::rsSinCos(v, sinOut, cosOut);
	}
	double pitchToFreq(double v)
	{
		return RAPT::rsPitchToFreq(v);
	}
} // namespace elan

double curve(double value, double tension)
{
	double t = tension;
	double v = value;
	return (t*v-v)/(2*t*v-t-1);
}

double pitchToFreq(double p)
{
	return 440.0 * pow(2.0, (p-69.0) * ONETWELVTH);
}

#include "../SynthesizerComponents/oscillator/Phasor.cpp"
#include "../SynthesizerComponents/oscillator/waveshapes.cpp"
#include "../SynthesizerComponents/utility/ParamSmoother.cpp"
#include "../SynthesizerComponents/utility/PreciseTimer.cpp"
#include "../SynthesizerComponents/oscillator/NoiseGenerator.cpp"

