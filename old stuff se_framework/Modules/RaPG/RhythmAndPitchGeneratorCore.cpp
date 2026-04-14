#include "RhythmAndPitchGeneratorCore.h"

RaPG_ClockDivider::RaPG_ClockDivider(RaPG_RhythmGenerator * masterRhthmGenerator)
	: master(masterRhthmGenerator)
{
	ClockDivider::setTriggerFunction([this]()
	{
		master->RunningClockArray.addIfNotAlreadyThere(this);
		pulseTimer1.restart();
		pulseTimer2.restart();
	});

	pulseTimer1.setTriggerFunction([this]()
	{
		if (isEnabled)
			master->clockWasTriggered(this);
	});

	pulseTimer2.setTriggerFunction([this]()
	{
		master->RunningClockArray.removeFirstMatchingValue(this);
		if (isEnabled)
			master->clockWasTriggered(this);
	});
}

void RaPG_ClockDivider::run()
{
	pulseTimer1.incrememt();
	pulseTimer2.incrememt();
}

void RaPG_ClockDivider::triggerBPMChange()
{
	bpm = midiMasterPtr->getBPM();
	recalculateTriggerFrequency();
}

void RaPG_ClockDivider::setSampleRate(double v)
{
	pulseTimer1.setSampleRate(v);
	pulseTimer2.setSampleRate(v);
	recalculateTriggerFrequency();
}

void RaPG_ClockDivider::setPrimaryPulseDelayNormalized(double v)
{
	primaryPulseDelayNormalized = v;
	recalculatePulseFrequency(1);
}

void RaPG_ClockDivider::setSecondaryPulseDelayNormalized(double v)
{
	secondaryPulseDelayNormalized = v;
	recalculatePulseFrequency(2);
}

void RaPG_ClockDivider::recalculateTriggerFrequency()
{
	triggerFrequency = masterTimerFrequency / (double)master->getClockDivision() / (double)ClockDivider::division;

	recalculatePulseFrequency(1);
	recalculatePulseFrequency(2);
}

void RaPG_ClockDivider::recalculatePulseFrequency(int index)
{
	switch (index)
	{
	case 1:
		pulseTimer1.setSeconds(1/triggerFrequency * primaryPulseDelayNormalized);
		return;
	case 2:
		pulseTimer2.setSeconds(1/triggerFrequency * secondaryPulseDelayNormalized);
		return;
	}
}

void RaPG_ClockDivider::setIsEnabled(bool v)
{
	isEnabled = v;
	if (isEnabled)
		master->EnabledClockArray.addIfNotAlreadyThere(this);
	else
		master->EnabledClockArray.removeFirstMatchingValue(this);
}

RaPG_RhythmGenerator::RaPG_RhythmGenerator()
{
	masterClockDivider.setTriggerFunction([this]()
	{
		if (steps > 0)
		{
			currentStep = currentStep % steps;

			if (currentStep == 0)
				noise.reset();
		}

		numClocksEnabled = EnabledClockArray.size();

		noiseValue = noise.getSampleUnpiolar();
		double noiseWithEnabled = (isEnabled ? noiseValue : 0);

		if (numClocksEnabled > 0)
		{
			enabledClockIndex = wrapIndexBipolarSingleOctave(int(noiseWithEnabled * numClocksEnabled + offsetValue), numClocksEnabled);
			enabledGhostClockIndex = wrapIndexUnipolarSingleOctave(enabledClockIndex + 1, numClocksEnabled);

			currentClock = EnabledClockArray[enabledClockIndex];
			currentGhostClock = EnabledClockArray[enabledGhostClockIndex];
		}

		for (auto & clock : AllClocksArray)
			clock->trigger();

		++currentStep;

		clockIndexValueForOutput = currentClock->id / 6.0;
	});

	for (int i = 0; i < AllClocksArray.size(); ++i)
	{
		AllClocksArray[i]->master = this;
		AllClocksArray[i]->id = i;
	}
}

void RaPG_RhythmGenerator::run()
{
	if (RunningClockArray.isEmpty())
		return;

	for (auto & clock : RunningClockArray)
		clock->run();
}

void RaPG_RhythmGenerator::trigger()
{
	masterClockDivider.trigger();
}

void RaPG_RhythmGenerator::clockWasTriggered(RaPG_ClockDivider * clock)
{
	if (currentClock == clock)
		triggerFunction();
	else if (enableGhostClicks && currentGhostClock == clock)
		ghostTriggerFunction();
}
