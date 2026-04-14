#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>

const double PI             =  3.14159265358979312; //      acos(0)*2
const double PI_z_2         =  1.57079632679489656; //      acos(0)
const double _2_z_PI        =  0.63661977236758138; // 2 / (acos(0)*2)
const double _4_z_PI        =  1.27323954473516276; // 4 / (acos(0)*2)
const double TAU            =  6.28318530717958623; //      acos(0)*4
const double TAU_INV        =  0.31830988618379069; // 1 / (acos(0)*2)
const double LN2_INV        =  1.44269504088896339; // 1 / log(2.0)
const double _1z3           =  0.33333333333333331; // 1 / 3
const double _2z3           =  0.66666666666666663; // 2 / 3
const double sin_pi_x_1_p_5 = -1.00000000000000000; // sin(2 * acos(0) * 1.5)
const double cos_pi_x_1_p_5 = -0.00000000000000018; // cos(2 * acos(0) * 1.5)

double dmin(double a, double b) {
	return a < b ? a : b; // STOP COMPLAINING. BIATCH.
}

double dmax(double a, double b) {
	return a > b ? a : b;
}

double clamp(double v, double min, double max) {
	if (v < min)
		return min;
	if (v > max)
		return max;
	return v;
}

#define WAVETABLE_SIZE 65536
double sinWavetable[WAVETABLE_SIZE];
void makeSinWavetable()
{
	for (int i = 0; i < WAVETABLE_SIZE; ++i)
		sinWavetable[i] = sin((double)i / (double)WAVETABLE_SIZE * TAU); 
} 

// phase is from 0 to 1
double wSin(double phase)
{
	phase = clamp(phase, 0, 1);
	return sinWavetable[(int)(phase * WAVETABLE_SIZE)];
}

// // https://stackoverflow.com/questions/40805206/simple-approximation-of-sine
// // double fSin(double phase)
// // {

// // 	int section = 0;

// // 	if (phase > 0.5)
	

// // 	phase *= 360;


// // 	double _180_m_ph = 180.0 - phase;
// // 	double ret =  4.0 * phase * _180_m_ph / (40500.0 - phase * _180_m_ph);

// // 	return ret;
// // }

// double squared(double v) { return v*v; }
// double log2(double x) {	return LN2_INV * log(x); }
// // double min(double a, double b) { return a > b ? b : a; }
// // double max(double a, double b) { return a > b ? a : b; }

double dbToAmpAccurate(double v)
{
	//return pow(10, v/20);
	return exp(v * 0.11512925464970228420089957273422);
}

double ampTodb(double v)
{
	return 8.6858896380650365530225783783321 * log(v);
}

double minValue(double* v, int size)
{
	double smallestValue = v[0];

	for (int i = 0; i < size; ++i)
		smallestValue = min(smallestValue, v[i]);

	return smallestValue;
}

double maxValue(double* v, int size)
{
	double largestValue = v[0];

	for (int i = 0; i < size; ++i)
		largestValue = max(largestValue, v[i]);

	return largestValue;
}

const int PrimeTable[] = { 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127, 131, 137, 139, 149, 151, 157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211, 223, 227, 229, 233, 239, 241, 251, 257, 263, 269, 271, 277, 281, 283, 293, 307, 311, 313, 317, 331, 337, 347, 349, 353, 359, 367, 373, 379, 383, 389, 397, 401, 409, 419, 421, 431, 433, 439, 443, 449, 457, 461, 463, 467, 479, 487, 491, 499, 503, 509, 521, 523, 541, 547, 557, 563, 569, 571, 577, 587, 593, 599, 601, 607, 613, 617, 619, 631, 641, 643, 647, 653, 659, 661, 673, 677, 683, 691, 701, 709, 719, 727, 733, 739, 743, 751, 757, 761, 769, 773, 787, 797, 809, 811, 821, 823, 827, 829, 839, 853, 857, 859, 863, 877, 881, 883, 887, 907, 911, 919, 929, 937, 941, 947, 953, 967, 971, 977, 983, 991, 997 };
double primePower(int n) 
{ 	
	return pow(PrimeTable[n], .001);
}

void affineTrafo(double* x, double* y, int size, double a, double b)
{
	for(int i = 0; i < size; i++)
		y[i] = a * x[i] + b;
}

