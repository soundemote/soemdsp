#include "../Oscillators/Ellipse.cxx"
#include "../Filters/Lowpass.cxx"
#include "NoiseGenerator.cxx"
#include "SampleAndHold.cxx"

class VibratoGenerator
{
    VibratoGenerator()
    {  
        setSeed(3,7);
    }

    // set the seed for both freq and amp randomizers
    void setSeed(int a, int b)
    {
        SnHSpeed.setSeed(a);
        SnHRange.setSeed(b);

        NoiseGenerator r;
        r.setSeed(a);

        speedRandomOffset = r.getSampleBipolar();
        rangeRandomOffset = r.getSampleBipolar();
        phaseDesyncValue = r.getSampleUnipolar();
    }

    void reset()
    {
        phase = pm;
    }

    // 0 being zero phase, 1 being random phase
    double phaseSpreadValue;
    double phaseSpreadMult;
    double phaseDesyncValue;
    double phaseDesyncMult;
    double finalPhaseSpreadDesync;

    void setPhaseSpread(double v)
    {
        phaseSpreadMult = v;
        updateSpreadDesyncPhase();
    }
    void setPhaseDesync(double v)
    {
        phaseDesyncMult = v;
        updateSpreadDesyncPhase();
    }
    void updateSpreadDesyncPhase()
    {
        finalPhaseSpreadDesync = phaseSpreadValue * phaseSpreadMult + phaseDesyncValue * phaseDesyncMult;
    }

	void setSampleRate(double v)
    {
        sampleRate = v;
        SnHSpeed.setSampleRate(v);
        SnHRange.setSampleRate(v);
        update();
    }

    void setPhase(double v)
    {
        phase = v;
    }

    void setPM(double v)
    {
    	pm = v;
    }

	void setFrequency(double v)
    {        
        frequency = v;
        SnHSpeed.setFrequency(v);
        SnHRange.setFrequency(v);
        update();
    }

    void setFM(double v)
    {
        fm = v;
        update();
    }

    void setRandomSpeedOffset(double v)
    {
        randomSpeedOffsetMult = v;
    }
    void setRandomRangeOffset(double v)
    {
        randomRangeOffsetMult = v;
    }

	// a is -1 to +1, b is -1 to +1
	void setShapeA(double v)
	{
        shapeA = v;
	}

    void setShapeB(double v)
    {
        shapeB = v * TAU;

        shapeBSin = sin(shapeB);
        shapeBCos = cos(shapeB);
    }

    void setShapeC(double v)
    {
        shapeC = v;
    }

    void setRange(double v)
    {
        range = v;
    }

    void setRandomSpeed(double v)
    {
        randomSpeedMult = v;
    }

    void setRandomRange(double v)
    {
        randomRangeMult = v;
    }

    double getSample()
	{   
		phase += increment;
		phase = phase - floor(phase);

        SnHSpeed.increment();
        setPM(finalPhaseOffsetFromRandom + SnHSpeed.getSmoothedValue() * randomSpeedMult + speedRandomOffset * randomSpeedOffsetMult);
        SnHRange.increment();

		return getValue(phase + pm + finalPhaseSpreadDesync) * range + (SnHRange.getSmoothedValue() * randomRangeMult * range + rangeRandomOffset * randomRangeOffsetMult * range);
	}

    // ph is phase, set shapeA and shapeB before calling
    double lastVal = 0;
	double getValue(double ph)
	{
        //return ellipse.getValue(ph, shapeA, shapeBSin, shapeBCos, shapeC);
        return sin(ph*TAU);
	}

    protected void update()
    {          
        increment = (frequency + fm) / sampleRate;
    }

	Ellipse ellipse;
    SampleAndHold SnHSpeed;
    SampleAndHold SnHRange;

	protected double sampleRate = 44100;
	protected double phase = 0;
	protected double frequency = 0;
    protected double fm = 0;
    protected double pm = 0;
    protected double increment = 0;
    double range = 0;
    protected double finalPhaseOffsetFromRandom = 0;

	protected double shapeA = 0;
    protected double shapeB = 0;
    protected double shapeBSin = 0;
    protected double shapeBCos = 1;
    protected double shapeC = 1;

    protected double randomSpeedMult = 0;
    protected double randomRangeMult = 0;

    double rangeRandomOffset = 0;
    double speedRandomOffset = 0;
    double randomSpeedOffsetMult = 0;
    double randomRangeOffsetMult = 0;
}