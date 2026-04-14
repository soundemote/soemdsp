#include "Phasor.cxx"

namespace WAVEFORM
{
    int SIN = 0;
    int COSIN = 1;
    int TRIANGLE = 2;
    int SQUARE = 3;
    int RECTANGLE = 4;
    int SAWTOOTH = 5;
    int RAMP = 6;
    int MODIFIED_TRIANGLE = 7;
    int MODIFIED_SQUARE = 8;
    int HALF_WAVE_RECTIFIED_SINE = 9;
    int FULL_WAVE_RECTIFIED_SINE = 10;
    int TRIANGULAR_PULSE = 11;
    int TRAPEZOID_FIXED = 12;
    int TRAPEZOID_VARIABLE = 13;
}

class PolyBLEP
{
    void setSampleRate(double v)
    {
        phasor.setSampleRate(v);
        update();
    }

    void setFrequency(double v)
    {
        phasor.setFrequency(v);
        update();   
    }

    void setMorph(double v)
    {
        morph = v;
    }

    void setWaveform(int v)
    {
        waveform = v;
    }

	double getSample()
    {
        double output = get();	

        phasor.inc();

        return output * amplitude;
    }

    void setPhase(double v)
    {
        phasor.setPhase(v);
    }

    void setPhaseOffset(double v)
    {
        phasor.setPhaseOffset(v);
    }

    void reset()
    {
        phasor.reset();
    }

    void update()
    {
        incAmtAbs = abs(phasor.increment);
    }

    void setIncrement(double v)
    {
        incAmtAbs = abs(v);
    }

    Phasor phasor;
    
	int waveform = WAVEFORM::SIN;
	double amplitude = 1.0; // Frequency dependent gain 0 to 1
	double morph = 0.5; // 0 to 1
    double _1z3 = 1.0/3.0;
    double incAmtAbs = 0;

	double get()
    {
        switch (waveform)
        {
        case 0:
            return sin(phasor.getValue() * TAU);
        case 1:
            return cos(phasor.getValue() * TAU);
        case 2:
            return tri(phasor.getValue());
        case 3:
            return sqr(phasor.getValue());
        case 4:
            return rect(phasor.getValue());
        case 5:
            return saw(phasor.getValue());
        case 6:
            return ramp(phasor.getValue());
        case 7:
            return tri2(phasor.getValue());
        case 8:
            return sqr2(phasor.getValue());
        case 9:
            return half(phasor.getValue());
        case 10:
            return full(phasor.getValue());
        case 11:
            return trip(phasor.getValue());
        case 12:
            return trap(phasor.getValue());
        case 13:
            return trap2(phasor.getValue());
        default:
            return sin(phasor.getValue() * TAU);
        }

        return sin(phasor.getValue() * TAU);
    }

