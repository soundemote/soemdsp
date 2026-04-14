#pragma once

class LinearAttack
{
public:
	bool isIdle();

	void setSampleRate(double v);

	void reset();

	/* Use the trigger function to trigger skip delay if
	* delay time is negligible. You may also simply trigger
	* delay or attack and skip the middleman. */
	void trigger();
	void triggerDelay();
	void triggerAttack();

	double getSample();

	// 1.e-6 to inf exponential
	void setDelaySeconds(double v);

	// 1.e-6 to inf exponential
	void setAttackSeconds(double v);

	// use envstate enum to set point of loop
	enum EnvState { OFF, DELAY, ATTACK };
	void setLoopMode(EnvState v) { loopmode = v; }

	enum TimeMode { TIMED, UNTIMED };
	void setTimedMode(TimeMode v) { timemode = v; }

protected:
	/* Core Functions */
	void doTimedMode();
	void doUntimedMode();

	double /* Parameters */
		delayInSeconds = 0,
		attackInSeconds = 1,
		decayInSeconds = 1;

	bool isLooping = false;

	double /* Internal Values*/
		sampleRate = 44100,
		secondsPerSample = 0,
		attackIncrement = 0,
		targetValue = 0;

	double /* Counters */
		currentValue = 0,
		outputValue = 0,
		secondsPassed = 0;

	int loopmode = EnvState::ATTACK;

	int envState = EnvState::OFF;

	int timemode = TimeMode::TIMED;
};
