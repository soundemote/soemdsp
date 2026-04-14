#pragma once

#include "se_framework/ElanSynthLib/ElanSynthLib.h"

class ChaoticaCore : public MonoSynth
{	
public:  
	ChaoticaCore() {}
  virtual ~ChaoticaCore() = default;

	/* Mono Synth */
	void setSampleRate(double v) override;
	void processSampleFrame(double *outL, double *outR) override;
	void triggerFrequencyChange() override;
	void triggerAttack() override;
	void triggerRelease() override;
	bool isSilent() override;
	void Reset() override;

	IanFritzOneEyeChaotica osc;
	BasicOscillator saw;
	double sawVal = 0;
	LinearADSR adsr;
	rosic::TwoPoleBandpass filter;
};

class ChaoticaCoreOversampled : public ChaoticaCore
{
public:
	virtual ~ChaoticaCoreOversampled() = default;

	void setOversampling(int newFactor)
	{
		oversampling = newFactor;
		ChaoticaCore::setSampleRate(oversampling * hostSampleRate);

		antiAliasFilterL.setSubDivision(oversampling);
		antiAliasFilterR.setSubDivision(oversampling);
	}

	void setSampleRate(double sr) override;

	void processSampleFrame(double *outL, double *outR) override;
protected:
	double hostSampleRate = 44100.0;
	double oversampling = 1.0;
	rosic::EllipticSubBandFilter antiAliasFilterL, antiAliasFilterR;
};