#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "se_framework/ElanSynthLib/ElanSynthLib.h"
#include "se_framework/ElanSynthLib/myparams.h"

class JerobeamKeplerBouwkampCore : public MonoSynth
{	
public:  
	JerobeamKeplerBouwkampCore() {}
  virtual ~JerobeamKeplerBouwkampCore() = default;

	/* Mono Synth */
	void setSampleRate(double newSampleRate) override;
	void processSampleFrame(double *outL, double *outR) override;
	void triggerFrequencyChange() override;
	void triggerAttack() override;
	void triggerRelease() override;
	bool isSilent() override;
	void Reset() override;

	JerobeamKeplerBouwkamp jbKepler;
};

//=================================================================================================

class JerobeamKeplerBouwkampCoreOversampled : public JerobeamKeplerBouwkampCore
{
public:
	virtual ~JerobeamKeplerBouwkampCoreOversampled() = default;

	void setOversampling(int newFactor)
	{
		oversampling = newFactor;
		JerobeamKeplerBouwkampCore::setSampleRate(oversampling * hostSampleRate);

		antiAliasFilterL.setSubDivision(oversampling);
		antiAliasFilterR.setSubDivision(oversampling);
	}

	void setSampleRate(double sr) override
	{
		hostSampleRate = sr;
		JerobeamKeplerBouwkampCore::setSampleRate(oversampling * hostSampleRate);
	}

	void processSampleFrame(double *outL, double *outR) override
	{
		for (int i = 0; i < oversampling; i++)
		{
			JerobeamKeplerBouwkampCore::processSampleFrame(outL, outR);

			*outL = antiAliasFilterL.getSampleDirect1(*outL);
			*outR = antiAliasFilterR.getSampleDirect1(*outR);
		}
	}
protected:
	double hostSampleRate = 44100.0;
	double oversampling = 1.0;
	rosic::EllipticSubBandFilter antiAliasFilterL, antiAliasFilterR;
};