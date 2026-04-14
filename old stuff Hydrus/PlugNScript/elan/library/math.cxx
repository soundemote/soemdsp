namespace ENVSTATE
{   
    const int OFF = 0;
    const int DELAY = 1;
    const int ATTACK = 2;
    const int DECAY = 3;
    const int SUSTAIN = 4;
    const int RELEASE = 5;
};

namespace ENVSHAPE
{
    const int EXP = 0;
    const int POW = 1;
    const int LOG = 2;
    const int SIGMOID = 3;
    const int LIN = 4;
}

const double PI = 3.14159265359;
const double PI_z_2 = PI / 2.0;
const double _2_z_PI = 2.0 / PI;
const double _4_z_PI = 4.0 / PI;
const double TAU = PI * 2;
const double TAU_INV = 1.0/TAU;
const double LN2_INV = 1.4426950408889634073599246810019; // 1 / log(2)

double squared(double v) { return v*v; }
double log2(double x) {	return LN2_INV * log(x); }
double min(double a, double b) { return a > b ? b : a; }
double max(double a, double b) { return a > b ? a : b; }
double sign(double v) {	return v < 0 ? -1 : v > 0 ? 1 : 0; }

double minValue(array<double>& v)
{
	double smallestValue = v[0];

	for (uint i = 0; i < v.length; ++i)
		smallestValue = min(smallestValue, v[i]);

	return smallestValue;
}

double maxValue(array<double>& v)
{
	double largestValue = v[0];

	for (uint i = 0; i < v.length; ++i)
		largestValue = max(largestValue, v[i]);

	return largestValue;
}

double primePower(int n) 
{ 	
	const array<int> PrimeTable = { 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127, 131, 137, 139, 149, 151, 157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211, 223, 227, 229, 233, 239, 241, 251, 257, 263, 269, 271, 277, 281, 283, 293, 307, 311, 313, 317, 331, 337, 347, 349, 353, 359, 367, 373, 379, 383, 389, 397, 401, 409, 419, 421, 431, 433, 439, 443, 449, 457, 461, 463, 467, 479, 487, 491, 499, 503, 509, 521, 523, 541, 547, 557, 563, 569, 571, 577, 587, 593, 599, 601, 607, 613, 617, 619, 631, 641, 643, 647, 653, 659, 661, 673, 677, 683, 691, 701, 709, 719, 727, 733, 739, 743, 751, 757, 761, 769, 773, 787, 797, 809, 811, 821, 823, 827, 829, 839, 853, 857, 859, 863, 877, 881, 883, 887, 907, 911, 919, 929, 937, 941, 947, 953, 967, 971, 977, 983, 991, 997 };
	
	return pow(PrimeTable[n], .001);
}

void transformRange(array<double>& x, array<double>& y, double targetMin, double targetMax)
{
	if (x.length < 2)
		return;

	double currentMin = minValue(x);
	double currentMax = maxValue(x);

	double a = (targetMin - targetMax) / (currentMin - currentMax);
	double b = (currentMax*targetMin - currentMin*targetMax) / (currentMax - currentMin);
	affineTrafo(x, y, a, b);
}

void affineTrafo(array<double>& x, array<double>& y, double a, double b)
{
	for(uint i = 0; i < x.length; i++)
		y[i] = a * x[i] + b;
}

double clamp(double v, double min, double max)
{
	if (v < min)
		return min;
	if (v > max)
		return max;
	return v;
}

int roundToInt(double v)
{
	return int(round(v));
}

double round(double v)
{
	double floorV = floor(v);
	double compare = v - floorV;

	return compare < 0.5 ? floorV : ceil(v);
}

double roundToMultiple(double n, double multiple)
{
	double numberOfDivisions = round(n / multiple);
	return numberOfDivisions * multiple;
}

double map (double value0To1, double targetRangeMin, double targetRangeMax)
{
    return targetRangeMin + value0To1 * (targetRangeMax - targetRangeMin);
}

double map(double sourceValue, double sourceRangeMin, double sourceRangeMax, double targetRangeMin, double targetRangeMax)
{
    return targetRangeMin + ((targetRangeMax - targetRangeMin) * (sourceValue - sourceRangeMin)) / (sourceRangeMax - sourceRangeMin);
}

double pitchToFrequency(double v)
{
	return 8.1757989156437073336828122976033 * exp(0.057762265046662109118102676788181 * v);
	//return 440.0*( pow(2.0, (pitch-69.0)/12.0) ); // naive, slower but numerically more precise
}
double frequencyToPitch(double freq, double masterTuneA4 = 440)
{
  return 12.0 * log2(freq / masterTuneA4) + 69.0;
}

double bipolarToUnipolar(double v) { return v * .5 + .5; }
double unipolarToBipolar(double v) { return v * 2 - 1; }

double wrapPhase(double phase)
{
	return phase - floor(phase);
}

double wrapPhaseExtreme(double phase)
{
	phase = fraction(phase);

	if (phase < 0)
		phase = 1 + phase;

	return phase;
}

double wrapPhaseTAU(double phase)
{
	phase -= TAU * floor(phase * TAU_INV);
	return phase;
}

// v for input value, c for curvature where -1 is log-like and +1 is exp-like.
double rationalCurve(double v, double c)
{
	double cv = c*v;
	return (cv+v) / (2*cv - c + 1);
	// for a plot, see: https://www.desmos.com/calculator/ql1hh1byy5
}

// Takes an input and a bias (0 top 1 value) and outputs the input via A or B or some mix of both
void forkMixer(double input, double bias, double &outA, double& outB)
{
	double x = bias * PI_z_2;

	outA = sin(x) * input;
	outB = cos(x) * input;
}

bool isEven(int v)
{
	return v % 2 == 0;
}

bool isOdd(int v)
{
	return v % 2 == 1;
}

// beat value of 1 occurs 4 times in one bar, 2 times (2hz) in 1 second given 120 BPM
double beatsToFrequency(double BPM, double beats)
{
	return 1 / beatsToTime(BPM, beats);
}
// bar value of 1 occurs 1/2 times (0.5 hz) in one second given 120 BPM.
double barsToFrequency(double BPM, double bars)
{
	return 1 / beatsToTime(BPM, bars * 4.0);
}
double beatsToTime(double BPM, double beats)
{
	return (60.0 / BPM) * beats;
}