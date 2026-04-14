#include "../../Oscillators/SineSaw.cxx"
#include "../../Oscillators/TriSaw.cxx"
#include "../../Oscillators/PolyBLEP.cxx"
#include "../../Randoms/NoiseGenerator.cxx"
#include "../../Dynamics/SoftClipper.cxx"
#include "../../Filters/Lowpass.cxx"
#include "../../Filters/Highpass.cxx"

class SuperloveExtremeFilterPrototype
{
    int MODE = 0;
    int MODE_bandpass = 0;
    int MODE_highpass = 1;

	SuperloveExtremeFilterPrototype()
	{
		oscillator.setFrequency(0);
		oscillator.setWaveform(WAVEFORM::MODIFIED_TRIANGLE);
        hpf.setFrequency(10);
        lpf.setFrequency(20000);
	}

    void setSampleRate(double v)
    {
        sampleRate = v;

        lpf.setSampleRate(v);
        hpf.setSampleRate(v);

		oscillator.setSampleRate(v);

		update();
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

	void setPitch(double v)
	{
		masterPitch = v;
		update();
	}

	void setHighpassFreqOffset(double v)
	{
		hpfFreqOffset = v;
		update();
	}

	void setLowpassFreqOffset(double v)
	{
		lpfFreqOffset = v;
		update();
	}

	void setBandwidthFreqOffset(double v)
	{
        halfBwFreqOffset = v * .5;
		update();
	}

	void setNoiseAmplitude(double v)
	{
		noiseAmp = v;
	}

	void setRails(double v)
	{
		//railValue = max(v, .0001);
		clip.setWidth(v);
	}

	void reset()
	{
		lpf.reset();
		hpf.reset();
        oscillator.reset();
	}

    double getSample(double input)
    {		
		feedbackSignal = clip.getSample(feedbackSignal * feedbackGain) + input * inputGain;

		oscillator.setPhaseOffset(feedbackSignal + phaseOffset);
		feedbackSignal = -oscillator.getSample();		

		feedbackSignal = lpf.getSample(feedbackSignal);
		feedbackSignal = hpf.getSample(feedbackSignal);

		return output = feedbackSignal * outputGain;
	}

    void update()
    {
		highpassFrequency = pitchToFrequency(masterPitch) + hpfFreqOffset - halfBwFreqOffset;
		highpassFrequency = clamp(highpassFrequency, 5, sampleRate *.5);
        hpf.setFrequency(highpassFrequency);

		lowpassFrequency = pitchToFrequency(masterPitch) + lpfFreqOffset + halfBwFreqOffset;
		lowpassFrequency = clamp(lowpassFrequency, 0, sampleRate *.5);
        lpf.setFrequency(lowpassFrequency);
    }

    double sampleRate = 44100;
	double outputGain = 1;
   	double feedbackGain = .1;
    double inputGain = 0;
	double shape = 0;
	double phaseOffset = 0;
	double railValue = 1;
	double noiseAmp = 0;
	double noiseAmpVsFreq = 0;
	double frequency = 0;
    double halfBandWidth = 0;
	double feedbackSignal = 0;
	double halfWidth = 10000;
	double hpfFreqOffset = 0;
	double lpfFreqOffset = 0;
	double halfBwFreqOffset = 0;
	double lowpassFrequency = 0;
	double highpassFrequency = 0;
	double masterPitch = 0;

	Highpass hpf;
	Lowpass lpf;	

	SoftClipper clip;

	double output = 0;
	double feedback = 0;

    PolyBLEP oscillator;
	TriSaw triSaw;
	Ellipse ellipse;
    double dcFrequency = 10;
};

class SuperloveExtremeFilter
{
	SuperloveExtremeFilter()
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

		case 1:
		// Crunchy
		break;

		case 2:
		// Howling
		break;

		case 3:
		// Woody
		break;

		case 4:
		// Brittle
		break;

		case 5:
		// Squelchy
		break;

		case 6:
		// Quirky
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
		//filter.setNoiseAmplitude(v);
	}

	double getSample(double v)
	{
		return filter.getSample(v);
	}

	void reset()
	{
		filter.reset();
	}

	SuperloveExtremeFilterPrototype filter;

	double cutoff = 0;
	double resonance = 0;
	double sampleRate = 0;
	int preset = 0;
}