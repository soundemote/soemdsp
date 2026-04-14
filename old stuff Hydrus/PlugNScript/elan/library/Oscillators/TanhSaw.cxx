#include "../math.cxx"

class TanhSaw
{
    void reset()
    {
        currentPos = 1.0;
        currentMag = 0.0;
    }

    void setSampleRate(double v)
    {
        sampleRate = v;
        srQuarter = v/4.0;
        update();
    }

    void setFrequency(double v)
    {
        frequency = v;
        update();
    }

    // 0 to 1, 0 being sine, 1 being saw
    void setMorph(double v)
    {
        morph = v;
    }

    void setMagnitudeOffset(double v)
    {
        magOffset = v;
    }

    void setPositionOffset(double v)
    {
        posOffset = v;
    }

    double getSample()
    {        
        currentMag = (currentMag+magOffset) - (currentPos+posOffset) * frequency;
        currentPos = currentPos + currentMag;

        print("currentPos: "+currentMag + "currentMag: "+currentPos);
        
        return tanh(currentPos * (tanScale*morph)) * currentMag * tanScale * .25;
    }

    double update()
    {
        reset();
        double base = frequency;

        
        tanScale = ((srQuarter / (log10(base) * base)) * PI_z_2) * 0.25;
        
        base = base / sampleRate;
        base = base * 4.0;
        return cos((1.0-base) * PI) * 2.0 + 2.0;
    }

    protected double increment = 0;
    double currentMag = 0;
    double magOffset = 1;
    double currentPos = 0;
    double posOffset = 0;
    double srQuarter = 0;
    double tanScale = 0;
    double frequency = 100;
    double sampleRate = 0;
    double morph = 0;
}