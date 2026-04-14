#include "LinearAttack.h"

bool LinearAttack::isIdle() { return envState == EnvState::OFF; }

void LinearAttack::setSampleRate(double v)
{
	secondsPerSample = 1/(sampleRate = v);
	setDelaySeconds(delayInSeconds);
	setAttackSeconds(attackInSeconds);
}

void LinearAttack::reset()
{
	currentValue = 0;
	envState = EnvState::OFF;
	secondsPassed = 0;
}

/*********************/
/* TRIGGER FUNCTIONS */
/*********************/
void LinearAttack::trigger()
{
	if (delayInSeconds > secondsPerSample)
		triggerDelay();
	else
		triggerAttack();
}

void LinearAttack::triggerDelay()
{
	envState = EnvState::DELAY;
	targetValue = 0.0;
	secondsPassed = 0;
}

void LinearAttack::triggerAttack()
{
	// do not reset seconds passed if env is re-attacking
	if (envState != EnvState::ATTACK)
		secondsPassed = 0;

	envState = EnvState::ATTACK;
	targetValue = 1.0;
}

/*****************/
/* SET FUNCTIONS */
/*****************/
void LinearAttack::setDelaySeconds(double v)
{
	delayInSeconds = v;
}

void LinearAttack::setAttackSeconds(double v)
{
	attackInSeconds = v;
	attackIncrement = (1-currentValue) * secondsPerSample * 1/attackInSeconds;
}

double LinearAttack::getSample()
{
	switch (timemode)
	{
	case TimeMode::TIMED:
		doTimedMode();
		break;
	case TimeMode::UNTIMED:
		doUntimedMode();
		break;
	}

	secondsPassed += secondsPerSample;

	/* clipping needed due to rounding errors. */
	return currentValue = clamp(currentValue, 0.0, 1.0);
}

void LinearAttack::doTimedMode()
{
	switch (envState)
	{
	case EnvState::DELAY:
		if (secondsPassed >= delayInSeconds)
			triggerAttack();
		break;

	case EnvState::ATTACK:
		currentValue += attackIncrement;
		if (secondsPassed >= attackInSeconds)
			switch (loopmode)
			{
			case EnvState::OFF:
				reset();
			case EnvState::DELAY:
				trigger();
			case EnvState::ATTACK:
				triggerAttack();
			}
		break;
	}
}

void LinearAttack::doUntimedMode()
{
	switch (envState)
	{
	case EnvState::DELAY:
		if (secondsPassed >= delayInSeconds)
			triggerAttack();
		break;

	case EnvState::ATTACK:
		currentValue += attackIncrement;
		if (currentValue >= targetValue)
			switch (loopmode)
			{
			case EnvState::OFF:
				reset();
			case EnvState::DELAY:
				trigger();
			case EnvState::ATTACK:
				triggerAttack();
			}
		break;
	}
}