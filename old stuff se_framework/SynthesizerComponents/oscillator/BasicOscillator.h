#pragma once

using namespace elan;

/* Be sure to use a different noiseSeed for each oscillator. */
class BasicOscillator
{
public:

	BasicOscillator() = default;
	~BasicOscillator() = default;

	double getSample();
	int getWaveform() { return waveform; }

	void setSampleRate(double v) 
	{ 
		phasor.setSampleRate(sampleRate = v);
		sampleRateInv = 1/v;
		updateTimerInc();
		setFrequency(frequency);
		polyBLEP.setSampleRate(v);
	}

	void reset() 
	{ 
		phasor.reset(); 
		timer = 1.0;
		polyBLEP.reset();
		//noise.reset();
	}

	void setFrequency(double v)
	{ 
		frequency = v;
		updateTimerInc(); 
		frequencyNeedsUpdating = true;
	}

	// -1 to +1
	void setPhaseOffset(double v)
	{ 
		phasor.setPhaseOffset(phaseOffset = v); 
		polyBLEP.setPhaseOffset(v);
	}

	// 0 to 1
	void setNoiseAmp(double v) 
	{ 
		noiseAmp = v;
		frequencyNeedsUpdating = true;
	}

	void setNoiseSeed(int v)
	{
		noise.setSeed(45973 + v);
		noise.reset();
	}

	enum WaveformMode { NONE, NOISE_ONLY, SINE, TRI, SAW, PHASOR, SQUARE, RAND_SQUARE, RAND_TRI, RAND_SAW, RAND_SLOPE_BI, RAND_SLOPE_UNI, SAW_UP_NOAA, SAW_DOWN_NOAA, TRI_NOAA, ELLIPSE };
	void setWaveform(int v) 
	{
		switch(v)
		{
		case WaveformMode::TRI:
			polyBLEP.setWaveform(PolyBLEP::Waveform::TRIANGLE);
			break;
		case WaveformMode::SAW:
			polyBLEP.setWaveform(PolyBLEP::Waveform::SAWTOOTH);
			break;
		case WaveformMode::SQUARE:
			polyBLEP.setWaveform(PolyBLEP::Waveform::SQUARE);
			break;			
		}
		waveform = (WaveformMode)v;
	}

	void setA(double v) { A = v; }
	void setB(double v) { elan::sinCos(v, &B_sin, &B_cos); }
	void setC(double v) { C = v; }

	Phasor phasor;
protected:
	/* Core Objects */
	
	elan::NoiseGenerator noise;
	double current_frequency = 0.0;
	bool frequencyNeedsUpdating = true;
	PolyBLEP polyBLEP;

	/* Parameters */
	WaveformMode waveform = WaveformMode::SINE;

	/* Internal Values*/
	double frequency = 0.0;
	double current_value = 0.0;
	double noiseAmp = 0;	
	double phaseOffset = 0;

	/* for Ellipse */
	double A = 0;
	double B_sin = 0;
	double B_cos = 0;
	double C = 0;

	// timer NEED TO FACTOR OUT
	double timer = 1.0;
	double timerInc = 0.0;
	double sampleRate = 44100.0;
	double sampleRateInv = 1/44100.0;
	double prev_rand_val = 0.0;
	double current_rand_val = 0.0;
	double current_tri_val;
	double tri_inc = 0.0;
	double rand_val_2 = 0.0;
	double current_freq = 0;
	void updateTimerInc()
	{
		timerInc = sampleRateInv * std::abs(frequency + frequency * current_rand_val*noiseAmp);
	}
	void updateTimerInc(double f)
	{
    timerInc = jmin(sampleRateInv * std::abs(f), 1.0);
	}

	void incrememt() { timer += timerInc; }

	void triggerEvent();
	
	double do_RAND_SQUARE_stuff();
	double do_RAND_SAW_stuff();
	double do_RAND_SLOPE_stuff();
	double do_RAND_SLOPE_UNI_stuff();
	
	void triggerTriEvent();

	double do_RAND_TRI_stuff();
};
