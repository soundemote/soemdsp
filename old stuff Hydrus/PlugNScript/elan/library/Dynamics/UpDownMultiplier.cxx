#include "../Filters/Lowpass.cxx"
#include "../Filters/LinearSmoother.cxx"

class UpDownMultiplier
{
    int direction;
    void setSampleRate(double v)
    {
        smooth.setSampleRate(v);
        lpf.setSampleRate(v);
    }

    void setSmoothingFrequency(double v)
    {
        smooth.setFrequency(v);
        lpf.setFrequency(v);
    }

    void setMultipliers(double up, double down)
    {
        upMult = up;
        downMult = down;
    }

    double getSample(double v)
    {
        direction = 0;
        double ret = v;
        if (v > lastVal)
        {
            direction = 1;
            ret *= lpf.getSample(smooth.getSample(upMult));
        }
        if (v < lastVal)
        {
            direction = -1;
            ret *= lpf.getSample(smooth.getSample(downMult));
        }

        lastVal = v;

        return ret;
    }

    double upMult = 1;
    double downMult = 1;
    double lastVal = 0;

    LinearSmoother smooth;
    Lowpass lpf;
}