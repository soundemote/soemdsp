#pragma once

class MidiModSource
{
public:

	double getSample();

	void setSampleRate(double v);
	void setSmoothing(double v);

	void setMidiSource(int v);

	void setPitchBend(double v);
	void setKeytrackValue(int v);
	void setKeytrackCenterKey(int v);
	void setChannelPressure(double v);
	void setVelocity(double v);

	void setCurrentCC(int v);
	void setCCValue(int ccIndex, double ccValue);

	void setGain(double v);
	void setOffset(double v);

	enum Source { pitchwheel, velocity, channelpressure, keytrack, cc };
	Source source = Source::cc;	

protected:
	/* Core Objects */
	ParamSmoother smoother;

	/* Internal */
	double
		pitchBendVal = 0.0,
		channelPressureVal = 0.0,
		keytrackValue = 0.0,
		keytrackCenter = 0.0,
		velocityValue = 0.0,
		ccValue = 0.0,
		gain = 1.0,
		offset = 0.0;

	int
		currentCC = 1;

	double getValueFromCurrentMidiSource();
	double getModifiedTargetValue();
	double targetValue;
	std::array<double, 128> midiControllerValues{ 0 };
};