    double tri(double t)
    {
        double t1 = t + 0.25;
        t1 = wrapPhase(t1);
        double t2 = t + 0.75;
        t2 = wrapPhase(t2);

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

    double ramp(double t)
    {
        double t1 = t + 0.5;
        t1 = wrapPhase(t1);

        double y = t1 * 2 - 1;

        y -= blep(t1, incAmtAbs);

        return y;
    }

    double saw(double t)
    {
        double t1 = t;
        t1 = wrapPhase(t1);

        double y = 1 - 2 * t1;

        y += blep(t1, incAmtAbs);

        return y;
    }

    double sqr(double t)
    {
        double t1 = t + 0.5;
        t1 = wrapPhase(t1);

        double y = t < 0.5 ? 1 : -1;

        y += blep(t, incAmtAbs) - blep(t1, incAmtAbs);

        return y;
    }

    double half(double t)
    {
        double t1 = t + 0.5;
        t1 = wrapPhase(t1);

        double y = (t < 0.5 ? 2 * sin(TAU * t) - _2_z_PI : -_2_z_PI);

        y += TAU * incAmtAbs * (blamp(t, incAmtAbs) + blamp(t1, incAmtAbs));

        return y;
    }

    double full(double t)
    {
        double t1 = t + 0.25;
        t1 = wrapPhase(t1);

        double y = 2 * sin(PI * t1) - _4_z_PI;

        y += TAU * incAmtAbs * blamp(t1, incAmtAbs);

        return y;
    }

    double tri2(double t) 
    {
        double pw = clamp(morph, 0.0001, 0.9999 );

        double t1 =  t + 0.5 * pw;
            t1 = wrapPhase(t1);
            double t2 = t + 1 - 0.5 * pw;
            t2 = wrapPhase(t2);

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

    double trip(double t)
    {
        double t1 = t + 0.75 + 0.5 * morph;
        t1 = wrapPhase(t1);

        double y;
        if (t1 >= morph)
        {
            y = -morph;
        }
        else
        {
            y = 4 * t1;
            y = (y >= 2 * morph ? 4 - y / morph - morph : y / morph - morph);
        }

        if (morph > 0)
        {
            double t2 =  t1 + 1 - 0.5 * morph;
            t2 = wrapPhase(t2);
            double t3 = t1 + 1 - morph;
            t3 = wrapPhase(t3);
            y += 2 * incAmtAbs / morph * (blamp(t1, incAmtAbs) - 2 * blamp(t2, incAmtAbs) + blamp(t3, incAmtAbs));
        }

        return y;
    }

    double trap(double t) 
    {
        double y = 4 * t;

        if (y >= 3)
            y -= 4;
        else if (y > 1)
            y = 2 - y;
        
        y = max(-1, min(1, 2 * y));

        double t1 = t + 0.125;
            t1 = wrapPhase(t1);
        double t2 = t1 + 0.5;
            t2 = wrapPhase(t2);

        // Triangle #1
        y += 4 * incAmtAbs * (blamp(t1, incAmtAbs) - blamp(t2, incAmtAbs));

        t1 = t + 0.375;
            t1 = wrapPhase(t1);
        t2 = t1 + 0.5;
            t2 = wrapPhase(t2);

        // Triangle #2
        y += 4 * incAmtAbs * (blamp(t1, incAmtAbs) - blamp(t2, incAmtAbs));

            return y;
    }

    double trap2(double t)
    {
        double pw = min(0.9999, morph);
        double scale = 1 / (1 - pw);

        double y = 4 * t;
        if (y >= 3) {
            y -= 4;
        } else if (y > 1) {
            y = 2 - y;
        }
        y = max(-1, min(1, scale * y));

        double t1 = t + 0.25 - 0.25 * pw;
            t1 = wrapPhase(t1);
        double t2 = t1 + 0.5;
            t2 = wrapPhase(t2);

        // Triangle #1
        y += scale * 2 * incAmtAbs * (blamp(t1, incAmtAbs) - blamp(t2, incAmtAbs));

        t1 = t + 0.25 + 0.25 * pw;
            t1 = wrapPhase(t1);
        t2 = t1 + 0.5;
            t2 = wrapPhase(t2);

        // Triangle #2
        return y + scale * 2 * incAmtAbs * (blamp(t1, incAmtAbs) - blamp(t2, incAmtAbs));
    }

    double sqr2(double t) 
    {
        double t1 = t + 0.875 + 0.25 * (morph - 0.5);
            t1 = wrapPhase(t1);

        double t2 = t + 0.375 + 0.25 * (morph - 0.5);
        t2 = wrapPhase(t2);

        // Square #1
        double y = t1 < 0.5 ? 1 : -1;

        y += blep(t1, incAmtAbs) - blep(t2, incAmtAbs);

        t1 += 0.5 * (1 - morph);
        t1 = wrapPhase(t1);

        t2 += 0.5 * (1 - morph);
        t2 = wrapPhase(t2);

        // Square #2
        y += t1 < 0.5 ? 1 : -1;

        y += blep(t1, incAmtAbs) - blep(t2, incAmtAbs);

        return 0.5 * y;
    }

    double rect(double t) 
    {
        double t1 = t + 1 - morph;
            t1 = wrapPhase(t1);

        double y = -2 * morph;

        if (t < morph)
            y += 2;

        y += blep(t, incAmtAbs) - blep(t1, incAmtAbs);

        return y;
    }


    double blep(double t, double dt)
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

    double blamp(double t, double dt)
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
};