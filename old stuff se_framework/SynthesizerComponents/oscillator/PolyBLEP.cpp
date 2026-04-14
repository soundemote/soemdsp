#include "PolyBLEP.h"

double _1z3 = 1/3.0;

double PolyBLEP::blep(double t, double dt)
{
	if (t < dt)
	{
		return -squared(t / dt - 1);
	}
	else if (t > 1 - dt)
	{
		return squared((t - 1) / dt + 1);
	}
	else
	{
		return 0;
	}
}

double PolyBLEP::blamp(double t, double dt)
{
	if (t < dt)
	{
		t = t / dt - 1;
		return -_1z3 * pow(t, 3);
	}
	else if (t > 1 - dt)
	{
		t = (t - 1) / dt + 1;
		return _1z3 * pow(t, 3);
	}
	else
	{
		return 0;
	}
}

void PolyBLEP::setPulseWidth(double v)
{
	pulseWidth = v;
}

//void PolyBLEP::sync(double phase)
//{
//	t = phase;
//	t -= floor(phase);
//}

void PolyBLEP::setWaveform(Waveform v)
{
	waveform = v;
}

double PolyBLEP::get()
{
	switch (waveform)
	{
	default:
	case SINE:
		return sin(getUnipolarValue() * TAU);
	case TRIANGLE:
		return tri(getUnipolarValue());
	case SQUARE:
		return sqr(getUnipolarValue());
	case SAWTOOTH:
		return saw(getUnipolarValue());
	case RAMP:
		return ramp(getUnipolarValue());
	case RECTANGLE:
		return rect(getUnipolarValue());
	case MODIFIED_TRIANGLE:
		return tri2(getUnipolarValue());
	case MODIFIED_SQUARE:
		return sqr2(getUnipolarValue());
	case HALF_WAVE_RECTIFIED_SINE:
		return half(getUnipolarValue());
	case FULL_WAVE_RECTIFIED_SINE:
		return full(getUnipolarValue());
	case TRIANGULAR_PULSE:
		return trip(getUnipolarValue());
	case TRAPEZOID_FIXED:
		return trap(getUnipolarValue());
	case TRAPEZOID_VARIABLE:
		return trap2(getUnipolarValue());
	}
}

double PolyBLEP::getSample()
{
	double out = get();	

	increment();

	return out * amplitude;
}

double PolyBLEP::tri(double t)
{
	double t1 = t + 0.25;
	wrapPhase(&t1);
	double t2 = t + 0.75;
	wrapPhase(&t2);

	double y = t * 4;

	if (y >= 3)
	{
		y -= 4;
	}
	else if (y > 1)
	{
		y = 2 - y;
	}

	return y + 4 * incAmtAbs * (blamp(t1, incAmtAbs) - blamp(t2, incAmtAbs));
}

double PolyBLEP::ramp(double t)
{
	double t1 = t + 0.5;
	wrapPhase(&t1);

	double y = t1 * 2 - 1;

	y -= blep(t1, incAmtAbs);

	return y;
}

double PolyBLEP::saw(double t)
{
	double t1 = t;
	wrapPhase(&t1);

	double y = 1 - 2 * t1;

	y += blep(t1, incAmtAbs);

	return y;
}

double PolyBLEP::sqr(double t)
{
	double t1 = t + 0.5;
	wrapPhase(&t1);

	double y = t < 0.5 ? 1 : -1;

	y += blep(t, incAmtAbs) - blep(t1, incAmtAbs);

	return y;
}

double PolyBLEP::half(double t)
{
	double t1 = t + 0.5;
	wrapPhase(&t1);

	double y = (t < 0.5 ? 2 * std::sin(TAU * t) - 2 / PI : -2 / PI);

	y += TAU * incAmtAbs * (blamp(t, incAmtAbs) + blamp(t1, incAmtAbs));

	return y;
}

double PolyBLEP::full(double t)
{
	double t1 = t + 0.25;
	wrapPhase(&t1);

	double y = 2 * std::sin(PI * t1) - 4 / PI;

	y += TAU * incAmtAbs * blamp(t1, incAmtAbs);

	return y;
}

