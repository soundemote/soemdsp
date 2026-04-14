#include "../OnePoleFilter.cxx"
#include "../../Randoms/NoiseGenerator.cxx"
#include "../../Oscillators/Phasor.cxx"
#include "../../Oscillators/Ellipse.cxx"

class StrangeResonatorPrototype
{
	StrangeResonatorPrototype() 
	{
		filter.setMode(1);

		dcfilter.setMode(2);
		dcfilter.setFrequency(5);
	}
	
	OnePoleFilter filter;
	OnePoleFilter dcfilter;	
	Phasor phasor1;
	Phasor phasor2;
    Ellipse ellipse;

	double osc1Ratio = 0;
	double osc2Ratio = 0;
	double osc1Value = 0;
	double osc2Value = 0;
	double osc1SelfMod = 0;
	double osc2SelfMod = 0;
	double osc1SinToSquare = 0;
	double osc2SinToSquare = 0;
	double osc1FeedbackAmp = 0;
	double osc1SelfModAmp = 0;
	double osc2FeedbackAmp = 0;
	double osc2SelfModAmp = 0;

    double Pm1Amount = 0;
    double Pm2Amount = 0;
    double Fm1Amount = 0;
    double Fm2Amount = 0;
    double osc1Ph = 0;
    double osc2Ph = 0;
    double osc1F = 0;
    double osc2F = 0;

	double sampleRate = 44100;
	double inputGain = 1;
    double masterPitch = 0;
	double filterPitchOffset = 0;
    double oscPitchOffset = 0;
    double oscFreqOffset = 0;
    double postFilterGain = 0;
	
	void setInputGain(double v)
	{
		inputGain = v;
	}

	void setMasterPitch(double v)
	{
        masterPitch = v;
        updateFrequency();
	}

    void setFilterPitchOffset(double v)
    {
        filterPitchOffset = v;
        updateFrequency();
    }

    void setOscPitchOffset(double v)
    {
        oscPitchOffset = v;
        updateFrequency();
    }

	void setSampleRate(double v)
	{
		sampleRate = v;

		filter.setSampleRate(v);
		dcfilter.setSampleRate(v);
		phasor1.setSampleRate(v);
		phasor2.setSampleRate(v);
	}
	
	double getSample(double v)
	{
		// input oscillator
		double inputSignal = v * inputGain;

		// establish feedback signal
		inputSignal = osc2Value + osc1SelfMod + inputSignal;

		// osc1
		{
			// mod
			double fm = Fm1Amount * inputSignal * osc1SelfModAmp;
			double freq1 = oscFreqOffset * osc1Ratio * fm;

			phasor1.setFrequency(freq1 + osc1F);
			phasor1.setPhaseOffset(inputSignal * Pm1Amount + osc1Ph);
			phasor1.inc();
			osc1Value = ellipse.getValueSinToSquare(phasor1.getValue(), osc1SinToSquare);

			//amp
			osc1Value *= osc1FeedbackAmp; // .05 for alternative triangle resonator
		}

		// main feedback filters
		inputSignal = filter.getSample(osc1Value) * postFilterGain;

		// osc 1 & 2 self feedback
		osc1SelfMod = inputSignal;
		osc2SelfMod = osc2Value;

		// osc2
		double output = 0;
		{
			double fm = Fm2Amount * inputSignal + osc2SelfMod * osc2SelfModAmp;
			double freq2 = oscFreqOffset * osc2Ratio * fm;

			phasor2.setFrequency(freq2 + osc2F);
			phasor2.setPhaseOffset(inputSignal * Pm2Amount + osc2Ph);
			phasor2.inc();
			osc2Value = ellipse.getValueSinToSquare(phasor2.getValue(), osc2SinToSquare);

			output = dcfilter.getSample(osc2Value);

			osc2Value *= osc2FeedbackAmp;
		}

		return -output;
	}
	
	void updateFrequency()
	{
        filter.setFrequency(pitchToFrequency(masterPitch + filterPitchOffset));		
		oscFreqOffset = pitchToFrequency(masterPitch + oscPitchOffset);		
	}
};