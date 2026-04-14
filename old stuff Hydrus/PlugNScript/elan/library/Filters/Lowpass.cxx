/*
Simple one pole lowpass filter
*/
#include "../math.cxx";

class Lowpass
{
    double sampleRate = 0;
    double frequency = 0;
    double output = 0;

    void reset()
    {
        output = 0;
    }

    void setSampleRate(double v)
    {
        sampleRate = v;
        update();
    }

    void setFrequency(double v)
    {
        frequency = v;
        update();
    }

    double getSample(double v)
    {
        output += a0 * (v - output);
        return output;
    }

    protected void update()
    {
        double x = exp(-TAU*frequency/sampleRate);
        b1 = -x;
        a0 = 1.0+b1;
    }

    protected double a0 = 0;
    protected double b1 = 0;
};