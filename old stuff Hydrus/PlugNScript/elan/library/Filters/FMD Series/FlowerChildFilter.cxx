#include "../OnePoleFilter.cxx"
#include "../../Randoms/NoiseGenerator.cxx"
#include "../../Oscillators/Phasor.cxx"
#include "../../Oscillators/Ellipse.cxx"

class FlowerChildFilterPrototype
{
	FlowerChildFilterPrototype()
	{
		filter1.setMode(1);
		filter2.setMode(1);
	}

    void setSampleRate(double v)
    {
        sampleRate = v;

        filter1.setSampleRate(v);
        filter2.setSampleRate(v);
    }

	// 0 to 1
    void setPitch(double v)
    {
        masterPitch = v;
		masterFrequency = pitchToFrequency(masterPitch);
		updateFilterFrequencies();
    }

	void setPMAmount(double v)
	{
		pmMod = v;
	}

	void setFMAmount(double v)
	{
		fmMod = v;
	}

	void setResonatorGain(double v)
	{
		resonatorGain = v;
	}

	void setSquareness(double v)
    {
        squareness = v;
    }

	void setRailValue(double v)
	{
		railValue = v;
	}

	void setInputGain(double v)
	{
		inputGain = v;
	}

	void setOutputGain(double v)
	{
		outputGain = v;
	}

    void setFeedbackGain(double v)
    {
    	feedbackGain = v;
    }

	void setPhaseOffset(double v)
	{
		phaseOffset = v;
	}

	// 0 to 1
	void setOutputTap(double v)
	{
		outTap = roundToInt(clamp(v, 0, 1));
	}

	void setFilterPitch1(double v)
	{
		p1 = v;
		updateFilterFrequencies();
	}

	void setFilterPitch2(double v)
	{
		p2 = v;
		updateFilterFrequencies();
	}

	void setNoiseAmplitude(double v)
	{
		noiseAmp = v;
	}

	void reset()
	{
		filter1.reset();
		filter2.reset();
        
        out1 = 0;
		out2 = 0;
        feedback = 0;
	}

	double getSample(double v)
    {       
		v = feedback * feedbackGain + clamp(-1 * v * inputGain, -railValue, railValue);

		phasor.setFrequency(masterFrequency * v * fmMod);
		phasor.inc();

		double phasorOut = unipolarToBipolar(phasor.phase) + feedback * pmMod;

		double ellipseOut = ellipse.getValueSinToSquare(phasorOut + phaseOffset, squareness) * resonatorGain;

        feedback = filter1.getSample(ellipseOut);

        out1 = feedback;

        feedback = filter2.getSample(feedback);

        out2 = feedback;

		switch(outTap)
		{
			case 0: return out1 * outputGain;
			case 1: return out2 * outputGain;
			default: return out2 * outputGain;
		}

		return out1 * outputGain;
	}

	protected void updateFilterFrequencies()
	{
		filter1.setFrequency(pitchToFrequency(masterPitch + p1));
		filter2.setFrequency(pitchToFrequency(masterPitch + p2));
	}

	Phasor phasor;
	Ellipse ellipse;
	OnePoleFilter filter1;
	OnePoleFilter filter2;
	NoiseGenerator noiseGenerator;

    double sampleRate = 44100;
   	double feedbackGain = .1;
	double resonatorGain = 1;
	double shape = 0;
	double railValue = 1;
	double noiseAmp = 0;
	double phaseOffset = 0;	
    double squareness = 0;

	int outTap = 1;
	double out1 = 0;
	double out2 = 0;	
	double inputGain = .01;
	double outputGain = 1;
	double feedback = 0;
    double fmMod = 0;
    double pmMod = 0;

	double masterPitch = 0;
	double masterFrequency = 440;
	double filter1Pitch = 0;
	double filter2Pitch = 0;
    double p1 = 0;
    double p2 = 0;
	double noiseAmpVsFreq = 0;
};

class FlowerChildFilter
{
	FlowerChildFilter()
	{
	}

	// 0 to 9
	void setPreset(double v)
	{
		int preset = roundToInt(clamp(v, 0, 9));

		switch(preset)
		{
		case 0:
		// Hot
		break;

		default:
		}
	}

	void setSampleRate(double v)
	{
		sampleRate = v;
		filter.setSampleRate(v);
	}

	// 0 to 1
	void setCutoff(double v)
	{
		cutoff = v;
		double mapped = map(cutoff, -60, 140);

		filter.setPitch(mapped);
	}

	// 0 to 1
	void setResonance(double v)
	{
		resonance = v;
		double curved = rationalCurve(resonance, .95);
		double mapped = map(curved, .3, 1);

		filter.setSquareness(mapped);
	}

	void setNoiseAmplitude(double v)
	{
		filter.setNoiseAmplitude(v);
	}

	double getSample(double v)
	{
		return filter.getSample(v);
	}

	void reset()
	{
		filter.reset();
	}

	FlowerChildFilterPrototype filter;

	double cutoff = 0;
	double resonance = 0;
	double sampleRate = 0;
	int preset = 0;
}