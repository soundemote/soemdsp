#pragma once

class XoxosOscillatorCore : public MonoSynth
{	
public:  
	XoxosOscillatorCore() {}
  virtual ~XoxosOscillatorCore() = default;

	/* Mono Synth */
	void setSampleRate(double v);
	void processSampleFrame(double *outL, double *outR) override;
	void triggerFrequencyChange();
	void triggerAttack();
	void triggerRelease();
	bool isSilent() override;
	void Reset();

	void incrementPitchGlide() 
	{ 
		// not implemented 
		jassertfalse;		
	}

	XoxosOscillator osc;
	double currentFrequency = 440.0;
};

class XoxosOscillatorCoreOversampled : public XoxosOscillatorCore
{
public:
	virtual ~XoxosOscillatorCoreOversampled() = default;

	void setOversampling(int newFactor)
	{
		oversampling = newFactor;
		XoxosOscillatorCore::setSampleRate(oversampling * hostSampleRate);

		antiAliasFilterL.setSubDivision(oversampling);
		antiAliasFilterR.setSubDivision(oversampling);
	}

	void setSampleRate(double sr);

	void processSampleFrame(double *outL, double *outR) override;
protected:
	double hostSampleRate = 44100.0;
	double oversampling = 1.0;	
	RAPT::rsEllipticSubBandFilter<double, double> antiAliasFilterL, antiAliasFilterR;
};
