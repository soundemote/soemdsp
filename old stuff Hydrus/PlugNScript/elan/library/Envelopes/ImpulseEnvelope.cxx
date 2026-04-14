/*
Polynomial Impulse: https://iquilezles.org/articles/functions/

NOT YET TESTED
*/

#include "../Filters/Lowpass.cxx"
#include "../Filters/LinearSmoother.cxx"

class ImpulseEnvelope
{
    void reset()
    {
        output = 0;
    }

    void trigger()
    {
        output = 0;
    }
    
    void setSampleRate(double v)
    {
        sampleRate = v;
        update();
    }

    void setLength(double v)
    {
        seconds = v;
        frequency = 1.0/v;
        update();
    }

    void setTightness(double v)
    {
        impulseTightness = v;
    }

    double getSample()
    {        
        output += increment; 

        return quaImpulse(impulseTightness, output);
    }  

    double quaImpulse(double k, double x)
    {
        return 2.0 * sqrt(k) * x/(1.0 + k*x*x);
    }

    void update()
    {
        increment = frequency / sampleRate;
    }

    // user variables
    double seconds;
    double frequency;
    double impulseTightness;

    // internal states
    double sampleRate = 44100;
    double increment = .0001;
    double output = 0;   
};