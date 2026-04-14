#include "../Randoms/FlexibleRandomWalk.cxx"
#include "../Filters/Lowpass.cxx"
#include "SineSaw.cxx"
#include "PolyBLEP.cxx"
#include "../Randoms/VibratoGenerator.cxx"
#include "../Envelopes/LinearEnvelope.cxx"

class HypersawUnit
{
	HypersawUnit()
	{
		osc.setMorph(1);
		//osc.setWaveform(WAVEFORM::SAWTOOTH);

		lpf.setMode(1);
		lpf.setFrequency(20000);

		hpf.setMode(2);
		hpf.setFrequency(0);

		smoother.setFrequency(20000);

		env.setSustainAmplitude(1);

		phaseDistribution = idx * div;
	}    

	double walkSignal = 0;
	double filteredWalkSignal = 0;
	double vibratoSignal = 0;
	double phaseDistribution = 0;
	double envelopeSignal = 0;

    double getSample()
    { 
		vibratoSignal = 0;
		filteredWalkSignal = 0;

		currentPitch = smoother.getSample(targetPitch) + pitchOffset;
		currentFrequency = pitchToFrequency(currentPitch + pitchOffsetFromDetune);

		/* HYPERSAW */
		if (PMDriftAmount > 0)
		{	
			walkPM.setDetail(map(rationalCurve(PMDriftDetail, -.9),0,currentFrequency*2));
			walkSignal = walkPM.getSample() * PMDriftAmount;
			filteredWalkSignal = lpf.getSample(hpf.getSample(walkSignal));
		}
		
		/* SYMHPONIC SAW */		
		if (vibrato.range > 0)
			vibratoSignal = vibrato.getSample();		

		envelopeSignal = env.getSample();

		phaseOffset = wrapPhaseExtreme(filteredWalkSignal + vibratoSignal + phaseDistribution + randomPhaseOffset);

        osc.setPhaseOffset(phaseOffset);
        osc.setFrequency(currentFrequency);

        return osc.getSample() * envelopeSignal;
    }

    void setPitchDetuneAmount(double v)
    {
        pitchDetuneAmount = v;
        updateDetune();
    }
    void setPitchDetuneCurve(double v)
    {
        pitchDetuneCurve = clamp(v, -0.999999, 0.99999);
        updateDetune();
    }

    void updateDetune()
    {        
        if (idx % 2 == 0)
            pitchOffsetFromDetune = rationalCurve(div * idx, pitchDetuneCurve) * 12 * (detuneAmount + detuneAmount);
        else
            pitchOffsetFromDetune = rationalCurve(div * idx, pitchDetuneCurve) * 12 * -(detuneAmount + detuneAmount);
   }

	// seed for A: random phase B: random frequency C: random vibrato frequency d: random vibrato amplitude
	void setSeed(int a, int b, int c, int  d, int e)
	{
		walkPM.setSeed(a);
		walkFM.setSeed(b);
		vibrato.setSeed(c, d);
		r.setSeed(e);

		portamentoRandomValue = r.getSampleUnipolar();

		updateAttack();
		updateRelease();
	}

	void setSampleRate(double v)
	{
		osc.setSampleRate(v);
		walkPM.setSampleRate(v);
		walkFM.setSampleRate(v);
		lpf.setSampleRate(v);
		hpf.setSampleRate(v);
		vibrato.setSampleRate(v);
		smoother.setSampleRate(v);
		env.setSampleRate(v);
	}

	void reset()
	{
		osc.reset();
		walkPM.reset();
		walkFM.reset();
		lpf.reset();
		hpf.reset();
		vibrato.reset();
		smoother.reset();
		env.reset();

		randomPhaseOffset = rand(-randomPhaseOffsetRange,randomPhaseOffsetRange);
	}

    void setOscMorph(double v)
    {
        osc.setMorph(v);
    }
	void setPitch(double v)
	{
		targetPitch = v;
	}
	void setPitchOffset(double v)
	{
		pitchOffset = v;
	}
	void setFilteredPhaseOffset(double v)
	{
		filteredPhaseOffset = v;
	}
    void setPortamentoSeconds(double v)
	{
		portamentoSeconds = max(v, 1.e-4);
        updatePortamento();
    }
	void setPortamentoRandomMult(double v)
	{
		portamentoRandomMult = v;
		updatePortamento();
	}
	void updatePortamento()
	{
		smoother.setFrequency(1.0/max(portamentoSeconds + portamentoRandomValue * portamentoRandomMult, 1.e-4));
	}

