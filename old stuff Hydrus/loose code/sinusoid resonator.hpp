#include "../../Latest RS-MET/Misc/APE Scripts/rapt_for_ape.cpp"

#include "../../SoundemoteFramework/se_framework/APE/se_framework_for_ape.cpp"

#include <effect.h>

using std::clamp;

using namespace ape;

GlobalData(SinusoidResonator_LP12, "Bubbly lowpass filter");

class OnePoleLowpass
{	
public:
	void setSampleRate(double v)
	{
		freqToOmega = PI_x_2 / v;
	}
	
	void setFrequency(double v)
	{
		f = v;
		calcCoeffs();
	}
	
	double getSample(double v)
	{
		y1 = b0*v + b1*x1 + a1*y1;
		x1 = v;
		return 	y1;
	}
	
	void reset()
	{
		x1 = 0;
		y1 = 0;
	}
	
protected:	
	void calcCoeffs()
	{
		w = freqToOmega * f;
		
		double t = tan(0.5 * w);
		
		a1 = (1-t) / (1+t);
		b0 = 0.5 * (1 - a1);
		b1 = b0;
	}
	
	double w = 0;
	double f = 1;
	double sampleRate = 44100;
	double b0 = 1;
	double b1 = 0;
	double a1 = 0;
	double x1 = 0;
	double y1 = 0;
	double freqToOmega;	
};

class OnePoleHighpass
{	
public:
	void setSampleRate(double v)
	{
		freqToOmega = PI_x_2 / v;
	}
	
	void setFrequency(double v)
	{
		f = v;
		calcCoeffs();
	}
	
	double getSample(double v)
	{
		y1 = b0*v + b1*x1 + a1*y1;
		x1 = v;
		return 	y1;
	}
	
	void reset()
	{
		x1 = 0;
		y1 = 0;
	}
	
protected:	
	void calcCoeffs()
	{
		w = freqToOmega * f;
		
		double t = tan(0.5 * w);
		
		a1 = (1-t) / (1+t);
		b0 = 0.5 * (1 + a1);
		b1 = -b0;
	}
	
	double w = 0;
	double f = 1;
	double sampleRate = 44100;
	double b0 = 1;
	double b1 = 0;
	double a1 = 0;
	double x1 = 0;
	double y1 = 0;
	double freqToOmega;
};

class OnePoleBandpass
{
	OnePoleLowpass lp;
	OnePoleHighpass hp;
	
	void setSampleRate(double v)
	{
		lp.setSampleRate(v);
		hp.setSampleRate(v);
	}
	
	void setLowFrequency(double v)
	{
		lp.setFrequency(v);
	}
	
	void setHighFrequency(double v)
	{
		hp.setFrequency(v);
	}
	
	void setLowPitch(double v)
	{
		double f = pitchToFreq(v);
		lp.setFrequency(f);
	}
	void setHighPitch(double v)
	{
		double f = pitchToFreq(v);
		hp.setFrequency(f);
	}
	
	void setPitchAndBandwidth(double p, double b)
	{
		double lpf = pitchToFreq(p + b);
		double hpf = pitchToFreq(p - b);
		
		lp.setFrequency(lpf);
		hp.setFrequency(hpf);
	}	
	
	double getSample(double v)
	{
		v = lp.getSample(v);
		v = hp.getSample(v);
		return v;
	}
	
};


class FMDFilter
{
public:
	FMDFilter() = default;
	virtual ~FMDFilter() = default;

	/* Run this in your constructor */

	virtual void setupCommon()
	{
	}

	virtual void setSampleRate(double v)
	{
		sampleRate = v;
	};
	virtual void setFrequency(double v) { frequency = v; } // 0 to 1
	virtual void setResonance(double v) { resonance = v; } // 0 to 1
	virtual void setChaosAmount(double v) { chaosAmount = v; } // 0 to 1
	virtual void setInputAmplitude(double v) { inputAmplitude = v; } // suggested -10 to +10
	virtual double getSample(double in)
	{
		return in * inputAmplitude;
	};
	virtual double getFrequency() { return frequency; }
	
protected:
	double smoothingTime = 0.050;
	double sampleRate = 44100;

	double inputAmplitude = 1;
	double frequency = .5;
	double resonance = 0;
	double chaosAmount = 0;
};

class SinusoidResonator_LP12 : public FMDFilter, public Effect
{
public:

	Param<float> paramInput{ "InputGain", Range(0, 1) };
	Param<float> paramCutoff{ "Cutoff", Range(0, 1) };
	Param<float> paramOutput{ "OutputGain", Range(0, 50) };
	
	Param<float> parFrqModAmt1{ "FrqModAmt1", Range(0, 180) };
	Param<float> parFrqModAmt2{ "FrqModAmt2", Range(0, 100000) };
	
	Param<float> parPhsModAmt1{ "PhsModAmt1", Range(.04, 3) };
	Param<float> parPhsModAmt2{ "PhsModAmt2", Range(-10, 10) };
	
	Param<float> parOsc1SelfMod{ "Osc1SelfMod", Range(0, 10) };
	Param<float> parOsc2SelfMod{ "Osc2SelfMod", Range(0, 100) };
	
	Param<float> parOsc1Ratio{ "Osc1Ratio", Range(-10, 100) };
	Param<float> parOsc2Ratio{ "Osc2Ratio", Range(0.01, 100) };
	
	Param<float> parSinToSqare{ "SinToSqare", Range(0, 1) };	
	
