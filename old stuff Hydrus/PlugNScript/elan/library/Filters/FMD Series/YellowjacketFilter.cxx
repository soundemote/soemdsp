#include "../OnePoleFilter.cxx"
#include "../../Randoms/NoiseGenerator.cxx"
#include "../../Oscillators/Phasor.cxx"
#include "../../Oscillators/Ellipse.cxx"
#include "../../Dynamics/SoftClipper.cxx"

class YellowjacketFilterPrototype
{
	YellowjacketFilterPrototype() 
	{
		filter.setMode(1);
        softClip.setWidth(9999);
	}
	
    // objects
	OnePoleFilter filter;
	Phasor phasor;
    Ellipse ellipse;
    SoftClipper softClip;

    // settable values
    double inputGain = 1;
    double outputGain = 1;
    double feedbackGain = .1;
    double fModAmp = 1.9400625; // Increase freq modulation constant for grittier sound and/or go negative for a chaotic alternative
    double oscSinToSquare = 0;
    double oscGain = 0.635417;
    double oscSelfModGain = 20;
    double oscFbGain = 1.3892758936011171;
    double railValue = 7; // 2.6615 was the original input clamp level
    double inputGainPostClamp = 1.04025;   

    // states
    double masterPitch = 0;
    double filterPitchOffset = 0;
    double oscPitchOffset = 0;
    double oscFreqOffset = 0;
    double oscValue = 0;
    double feedback = 0;
    double oscSelfMod = 0;   

	void setSampleRate(double v)
	{
		filter.setSampleRate(v);
		phasor.setSampleRate(v);
	}
	
	void setInputGain(double v)
	{
		inputGain = v;
	}

    void setOutputGain(double v)
    {
        outputGain = v;
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

    void setSoftClipperWidth(double v)
    {
        softClip.setWidth(v);
    }

    void reset()
    {
        filter.reset();
        phasor.reset();

        double oscValue = 0;
        double feedback = 0;
        double oscSelfMod = 0;   
    }
	
	double getSample(double v)
	{
	// input
		double inputSignal = clamp(v * inputGain, -railValue, +railValue); 

		// establish feedback signal
		inputSignal = oscSelfMod + inputGainPostClamp * inputSignal + feedback;

		// mod
		phasor.setFrequency(oscFreqOffset * fModAmp * inputSignal);
		phasor.inc();
		oscValue = ellipse.getValue(phasor.getValue(), 0.0, -0.71286768918541499, 0.70129855105756955, oscSinToSquare);

		//  amp
		oscValue = softClip.getSample(oscValue * oscGain);
        
		// main feedback filter	
		inputSignal = filter.getSample(oscValue);

		// osc self feedback
		oscSelfMod = inputSignal * oscSelfModGain;

		// amplitude envelope
		double output = oscFbGain * oscValue; // Replace oscValue with inputSignal for lowpass sound

		feedback = output * feedbackGain; // TODO: remove this 0.5 multiplier by multiplying graph values, also place feedback gain value in the first line of code

		return output * outputGain;
	}
	
	protected void updateFrequency()
	{
        filter.setFrequency(pitchToFrequency(masterPitch + filterPitchOffset));		
		oscFreqOffset = pitchToFrequency(masterPitch + oscPitchOffset);		
	}
};