double PolyBLEP::tri2(double t) 
{
    double pw = clamp(pulseWidth, 0.0001, 0.9999 );

    double t1 =  t + 0.5 * pw;
		wrapPhase(&t1);
		double t2 = t + 1 - 0.5 * pw;
		wrapPhase(&t2);

    double y = t * 2;

    if (y >= 2 - pw) {
        y = (y - 2) / pw;
    } else if (y >= pw) {
        y = 1 - (y - pw) / (1 - pw);
    } else {
        y /= pw;
    }

    y += incAmtAbs / (pw - pw * pw) * (blamp(t1, incAmtAbs) - blamp(t2, incAmtAbs));

		return y;
}

double PolyBLEP::trip(double t)
{
	double t1 = t + 0.75 + 0.5 * pulseWidth;
	wrapPhase(&t1);

	double y;
	if (t1 >= pulseWidth)
	{
		y = -pulseWidth;
	}
	else
	{
		y = 4 * t1;
		y = (y >= 2 * pulseWidth ? 4 - y / pulseWidth - pulseWidth : y / pulseWidth - pulseWidth);
	}

	if (pulseWidth > 0)
	{
		double t2 =  t1 + 1 - 0.5 * pulseWidth;
		wrapPhase(&t2);
		double t3 = t1 + 1 - pulseWidth;
		wrapPhase(&t3);
		y += 2 * incAmtAbs / pulseWidth * (blamp(t1, incAmtAbs) - 2 * blamp(t2, incAmtAbs) + blamp(t3, incAmtAbs));
	}

	return y;
}

double PolyBLEP::trap(double t) 
{
    double y = 4 * t;

    if (y >= 3)
        y -= 4;
    else if (y > 1)
        y = 2 - y;
    
    y = std::fmax(-1, std::fmin(1, 2 * y));

    double t1 = t + 0.125;
		wrapPhase(&t1);
    double t2 = t1 + 0.5;
		wrapPhase(&t2);

    // Triangle #1
    y += 4 * incAmtAbs * (blamp(t1, incAmtAbs) - blamp(t2, incAmtAbs));

    t1 = t + 0.375;
		wrapPhase(&t1);
    t2 = t1 + 0.5;
		wrapPhase(&t2);

    // Triangle #2
    y += 4 * incAmtAbs * (blamp(t1, incAmtAbs) - blamp(t2, incAmtAbs));

		return y;
}

double PolyBLEP::trap2(double t)
{
    double pulseWidth = jmin(0.9999, this->pulseWidth);
    double scale = 1 / (1 - pulseWidth);

    double y = 4 * t;
    if (y >= 3) {
        y -= 4;
    } else if (y > 1) {
        y = 2 - y;
    }
    y = std::fmax(-1, std::fmin(1, scale * y));

    double t1 = t + 0.25 - 0.25 * pulseWidth;
		wrapPhase(&t1);
    double t2 = t1 + 0.5;
		wrapPhase(&t2);

    // Triangle #1
    y += scale * 2 * incAmtAbs * (blamp(t1, incAmtAbs) - blamp(t2, incAmtAbs));

    t1 = t + 0.25 + 0.25 * pulseWidth;
		wrapPhase(&t1);
    t2 = t1 + 0.5;
		wrapPhase(&t2);

    // Triangle #2
    return y + scale * 2 * incAmtAbs * (blamp(t1, incAmtAbs) - blamp(t2, incAmtAbs));
}

double PolyBLEP::sqr2(double t) 
{
    double t1 = t + 0.875 + 0.25 * (pulseWidth - 0.5);
		wrapPhase(&t1);

    double t2 = t + 0.375 + 0.25 * (pulseWidth - 0.5);
    wrapPhase(&t2);

    // Square #1
    double y = t1 < 0.5 ? 1 : -1;

    y += blep(t1, incAmtAbs) - blep(t2, incAmtAbs);

    t1 += 0.5 * (1 - pulseWidth);
    wrapPhase(&t1);

    t2 += 0.5 * (1 - pulseWidth);
    wrapPhase(&t2);

    // Square #2
    y += t1 < 0.5 ? 1 : -1;

    y += blep(t1, incAmtAbs) - blep(t2, incAmtAbs);

    return 0.5 * y;
}

double PolyBLEP::rect(double t) 
{
    double t1 = t + 1 - pulseWidth;
		wrapPhase(&t1);

    double y = -2 * pulseWidth;

    if (t < pulseWidth)
        y += 2;

    y += blep(t, incAmtAbs) - blep(t1, incAmtAbs);

		return y;
}