	void setPMDriftAmount(double v)
	{
		PMDriftAmount = v;
	}
    void setPMDriftColor(double v)
	{
		walkPM.setColor(v);
	}
	void setPMDriftDetail(double v)
	{		
		PMDriftDetail = v;
		//walkPM.setDetail(min(v,currentFrequency*2));
		//walkPM.setDetail(v);
	}
	void setDriftHPF(double v)
	{
		hpf.setFrequency(v);
	}
	void setDriftLPF(double v)
	{
		lpf.setFrequency(v);
	}

	void setVibratoSpeed(double v)
	{
		vibrato.setFrequency(v);
	}
	void setVibratoRandomSpeed(double v)
	{
		vibrato.setRandomSpeed(v);
	}
	void setVibratoRandomSpeedOffset(double v)
	{
		vibrato.setRandomSpeedOffset(v);
	}
	void setVibratoRange(double v)
	{
		vibrato.setRange(v);
	}
	void setVibratoRandomRange(double v)
	{
		vibrato.setRandomRange(v);
	}
	void setVibratoRandomRangeOffset(double v)
	{
		vibrato.setRandomRangeOffset(v);
	}
	void setVibratoDesync(double v)
	{
		vibrato.setPhaseDesync(v);
	}

	void triggerAttack()
	{
		updateAttack();
		updateDelay();

		//if (env.isIdle())
			//randomPhaseOffset = rand(-randomPhaseOffsetRange,randomPhaseOffsetRange);

		env.triggerAttack();
	}

	void triggerRelease()
	{
		updateRelease();
		env.triggerRelease();
	}

	void setRandomDelayRange(double v)
	{
		delaySecondsRandomRange = v;
		updateDelay();
	}
	void setAttackSeconds(double v)
	{
		attackSeconds = v;
		updateAttack();
	}
	void setAttackSecondsRandomMult(double v)
	{
		attackSecondsRandomMult = v;
		updateAttack();
	}
	void setReleaseSeconds(double v)
	{
		releaseSeconds = v;		
		updateRelease();
	}
	void setReleaseSecondsRandomMult(double v)
	{
		releaseSecondsRandomMult = v;
		updateRelease();
	}

	void updateDelay()
	{
		double randomDelay = r.getSampleUnipolar();
		env.setDelayTime((randomDelay * idx * div) * delaySecondsRandomRange);
	}
	void updateAttack()
	{
		double randomAttack = r.getSampleUnipolar();
		env.setAttackTime(attackSeconds + randomAttack * attackSecondsRandomMult);		
	}
	void updateRelease()
	{
		double randomRelease = r.getSampleUnipolar();	
		double sec = releaseSeconds + randomRelease * releaseSecondsRandomMult;
		env.setReleaseTime(sec);
		env.setDecayTime(sec);
	}

	double targetPitch = 69;
	double pitchOffset = 0;

    double pitchDetuneAmount = 0;
    double pitchDetuneCurve = 0;
    double pitchOffsetFromDetune = 0.01;

	double currentPitch = 69;
	double currentFrequency = 440;
	double lastPitchValue = 69;
	double phaseOffset = 0;
	double randomPhaseOffset = 0;
	double randomPhaseOffsetRange = 0.3;

	double PMDriftAmount = 0;
	double PMDriftDetail = 0;

	SineSaw osc;
	//PolyBLEP osc;
	FlexibleRandomWalk walkPM;
	FlexibleRandomWalk walkFM;
	VibratoGenerator vibrato;
	double targetFrequency = 440;
	double filteredPhaseOffset = 0;
	OnePoleFilter lpf;
	OnePoleFilter hpf;
	LinearEnvelope env;

	Lowpass smoother;
	double portamentoSeconds = 0;
	double portamentoRandomValue = 0;
	double portamentoRandomMult = 0;

	NoiseGenerator r;

	double delaySecondsRandomRange = 0;

	double attackSeconds = 0;
	double attackSecondsRandomMult   = 0;

	double releaseSeconds = 0;
	double releaseSecondsRandomMult   = 0;

	int idx = 0;
	double div = 1;
	// equal to 1 / number of sawtooths for spreading things between 0 and 1 such as oscillator phase.
	// set by Hypersaw parent
}