void transformRange(double* x, double* y, int size, double targetMin, double targetMax)
{
	if (size < 2)
		return;

	double currentMin = minValue(x, size);
	double currentMax = maxValue(x, size);

	double a = (targetMin - targetMax) / (currentMin - currentMax);
	double b = (currentMax*targetMin - currentMin*targetMax) / (currentMax - currentMin);
	affineTrafo(x, y, size, a, b);
}

double squared(double v)
{
	return v * v;
}


double _round(double v)
{
	double floorV = floor(v);
	double compare = v - floorV;

	return compare < 0.5 ? floorV : ceil(v);
}

int roundToInt(double v)
{
	return (int)_round(v);
}

double roundToMultiple(double n, double multiple)
{
	double numberOfDivisions = _round(n / multiple);
	return numberOfDivisions * multiple;
}

float map0to1(float value0To1, float targetRangeMin, float targetRangeMax)
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

double pitchToFrequency2(double pitch, double _440_div_sampleRate)
{
	return _440_div_sampleRate * pow(2.0, pitch);
}

double frequencyToPitch(double freq)
{
	return 12.0 * log2(freq / 440) + 69.0;
}

double bipolarToUnipolar(double v) { return v * .5 + .5; }
double unipolarToBipolar(double v) { return v * 2 - 1; }

bool isEven(int v)
{
	return v % 2 == 0;
}

bool isOdd(int v)
{
	return v % 2 == 1;
}

double wrapPhase(double phase)
{
	return phase - floor(phase);
}

double wrapPhaseExtreme(double phase)
{	
	double dummy;
	phase = modf(phase, &dummy);

	if (phase < 0)
		phase = 1 + phase;

	return phase;
}

double wrapPhaseTAU(double phase)
{
	phase -= TAU * floor(phase * TAU_INV);
	return phase;
}

double wrapPhaseBidirectional(double phase)
{
	phase = fabs(phase);

	if (isEven(phase)) {
		return wrapPhaseExtreme(phase);
	}
	else {
		return 1 - wrapPhaseExtreme(phase);
	}
}

double wrapPhaseBidirectionalBipolar(double phase)
{
	phase = fabs(phase);

	if (isEven(phase)) {
		return unipolarToBipolar(wrapPhaseExtreme(phase));
	}
	else {
		return unipolarToBipolar(1 - wrapPhaseExtreme(phase));
	}

}

// value is 0 to 1, tension is -1 to +1, negative tension is rise fast, positive is rise slow
double rationalCurve(double v, double c)
{
	double cv = c*v;
	return (cv+v) / (2*cv - c + 1);
	// for a plot, see: https://www.desmos.com/calculator/ql1hh1byy5
}

// value is 0 to 1, tension is -1 to +1, negative tension is rise slow-fast-slow, positive is rise fast-slow-fast
double rationalCurveS(double value, double tension)
{
	double t = tension;
	double v = value;

	if (v < 0.5)
		return (t*v-v) / (4*t*v-t-1);

	v += -0.5;
	t *= -0.5;
	return (t*v-v*0.5) / (4*t*v-t-0.5) + 0.5;
}

// Takes an input and a bias (0 to 1 value) and outputs the input via A or B or some mix of both
void sinMix(double input, double bias, double* outA, double* outB)
{
	double x = bias * PI_z_2;

	*outA = sin(x) * input;
	*outB = cos(x) * input;
}

double beatsToTime(double BPM, double beats)
{
	return (60.0 / BPM) * beats;
}
// beat value of 1 occurs 4 times in one bar, 2 times (2hz) in 1 second given 120 BPM
double beatsToFrequency(double BPM, double beats)
{
	return 1.0 / beatsToTime(BPM, beats);
}
// bar value of 1 occurs 1/2 times (0.5 hz) in one second given 120 BPM.
double barsToFrequency(double BPM, double bars)
{
	return 1.0 / beatsToTime(BPM, bars * 4.0);
}

int howManyHarmonics(double frequency, int userNumberHarmonics, double sampleRate, double maxFrequency, int ABSOLUTEMAX)
{
	int n = clamp(min(sampleRate / frequency, maxFrequency), 0, ABSOLUTEMAX);
	return clamp(n, 0, userNumberHarmonics);
}