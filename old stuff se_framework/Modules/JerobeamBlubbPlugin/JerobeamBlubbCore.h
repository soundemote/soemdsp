#pragma once

#include "se_framework/ElanSynthLib/ElanSynthLib.h"
#include "se_framework/ElanSynthLib/myparams.h"

class JerobeamBlubbCore : public MonoSynth
{	
public:  
	JerobeamBlubbCore() {}
  virtual ~JerobeamBlubbCore() = default;

	/* Mono Synth */
	void setSampleRate(double newSampleRate) override;
	void processSampleFrame(double *outL, double *outR) override;
	void triggerFrequencyChange() override;
	void triggerAttack() override;
	void triggerRelease() override;
	bool isSilent() override;
	void Reset() override;

	JerobeamBlubb jbBlubb;
};

//=================================================================================================

class JerobeamBlubbCoreOversampled : public JerobeamBlubbCore
{
public:
	virtual ~JerobeamBlubbCoreOversampled() = default;

	void setOversampling(int newFactor)
	{
		oversampling = newFactor;
		JerobeamBlubbCore::setSampleRate(oversampling * hostSampleRate);

		antiAliasFilterL.setSubDivision(oversampling);
		antiAliasFilterR.setSubDivision(oversampling);
	}

	void setSampleRate(double sr) override
	{
		hostSampleRate = sr;
		JerobeamBlubbCore::setSampleRate(oversampling * hostSampleRate);
	}

	void processSampleFrame(double *outL, double *outR) override
	{
		for (int i = 0; i < oversampling; i++)
		{
			JerobeamBlubbCore::processSampleFrame(outL, outR);

			*outL = antiAliasFilterL.getSampleDirect1(*outL);
			*outR = antiAliasFilterR.getSampleDirect1(*outR);
		}
	}
protected:
	double hostSampleRate = 44100.0;
	double oversampling = 1.0;
	rosic::EllipticSubBandFilter antiAliasFilterL, antiAliasFilterR;
};