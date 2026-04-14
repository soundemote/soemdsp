#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "se_framework/ElanSynthLib/ElanSynthLib.h"
#include "se_framework/ElanSynthLib/myparams.h"

class JerobeamBoingCore : public MonoSynth
{	
public:  
	JerobeamBoingCore() {}
  virtual ~JerobeamBoingCore() = default;

	/* Mono Synth */
	void setSampleRate(double newSampleRate) override;
	void processSampleFrame(double *outL, double *outR) override;
	void triggerFrequencyChange() override;
	void triggerAttack() override;
	void triggerRelease() override;
	bool isSilent() override;
	void Reset() override;

	JerobeamBoing jbBoing;
};

//=================================================================================================

class JerobeamBoingCoreOversampled : public JerobeamBoingCore
{
public:
	virtual ~JerobeamBoingCoreOversampled() = default;

	void setOversampling(int newFactor)
	{
		oversampling = newFactor;
		JerobeamBoingCore::setSampleRate(oversampling * hostSampleRate);

		antiAliasFilterL.setSubDivision(oversampling);
		antiAliasFilterR.setSubDivision(oversampling);
	}

	void setSampleRate(double sr) override
	{
		hostSampleRate = sr;
		JerobeamBoingCore::setSampleRate(oversampling * hostSampleRate);
	}

	void processSampleFrame(double *outL, double *outR) override
	{
		for (int i = 0; i < oversampling; i++)
		{
			JerobeamBoingCore::processSampleFrame(outL, outR);

			*outL = antiAliasFilterL.getSampleDirect1(*outL);
			*outR = antiAliasFilterR.getSampleDirect1(*outR);
		}
	}
protected:
	double hostSampleRate = 44100.0;
	double oversampling = 1.0;
	rosic::EllipticSubBandFilter antiAliasFilterL, antiAliasFilterR;
};