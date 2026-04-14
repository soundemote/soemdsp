/*
Extended DSF Algorithms by Walter H. Hackett IV
- Engineer's Square to Sine

Antialiased "musical" sawtooth oscillator
with a smooth harmonic falloff.
*/

class MusicalSaw
{
    MusicalSaw() {}

    double sampleRate = 0;
    double frequency = 0;
    double frequencyOffset = 0;
    double phase = 1;
    double output = 0;
    double morph = 1;
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

    void setFrequencyOffset(double v)
    {
        frequencyOffset = v;
        update();
    }

    // 0 to 1, 0 being sine, 1 being saw
    void setMorph(double v)
    {
        morph = v;
        k = 1-(1/(pow(((v)/2.0+0.35),14)*10000+1))+0.0000001;
    }

    double getSample()
    {
        phase += increment * 0.9999; // added 0.9999 to help prevent errors
        phase = phase - floor(phase);

        x = (phase + phaseOffset) * TAU;

        leak = leak * 0.99 + 0.000005;
        value = value * (1.0-leak) + engineerSawDSF() * increment;        
        
        lastValue = value;   
 
        return value;
    }

    protected void update()
    {  
        double f = frequency+frequencyOffset;
        increment = f / sampleRate;
        double maxPartials = f == 0.0 ? 1.0 : (sampleRate * 0.5) / f;
        n = max(floor(maxPartials), 1);
    }

    protected double musicalSawDSF()
    {
        return (sin(x)*k+pow(k,n+1)*(k*sin(x*n)-sin((n+1)*x)))/(1-2*cos(x)*k+k*k);
    }

    protected double engineerSawDSF()
    {
    	return 2.0 * (-pow(k,n+1)*cos((n+1)*x)-k*(-pow(k,n+1)*cos(x*n)+k-cos(x)))/(1-2*cos(x)*k+k*k);
    }

    double phaseOffset = 0;
    double leak = 1.0;
    double increment = 0;
    double lastValue = 0;
    double n = 0; // number of partials
    double k = 0; // morph value
    double x = 0; // phase * TAU
}