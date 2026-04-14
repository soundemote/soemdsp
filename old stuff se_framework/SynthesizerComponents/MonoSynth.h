#pragma once

class MonoSynth
{
public:
  
  virtual ~MonoSynth() = default;

	//virtual void setSampleRate(double newSampleRate);
	virtual void processSampleFrame(double* /*outL*/, double* /*outR*/) {};

	// call this per sample before returning final output value
	void handleChannelMode(double * left, double * right); /* optional */

	/* Channel Mode & Amplitude */
	enum ChannelMode { Stereo, Mono, Left, Right };
	static const vector<String> ChannelModeMenuItems;
	void setChannelMode(int v) { channelMode = (ChannelMode)v; }
	void setOutputAmplitude(double v) { outputAmp = v; }
	
	/* Frequency & Tempo */
	void setPlayheadInfo(juce::AudioPlayHead::CurrentPositionInfo info) { playheadInfo = info; }

	bool resetWhenBeatTempoOrMultChanes = false;

	virtual bool isSilent() 
	{ 
		return std::abs(outputAmp) <= 1.e-6;
	}

protected:

	// data:
	juce::AudioPlayHead::CurrentPositionInfo playheadInfo{};
	double sampleRate = 44100.0;

	/* OPTIONS */
	ChannelMode channelMode = ChannelMode::Stereo;

	double getBarPhase();
	double getPhaseBasedOnBarsPerCycle(double barsPerCycle);

	static const double powersOfTwo[13];

	double outputAmp{ 1.0 };
};
