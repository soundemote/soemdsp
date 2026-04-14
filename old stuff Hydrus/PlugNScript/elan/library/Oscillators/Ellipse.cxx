#include "../math.cxx"

double ellipsoidSineToSquare(double x, double sineToSquare)
{
    double TAU_x_X = TAU * x;
    double s = sin(TAU_x_X);
    double c = cos(TAU_x_X);

    x = pow(c, 2) + pow(s * sineToSquare, 2);

    return (c / sqrt(x));
}

class Ellipse
{
    Ellipse() {}

    double sampleRate = 44100;
    double frequency = 1;
    double scale = 0;
    double offset = 1;
    double phase = 0;
    double phaseOffset = 0;
    double increment = 0;
    double shape = 0;

	void setFrequency(double v) 
	{ 
		frequency = v; 
		update();
	}

    void setPhaseOffset(double v)
    {
        phaseOffset = v;
    }

	void setSampleRate(double v) 
	{ 
		sampleRate = v; 
		update(); 
	}

	// -1 to +1
	void setOffset(double v) 
    {
        offset = v; 
    }

	// 0 to 1
	void setAngle(double v) 
    {
        B_sin = sin(v*PI);
        B_cos = cos(v*PI);
    }

    void setShape(double v)
    {
        shape = v;
    }

	// 0 to inf
	void setScale(double v)
    {
        scale = v;
    }

    double getSample()
    {
		phase += increment;
		phase = phase - floor(phase);
		return getValue(phase + phaseOffset, offset, B_sin, B_cos, scale);
    }

    double getSampleSinToSquare()
    {
		phase += increment;
		phase = phase - floor(phase);
        return getValueSinToSquare(phase + phaseOffset, shape);
    }

    double getSampleSinToSaw()
    {
        phase += increment;
		phase = phase - floor(phase);
        return getValueSinToSaw(phase + phaseOffset, shape);
    }

    // A = Offset, B = angle, C = scale
    double getValue(double phase, double A, double B_sin, double B_cos, double C)
	{
		double sin_x, cos_x;
        sin_x = sin(phase*TAU);
        cos_x = cos(phase*TAU);

		double sqrt_val = sqrt(pow(A + cos_x, 2) + pow(C*sin_x, 2));
		double output = ((A + cos_x)*B_cos + (C*sin_x)*B_sin) / sqrt_val;

		return output;
	}
    
    // shape is 0 to 1, 0 being sin, 1 being square
    double getValueSinToSquare(double phase, double shape)
    {
        shape = 1 - shape;
        return -getValue(phase, 0, 0, 1, shape);
    }

    // shape is -01 to 1, 0 being sin, 1 being ramp, -1 being saw
    private double sin_pi_x_1_p_5 = sin(PI*1.5);
    private double cos_pi_x_1_p_5 = cos(PI*1.5);
    double getValueSinToSaw(double phase, double shape)
    {
        phase = map(phase, -.75, .25);
        return getValue(phase, -shape, sin_pi_x_1_p_5, cos_pi_x_1_p_5, 1);
    }

    protected void update()
    {
		increment = frequency / sampleRate;
    }

    protected double B_sin = 0;
    protected double B_cos = 1;
}