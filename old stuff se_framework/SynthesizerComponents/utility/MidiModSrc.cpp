#include "MidiModSrc.h"

void MidiModSource::setSampleRate(double v)
{
	smoother.setSampleRate(v);
}

void MidiModSource::setSmoothing(double v)
{
	smoother.setSmoothingTime(v);
}

void MidiModSource::setPitchBend(double v)
{
	pitchBendVal = v;
}

void MidiModSource::setKeytrackValue(int v)
{
	keytrackValue = v;
}

void MidiModSource::setKeytrackCenterKey(int v)
{
	keytrackCenter = v;
}

void MidiModSource::setChannelPressure(double v)
{
	channelPressureVal = v;
}

void MidiModSource::setVelocity(double v)
{
	velocityValue = v;
}

void MidiModSource::setCurrentCC(int v)
{
	currentCC = v;
}

void MidiModSource::setCCValue(int ccIndex, double ccValue)
{
	midiControllerValues[ccIndex] = ccValue;
}

void MidiModSource::setGain(double v)
{
	gain = v;
}

void MidiModSource::setOffset(double v)
{
	offset = v;
}

double MidiModSource::getValueFromCurrentMidiSource()
{
	switch (source)
	{
	case Source::pitchwheel:
		return pitchBendVal;
	case Source::velocity:
		return velocityValue;
	case Source::channelpressure:
		return channelPressureVal;
	case Source::keytrack:
		return keytrackValue - keytrackCenter;
	case Source::cc:
	default:
		return midiControllerValues[currentCC];
	}
}

double MidiModSource::getModifiedTargetValue()
{
	return targetValue * gain + offset;
}

void MidiModSource::setMidiSource(int v)
{
	source = (Source)v;
	targetValue = getValueFromCurrentMidiSource();
	smoother.setInternalValue(getModifiedTargetValue());
}

double MidiModSource::getSample()
{
	targetValue = getValueFromCurrentMidiSource();
	return smoother.getSample(getModifiedTargetValue());
}
