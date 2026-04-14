#include "MusicalSaw.cxx"
#include "../Randoms/NoiseGenerator.cxx"
#include "../Randoms/FlexibleRandomWalk.cxx"
#include "../Filters/Bandpass.cxx"
#include "../Filters/Highpass.cxx"
#include "../Randoms/SampleAndHold.cxx"

class NoiseSaw
{
	NoiseSaw()
	{
		bpf.setWidth(0);
		noise.setSeed(4);
		walk.setSeed(3);
	}

	double sampleRate = 0;
	double frequency = 0;
	double frequencyOffset = 0;
	double phase = 0;
	double output = 0;
	double morph = 1;
	double value = 0;
	double noiseLevelPM = 0;
	double smoothLevelPM = 0;
	double walkLevelPM = 0;
	double noiseLevelFM = 0;
	double smoothLevelFM = 0;
	double walkLevelFM = 0;
	double noiseSignal = 0;
	double smoothSignal = 0;
	double walkSignal = 0;

	void reset()
	{
		osc.reset();
		noise.reset();
		bpf.reset();
		SnH.reset();
		hpf.reset();
		hpf2.reset();
		walk.reset();
		lpf.reset();
	}

	void setPhase(double v)
	{
		osc.setPhase(v);
	}

	void setPhaseOffset(double v)
	{
		osc.setPhaseOffset(v);
	}

	void setSampleRate(double v)
	{
		osc.setSampleRate(v);
		SnH.setSampleRate(v);
		bpf.setSampleRate(v);
		hpf.setSampleRate(v);
		hpf2.setSampleRate(v);
		walk.setSampleRate(v);
		lpf.setSampleRate(v);
	}

	void setFrequency(double v)
	{
		osc.setFrequency(v);
	}


	// 0 to 1, 0 being sine, 1 being saw
	void setMorph(double v)
	{
		osc.setMorph(v);
	}

	void setNoiseFilterFrequency(double v)
	{
		bpf.setFrequency(v);
	}

	void setSmoothFrequency(double v)
	{
		SnH.setFrequency(v);
	}

	void setHighpassFrequency(double v)
	{
		hpf.setFrequency(v);
		hpf2.setFrequency(v);
	}

	void setLowpassFrequency(double v)
	{
		lpf.setFrequency(v);
	}

	void setNoiseAmplitudePM(double v)
	{
		noiseLevelPM = v;
	}

	void setSmoothAmplitudePM(double v)
	{
		smoothLevelPM = v;
	}

	void setWalkAmplitudePM(double v)
	{
		walkLevelPM = v;
	}
	
	void setNoiseAmplitudeFM(double v)
	{
		noiseLevelFM = v;
	}

	void setSmoothAmplitudeFM(double v)
	{
		smoothLevelFM = v;
	}

	void setWalkAmplitudeFM(double v)
	{
		walkLevelFM = v;
	}

	void setNoiseToSmoothBias(double v)
	{
		//forkMixer(1, v, noiseLevel, SnHLevel);
	}

	double getSample()
	{
		noiseSignal = noise.getSampleBipolar();
		noiseSignal = bpf.getSample(noiseSignal);

		SnH.increment();
		smoothSignal = SnH.getSmoothedValue();

		walkSignal = walk.getSample();

		osc.setPhaseOffset(lpf.getSample(hpf.getSample(walkSignal * walkLevelPM + noiseSignal  * noiseLevelPM + smoothSignal  * smoothLevelPM)));


		osc.setFrequencyOffset(pitchToFrequency(noiseSignal * noiseLevelFM * 100 + walkSignal   * walkLevelFM * 100 + smoothSignal * smoothLevelFM * 100));		

		return osc.getSample();
	}

	void setIncrement(double v)
	{
		osc.setIncrement(v);
	}

	MusicalSaw osc;
	NoiseGenerator noise;
	Bandpass bpf;
	SampleAndHold SnH;
	Highpass hpf, hpf2;
	Lowpass lpf;
	FlexibleRandomWalk walk;
	
}