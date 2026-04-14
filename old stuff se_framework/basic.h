#pragma once

#include "JuceHeader.h"

#include <map>
#include <set>
#include <random>
#include <vector>
#include <array>

using jura::Parameter;
using jura::ModulatableParameter2;
using jura::ModulationConnection;
using jura::MetaControlledParameter;

typedef RAPT::rsEllipticSubBandFilter<double, double> EllipticSubBandFilter;
typedef RAPT::rsOnePoleFilter<double, double> OnePoleFilter;
typedef rosic::rsOnePoleFilterStereo OnePoleFilterStereo;
typedef RAPT::rsLadderFilter<double, double> LadderFilter;
typedef RAPT::rsStateVariableFilter<double, double> StateVariableFilter;
typedef rosic::BreakpointModulator BreakpointModulator;
typedef RAPT::rsNormalizedSigmoids<double> NormalizedSigmoids;
typedef RAPT::rsScaledAndShiftedSigmoid<double> ScaledAndShiftedSigmoid;
typedef RAPT::rsPositiveBellFunctions<double> PositiveBellFunctions;
typedef RAPT::rsParametricBellFunction<double> ParametricBellFunction;

class TriggerReceiver
{
public:
	TriggerReceiver() = default;
	virtual ~TriggerReceiver() = default;

	virtual void triggerWasReceived() = 0;
};

class TriggerSender
{
public:

	void addReciever(TriggerReceiver * receiver)
	{
		receivers.addIfNotAlreadyThere(receiver);
	}

	void removeReciever(TriggerReceiver * receiver)
	{
		receivers.addIfNotAlreadyThere(receiver);
	}

	void notifyReceivers()
	{
		for (const auto & r : receivers)
			r->triggerWasReceived();
	}

private:
	Array<TriggerReceiver*> receivers;
};

/* STRING CONVERSION */
extern const std::vector<juce::String> BeatMultiplierStr;
extern const std::vector<juce::String> TempoOptionsStr;
struct signature { juce::String name; double value; };
extern const std::vector<signature> timeSignatures;


namespace elan {
juce::String valueToStringWithPlusMinusSign(double v, int decimalplaces = 0);
juce::String secondsToStringWithUnitTotal4(double v);

juce::String StringFunc0WithX(double v);
juce::String StringFunc0WithCapitolX(double v);
juce::String StringFunc3WithX(double v);
juce::String StringFunc0(double v);
juce::String StringFunc1(double v);
juce::String StringFunc2(double v);
juce::String StringFunc3(double v);
juce::String StringFunc4(double v);
juce::String StringFunc5(double v);
juce::String percentToStringWith2Decimals(double v);
juce::String percentToStringWith3Decimals(double v);
juce::String hertzToStringWithUnitTotal5(double v);
juce::String hertzToStringWithUnitTotal5_bipolar(double v);
juce::String hertzToStringWithUnitTotal4(double v);
juce::String hertzToStringWithUnitTotal4_bipolar(double v);
juce::String hertzToStringWithUnitTotal3(double v);
juce::String hertzToStringWithUnitTotal3_bipolar(double v);
juce::String hertzToStringLowpassFilter(double v);
juce::String hertzToStringHighpassFilter(double v);
juce::String secondsToStringWithUnitTotal4(double v);
juce::String semitonesToStringWithUnit2(double v);
juce::String valueToPercentStringWithUnit0(double v);
juce::String decibelsToStringWithUnit1(double v);
juce::String decibelsToStringWithUnit2(double v);
juce::String octavesToStringWithUnit0(double v);
juce::String beatMulToString(double v);
juce::String tempoMulToString(double v);
juce::String bipolarRateToString(double v);
juce::String numShroomsToString(double v);
juce::String beatsPerSecondToString(double v);
juce::String dotLimitToString(double v);
juce::String afterGlowToString(double v);
juce::String indexToTimeSigName(double v);
double indexToTimeSigValue(double v);
} // namespace elan
