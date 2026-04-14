#include "../OnePoleFilter.cxx"
#include "../../Oscillators/SineSaw.cxx"
#include "../../Oscillators/TriSaw.cxx"
#include "../../Randoms/NoiseGenerator.cxx"
#include "../../Dynamics/SoftClipper.cxx"

class SuperloveFilterPrototype
{
	SuperloveFilterPrototype()
	{
		filter1.setMode(1);
		filter2.setMode(1);
		filter3.setMode(1);

		dcfilter.setMode(2);

		dcfilter.setFrequency(5);

		oscillator.setFrequency(0);
	}

    void setSampleRate(double v)
    {
        sampleRate = v;

        filter1.setSampleRate(v);
        filter2.setSampleRate(v);
        filter3.setSampleRate(v);

        dcfilter.setSampleRate(v);

		oscillator.setSampleRate(v);
    }

    void setPitch(double v)
    {
        pitch = v;
		double freq = clamp(pitchToFrequency(pitch), 20, 20000);
		noiseAmpVsFreq = map(freq, 20, 20000, 0, 1);
		noiseAmpVsFreq = rationalCurve(noiseAmpVsFreq, .9);

        double f1 = pitchToFrequency(v+p1);
        double f2 = pitchToFrequency(v+p2);
        double f3 = pitchToFrequency(v+p3);

  		filter1.setFrequency(clamp(f1, 0, 20000));
  		filter2.setFrequency(clamp(f2, 0, 20000));
  		filter3.setFrequency(clamp(f3, 0, 20000));

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

	// 0 to 1
    void setShape(double v)
    {
    	shape = v;
		oscillator.setMorph(v);
    }

	void setPhaseOffset(double v)
	{
		phaseOffset = v;
		oscillator.setPhaseOffset(v);
	}

	// 0 to 2
	void setOutputTap(double v)
	{
		outTap = roundToInt(clamp(v, 0, 2));
	}

    double getSample(double v)
    {		
		feedback = clamp(feedback * feedbackGain , -railValue, railValue);
		//feedback = clip.getSample(feedback * feedbackGain);

    	feedback += v * inputGain;

		double noiseMod = noiseGenerator.getSampleBipolar() * noiseAmp * noiseAmpVsFreq;

		double p = feedback + phaseOffset + noiseMod;

		feedback = /*(*/triSaw.getValue(p, shape)/* + triSaw.getValue(wrapPhaseExtreme(p*1.48), shape) + triSaw.getValue(wrapPhaseExtreme(p*13.02), shape))/3.0*/;

    	feedback = filter1.getSample(feedback);

		out1 = feedback;
    	
    	feedback = filter2.getSample(feedback);

		out2 = feedback;

    	feedback = filter3.getSample(feedback);

		out3 = feedback;

		switch(outTap)
		{
			case 0: return dcfilter.getSample(out1 * outputGain);
			case 1: return dcfilter.getSample(out2 * outputGain);
			case 2: return dcfilter.getSample(out3 * outputGain);
			default: return dcfilter.getSample(out1 * outputGain);
		}

		return 0;
	}

	void setFilterPitch1(double v)
	{
		p1 = v;
		double f = pitchToFrequency(pitch + v);
		filter1.setFrequency(clamp(f, 0, 20000));
	}
	void setFilterPitch2(double v)
	{
		p2 = v;
		double f = pitchToFrequency(pitch + v);
		filter2.setFrequency(clamp(f, 0, 20000));
	}
	void setFilterPitch3(double v)
	{
		p3 = v;
		double f = pitchToFrequency(pitch + v);
		filter3.setFrequency(clamp(f, 0, 20000));
	}

	void setNoiseAmplitude(double v)
	{
		noiseAmp = v;
	}

	void setRails(double v)
	{
		railValue = max(v, .0001);
		clip.setWidth(railValue);
	}

	void reset()
	{
		filter1.reset();
		filter2.reset();
		filter3.reset();
		dcfilter.reset();
        
        out1 = 0;
		out2 = 0;
		out3 = 0;
        feedback = 0;
	}

    double pitch = 0;
    double sampleRate = 44100;
	double inputGain = .01;
	double outputGain = 1;
   	double feedbackGain = .1;
	double shape = 0;
	double phaseOffset = 0;
	double railValue = 1;
	double noiseAmp = 0;
	double noiseAmpVsFreq = 0;

	OnePoleFilter filter1;
	OnePoleFilter filter2;
	OnePoleFilter filter3;

	OnePoleFilter dcfilter;

	NoiseGenerator noiseGenerator;

	SineSaw oscillator;

	SoftClipper clip;

	double out1 = 0;
	double out2 = 0;
	double out3 = 0;
	int outTap = 1;
	double feedback = 0;

    double p1 = 22.08;
    double p2 = 7.08;
    double p3 = 36.0864;

	TriSaw triSaw;
	Ellipse ellipse;
};

class SuperloveFilter
{
	SuperloveFilter()
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
		filter.setInputGain(0.06);
		filter.setFeedbackGain(.9);
		filter.setRails(999);
		filter.setFilterPitch1(32);
		filter.setFilterPitch2(44);
		filter.setFilterPitch3(56);
		filter.setPhaseOffset(-.392);
		filter.setOutputTap(1);
		filter.setOutputGain(1);
		break;