	Param<float> parPostFltGain{ "PostFltGain", Range(0, 4) };
	
	Param<float> parOsc1Ph{ "Osc 1 Ph", Range(-1, 1) };
	Param<float> parOsc2Ph{ "Osc 2 Ph", Range(-1, 1) };
	
	Param<float> parOsc1f{ "Osc 1 f", Range(-100, 100) };
	Param<float> parOsc2f{ "Osc 2 f", Range(-10000, 10000) };	

	SinusoidResonator_LP12() 
	{
		filter.setMode(RAPT::rsLadderFilter<double, double>::LP_6);
		filter2.setMode(RAPT::rsLadderFilter<double, double>::LP_6);
		filter2.setCutoff(20000);

		dcfilter.setMode(RAPT::rsLadderFilter<double, double>::HP_6);
		dcfilter.setCutoff(10);
	
		dcfilter2.setMode(RAPT::rsLadderFilter<double, double>::HP_6);
		dcfilter2.setCutoff(5);
		
		dcfilter3.setMode(RAPT::rsLadderFilter<double, double>::HP_6);
		dcfilter3.setCutoff(5);
	}
	
	RAPT::rsLadderFilter<double, double> filter;
	RAPT::rsLadderFilter<double, double> filter2;
	RAPT::rsLadderFilter<double, double> dcfilter;
	RAPT::rsLadderFilter<double, double> dcfilter2;
	RAPT::rsLadderFilter<double, double> dcfilter3;
	
	Phasor phasor1;
	Phasor phasor2;

	double freqModAmt;
	double phaseModAmt;
	double freqRatio;
	double osc1Ratio;
	double osc2Ratio;

	double osc1Value;
	double osc2Value;
	double osc1SelfMod;
	double osc2SelfMod;

	double frequencyNormalized;
	double maxFreqNorm = 0.9;
	double freqNormInUse = 1.0;
	double resDropPoint = 0.88;

	
	void setInputAmplitude(double v) override
	{
		inputAmplitude = v * 2;
	}

	void setFrequency(double v) override
	{
		frequencyNormalized = v;
		updateFrequency();
	}

	void setResonance(double v) override // Stable Resonance
	{
		FMDFilter::setResonance(v);
	}

	void setChaosAmount(double v) override // Chaotic Resonance
	{
		phaseModAmt = jmap(v, .256, .166);
	}

	void setSampleRate(double v) override
	{
		sampleRate = v;

		filter.setSampleRate(v);
		filter2.setSampleRate(v);
		dcfilter.setSampleRate(v);
		dcfilter2.setSampleRate(v);
		dcfilter3.setSampleRate(v);
		phasor1.setSampleRate(v);
		phasor2.setSampleRate(v);

		updateFrequency();
	}
	
	double freq2 = 0, freq1 = 0;
	double getSample(double in) override
	{
		// input oscillator
		double inputSignal = inputAmplitude * in;

		// establish feedback signal
		inputSignal = osc2Value + osc1SelfMod + inputSignal;

		// osc1
		{
			// mod
			freq1 = frequency * parOsc1Ratio * parFrqModAmt1 * inputSignal;

			phasor1.setFrequency(freq1 + parOsc1f);
			phasor1.setPhaseOffset(inputSignal * parPhsModAmt1 + parOsc2Ph);
			phasor1.increment();
			osc1Value = waveshape::ellipse(phasor1.getUnipolarValue(), 0, 0, 1, parSinToSqare);
		}

		// main feedback filters
		inputSignal = filter.getSample(osc1Value) * parPostFltGain;

		// osc 1 & 2 self feedback
		osc1SelfMod = inputSignal;
		osc2SelfMod = osc2Value;

		// osc2
		double out;
		{
			freq2 = frequency * parOsc2Ratio * (parFrqModAmt2 *  dcfilter2.getSample(inputSignal) +  osc2SelfMod * parOsc1SelfMod);

			phasor2.setFrequency(freq2 + parOsc2f);
			phasor2.setPhaseOffset(inputSignal * parPhsModAmt2 + parOsc2Ph);
			phasor2.increment();
			osc2Value = waveshape::sine(phasor2.getUnipolarValue()); //morph from sine to square or tri to saw or sine to saw for alternative tones

			out = osc2Value;
			osc2Value *= parOsc2SelfMod;
		}

		return -out;
	}
	
	void updateFrequency()
	{
		freqNormInUse = std::min(frequencyNormalized, maxFreqNorm);
		
		frequency = elan::pitchToFreq(jmap(freqNormInUse, -72.96, 69.76));
		filter.setCutoff(frequency * jmap(curve(freqNormInUse, -.36), 0.248387, 0.0927813));
	}


private:
	
	void start(const IOConfig& cfg) override
	{
		setSampleRate(cfg.sampleRate);
	}
		
	void process(ape::umatrix<const float> ins, ape::umatrix<float> outs, size_t numFrames) override
	{
		setInputAmplitude(paramInput);
		setFrequency(paramCutoff);
		
    	const auto numChannels = sharedChannels();
    	
    	for(size_t n = 0; n < numFrames; ++n)
    	{
      		float x = getSample(ins[0][n]) * paramOutput;

			//outs[0][n] = freq2 * paramOutput;
			//outs[1][n] = freq2 * paramOutput;
      		
      		for(size_t c = 0; c < numChannels; ++c)
        		outs[c][n] = x;                 
    	}

	}
};