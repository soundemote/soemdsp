/*
Extended DSF Algorithms by Walter H. Hackett IV
- Engineer's Square to Sine

Antialiased oscillator that is able to morph from sine to square
with a smooth harmonic falloff.
*/

#include "../math.cxx"

class SineSquare
{
    SineSquare() {}

    double sampleRate = 0;
    double frequency = 0;
    double phase = 0;
    double phaseOffset = 0;
    double output = 0;
    double morph = 0;
    double value = 0;

    void reset()
    {
        phase = phaseOffset;
        leak = 1;
    }

    void setPhase(double v)
    {
        phase = v;
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

    void setFrequency(double v)
    {
        frequency = v;
        update();
    }

    // 0 to 1, 0 being sine, 1 being square
    void setMorph(double v)
    {
        morph = v;
        k = 1 - (1.0/(pow(((v)/2 + 0.25),14) * 10000 + 1)) + .000000000001;
    }

    double getSample()
    {        
        phase += increment * 0.9999; // added 0.9999 to help prevent errors
        phase = phase - floor(phase);

        x = (phase + phaseOffset) * TAU;

        leak = leak * 0.99 + 0.000005;
        value = value * (1.0-leak) + sinSquareDSF() * increment;        
        
        lastValue = value;

        return value;
    }

    protected void update()
    {
        increment = frequency / sampleRate;
        double maxPartials = frequency == 0.0 ? 1.0 : (sampleRate * 0.5) / frequency;
        n = max(floor(maxPartials * 0.5), 1);
    }

    protected double sinSquareDSF()
    {
        double pow_k_n_p_1 = pow(k, n + 1);
        return 8.0*((pow_k_n_p_1*k*cos(x*(2*n-1))-pow_k_n_p_1*cos(x*(2*n+1))-k*cos(x)*(k-1))/k/(1+k*k-2*k*cos(2*x)));
    }

    protected double leak = 1.0;
    protected double increment = 0;
    protected double lastValue = 0;
    protected double n = 0; // number of partials
    protected double k = 0;
    protected double x = 0;
}