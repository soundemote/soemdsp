#pragma once

#include "MaxMSPHelper.h"

class JerobeamNyquistShannon : protected gen
{
public:
	JerobeamNyquistShannon() { toneSmoother.setSmootherType(ParamSmoother::type::LINEAR); }
	virtual ~JerobeamNyquistShannon() = default;

	void setFrequencyA(double v) 
	{
		userFreqA = v;
		frequencyToPitch = RAPT::rsFreqToPitch(std::abs(userFreqA))-48;
		recalculatePhaseInc(); 
	}

	void getSampleFrame(double *outL, double *outR);

	void setSampleRate(double v) 
	{ 
		toneSmoother.setSampleRate(v);
		phasor.setSampleRate(v);
		phaseRotator.setSampleRate(v);
		recalculatePhaseInc(); 
	}

	void reset() 
	{ 
		phasor.reset();
		phaseRotator.reset();
	}

	// 0 to 127, used for tone modulation, so be sure to call this whenever midi note changes.
	void setMidiNote(int v) { midiNote = v-48;  }
	// 1.e-6 to 100
	void setRate(double v) { sr = v; }
	// 0 to 1
	void setSampleDots(double v) { blend = 1/(1-v+0.001); }
	// 0 to 1
	void setPhase(double v) { phasor.setPhaseOffset(v); }
	// 0 to 100
	void setFrequencyB(double v) { pitch = v; recalculatePhaseInc(); }
	// 0 to 1
	void setSubPhase(double v) { phaseRotator.setPhaseOffset(-v); }
	// -20 to +20
	void setSubPhaseRotationSpeed(double v) { phaseRotator.setFrequency(-v); }
	// -100 to 100
	void setTone(double v) { tone = v; }
	// 0 to inf
	void setToneSmoother(double v) { toneSmoother.setSmoothingTime(v); }
	// 0 to 1
	void setArtifact(double v) { tri = 1-v; }
	// true or false
	void setEnableToneModPitch(bool v) { toneModPitch = v; updateToneMode(); }
	// true or false
	void setEnableToneModFreq(bool v) { toneModFreq = v; updateToneMode(); }
	// true or false
	void setEnableToneModNote(bool v) { toneModNote = v; updateToneMode(); }

	double getUnipolarTrianglePhasorValue() { return fphas; }

	Phasor phasor;
	Phasor phaseRotator;
	ParamSmoother toneSmoother;

protected:
	void recalculatePhaseInc() 
	{ 
		phasor.setFrequency(userFreqA * pitch);
	}

	enum ToneMode{None, Note, Pitch, Note_Pitch, Freq, Note_Freq, Pitch_Freq, Note_Pitch_Freq };
	void updateToneMode()
	{
		toneMode = ToneMode((toneModNote ? 1 : 0) + (toneModPitch ? 2 : 0) + (toneModFreq ? 4 : 0));
	}

	/* Parameters */
	double freq = 440;
	double midiNote = 0;
	double pitch = 5;
	double sr = 20;
	double blend = 0.2;
	double tri = 0;
	double tone = 0;
	ToneMode toneMode = ToneMode::Pitch;
	bool toneModPitch = true;
	bool toneModFreq = false;
	bool toneModNote = false;

	double
		userFreqA = 0,
		userFreqB = 0,
		userTone = 0;	

	/* Internal States */
	double frequencyToPitch = 0;
	double fphas = 0;
};
