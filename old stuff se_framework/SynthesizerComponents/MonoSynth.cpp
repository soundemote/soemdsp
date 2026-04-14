#include "MonoSynth.h"

const vector<String> MonoSynth::ChannelModeMenuItems{ "Stereo", "Mono", "Left (Mono)", "Right (Mono)" };

double MonoSynth::getBarPhase()
{
	return (playheadInfo.ppqPosition - playheadInfo.ppqPositionOfLastBarStart) * 0.25;
}
double MonoSynth::getPhaseBasedOnBarsPerCycle(double barsPerCycle)
{
	jassert(barsPerCycle > 0);

	if (barsPerCycle <= 1)
	{
		return fmod(getBarPhase(), barsPerCycle) / barsPerCycle;
	}
	else
	{
		int barLengthsPassed = int((playheadInfo.ppqPositionOfLastBarStart*0.25) / barsPerCycle);
		return fmod(getBarPhase() + barLengthsPassed, barsPerCycle);
	}
}

void MonoSynth::handleChannelMode(double * left, double * right)
{
	switch (channelMode)
	{
	case ChannelMode::Stereo:
		break;
	case ChannelMode::Mono:
		*left = *right = (*left + *right)*0.5;
		break;
	case ChannelMode::Left:
		*right = *left;
		break;
	case ChannelMode::Right:
		*left = *right;
		break;
	}
}