		case 1:
		// Crunchy
		filter.setOutputTap(0);
		filter.setInputGain(.28);
		filter.setFeedbackGain(.7);
		filter.setRails(999);
		filter.setFilterPitch1(38.404);
		filter.setFilterPitch2(18.118);
		filter.setFilterPitch3(104.644);
		filter.setPhaseOffset(-.085);
		filter.setOutputGain(1);
		break;

		case 2:
		// Howling
		filter.setOutputTap(0);
		filter.setInputGain(.1);
		filter.setFeedbackGain(.5);
		filter.setRails(999);
		filter.setFilterPitch1(47.305);
		filter.setFilterPitch2(13.771);
		filter.setFilterPitch3(6.733);
		filter.setPhaseOffset(0);
		filter.setOutputGain(1);
		break;

		case 3:
		// Woody
		filter.setOutputTap(0);
		filter.setInputGain(0.5);
		filter.setFeedbackGain(.45);
		filter.setRails(999);
		filter.setFilterPitch1(75.457);
		filter.setFilterPitch2(91.810);
		filter.setFilterPitch3(73.180);	
		filter.setPhaseOffset(0.2);
		filter.setOutputGain(1);
		break;

		case 4:
		// Brittle
		filter.setOutputTap(2);
		filter.setInputGain(.8);
		filter.setFeedbackGain(1.15);
		filter.setRails(999);
		filter.setFilterPitch1(32.401);
		filter.setFilterPitch2(35.299);
		filter.setFilterPitch3(42.130);
		filter.setPhaseOffset(0);
		filter.setOutputGain(1);
		break;

		case 5:
		// Squelchy
		filter.setOutputTap(2);
		filter.setInputGain(.8);
		filter.setFeedbackGain(2.2);
		filter.setRails(999);
		filter.setFilterPitch1(28.675);
		filter.setFilterPitch2(39.025);
		filter.setFilterPitch3(39.232);		
		filter.setPhaseOffset(0);
		filter.setOutputGain(1);
		break;

		case 6:
		// Quirky
		filter.setOutputTap(0);
		filter.setInputGain(0.6);
		filter.setFeedbackGain(13);
		filter.setRails(999);
		filter.setFilterPitch1(13.308);
		filter.setFilterPitch2(76.029);
		filter.setFilterPitch3(57.192);	
		filter.setPhaseOffset(0.2);
		filter.setOutputGain(1);
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

		filter.setShape(mapped);
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

	SuperloveFilterPrototype filter;

	double cutoff = 0;
	double resonance = 0;
	double sampleRate = 0;
	int preset = 0;
}