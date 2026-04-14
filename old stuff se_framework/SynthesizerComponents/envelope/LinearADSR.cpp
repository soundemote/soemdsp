#include "LinearADSR.h"

// round up to odd above 0 and round down to odd below 0 == ceil(1-(1-v)-floor(1-v))
// do the same for even: ceil(2-v-floor(v))
// -1 to +1 sawtooth: (v-floor(v)*2-1
void calculateOvershootAndReflect(double * v)
{
	*v = 1.0 - (*v - floor(*v));
}
void calculateOvershoot(double * v)
{
	*v -= floor(*v);
}

void FeedbackADSR::makeRandomDecision(double * v)
{
	*v = noise.getSampleUnpiolar();
}

void FeedbackADSR::setSampleRate(double v)
{
	secondsPerSample = 1/v;

	updateAttackIncrement();
	updateDecayIncrement();
	updateReleaseIncrement();

	feedbackAmtMultiplier = 44100.0/v;
	updateGlobalFeedback();
	
}

void FeedbackADSR::updateGlobalFeedback()
{
	finalFeedbackAmtMultiplier = feedbackAmt*feedbackAmtMultiplier;
	updatefbAttack();
	updatefbDecay();
	updatefbRelease();
}

void FeedbackADSR::setGlobalFeedback(double v)
{
	feedbackAmt = v;
	updateGlobalFeedback();
}

void FeedbackADSR::updatefbAttack()
{
	fbAttack = attackFBAmt + attackFBAmt * finalFeedbackAmtMultiplier;
	fbAttack = jmin(.999999, fbAttack); 
}

void FeedbackADSR::updatefbDecay()
{
	fbDecay = decayFBAmt + decayFBAmt * finalFeedbackAmtMultiplier;
	fbDecay = jmin(.999999, fbDecay);
}

void FeedbackADSR::updatefbRelease()
{
	fbRelease = releaseFBAmt + releaseFBAmt * finalFeedbackAmtMultiplier;
	fbRelease = jmin(.999999, fbRelease);
}

void FeedbackADSR::updateAttackIncrement()
{
	attackIncrement = (velocityScaling-currentValue) * secondsPerSample / (attackTime+globalTime);
}
void FeedbackADSR::updateDecayIncrement()
{
	decayIncrement = (currentValue-(sustainAmp*velocityScaling)) * secondsPerSample / (decayTime+globalTime);
	//jassert(decayIncrement >= 0);
}
void FeedbackADSR::updateReleaseIncrement()
{
	releaseIncrement = (currentValue-0) * secondsPerSample / (releaseTime+globalTime);
}

double FeedbackADSR::calculateNextVaue(double inc, Shape shape, double fbAmount)
{
	inc = jmax(1.e-8, inc);

	double out;
	switch (shape)
	{
	case Shape::LIN: default:
		out = inc;
		break;
	case Shape::EXP:
		out = inc + currentValue*fbAmount + fbAmount*r*.01 + r*.01;
		break;
	case Shape::LOG:
		out =  inc + (velocityScaling-currentValue)*fbAmount + fbAmount*r + r*.001;
		break;
	case Shape::SIGMOID:
		out =  inc + (velocityScaling-currentValue)*currentValue*fbAmount + fbAmount*r*.0001;
		break;
	case Shape::POW:
		out =  inc + currentValue*currentValue*fbAmount + fbAmount*r*.01 + r*.0001;
		break;
	}

	// level += A*level^2 + B*level + C

	//out = jmin(out, 0.5);

	return out;
}

void FeedbackADSR::reset()
{
	currentValue = 0;
	envState = OFF;
	secondsPassed = 0;
}

bool FeedbackADSR::isIdle()
{
	return envState == OFF;
}

void FeedbackADSR::trigger() {}
void FeedbackADSR::triggerDelay() {}
void FeedbackADSR::triggerAttack(double velocity, bool updateVelScale)
{
	/* skip delay stage is delay time is negligible
	and skip attack stage if attack time is
	negliglbe. */
	if (updateVelScale)
		velocityScaling = 1 - (1-velocity) * velocityInfluence;

	if (envState == OFF && delayTime >= secondsPerSample)
	{
		envState = DELAY;
		return;
	}

	if (currentValue >= velocityScaling)
	{
		updateDecayIncrement();
		envState = DECAY;
		return;
	}

	updateAttackIncrement();
	envState = ATTACK;
	return;
}
void FeedbackADSR::triggerDecay() 
{

}
void FeedbackADSR::triggerSustain() 
{

}

void FeedbackADSR::triggerRelease()
{
	envState = RELEASE;
	updateReleaseIncrement();
}

double FeedbackADSR::getSample()
{
	if (doLoop)
	{
		r = noise.getSampleBipolar()*.00001;
		updatefbAttack();
		updatefbDecay();
	}
	else
		r = 0;

	switch (envState)
	{
	case OFF:
		break;

	case DELAY:
		secondsPassed += secondsPerSample;
		if (secondsPassed >= delayTime)
		{
			secondsPassed = 0;
			envState = ATTACK;			
		}
		break;

	case ATTACK:
		currentValue += calculateNextVaue(attackIncrement, atkShape, fbAttack);
		if (currentValue >= velocityScaling)
		{
			double timeSpentDecaying = velocityScaling - 1/currentValue;
			currentValue = velocityScaling;
			updateDecayIncrement();
			currentValue -= timeSpentDecaying * calculateNextVaue(decayIncrement, decShape, fbDecay);
			envState = DECAY;
		}		
		break;

	case DECAY:
		if (currentValue < (sustainAmp*velocityScaling))
		{
			makeRandomDecision(&currentValue);
			jmap(currentValue, 0.0, 1.0, sustainAmp, 1.0);
		}
		currentValue -= calculateNextVaue(decayIncrement, decShape, fbDecay);
		// Instead of this if statement here, transition to 100% feedback 
		// where speed of transition is linear based on based on a "sustain factor"
		// such as feedback_value -= sustainFactor;
		// then when feedback_value >= 0%, go to sustain and hold current value.
		if (doLoop)
		{
			if (currentValue <= sustainAmp)
			{				
  			double timeSpentAttacking = (((sustainAmp*velocityScaling) - std::abs(decayIncrement)) / currentValue) - velocityScaling;
				calculateOvershoot(&timeSpentAttacking);
				//if (currentValue < 0 || currentValue > 1)
					//makeRandomDecision(&currentValue);
				currentValue = (sustainAmp*velocityScaling);
				updateAttackIncrement();
				currentValue += timeSpentAttacking * calculateNextVaue(attackIncrement, atkShape, fbAttack);
				envState = ATTACK;
				if (currentValue > 1)
					makeRandomDecision(&currentValue);
			}				
		}
		else if (currentValue < (sustainAmp*velocityScaling))
		{
			envState = SUSTAIN;
			currentValue = (sustainAmp*velocityScaling);
		}
		break;

	case SUSTAIN:
			currentValue = (sustainAmp*velocityScaling);
		break;

	case RELEASE:
		// Release now controls the speed transition to intitial feedback value
		// feedback_value += releaseFactr;
		// if feedback_value == intial_feedback_value, stop incrementing
		// or you can try just going to %100% feedback value
		currentValue -= calculateNextVaue(releaseIncrement, relShape, fbRelease);
		if (currentValue <= 0)
			reset();
		break;
	}

	//jassert(currentValue >= 0 && currentValue <= 1);
	if (currentValue < 0 || currentValue > 1)
		makeRandomDecision(&currentValue);
	return currentValue;
}

bool LinearADSR::isIdle() { return envState == OFF; }

void LinearADSR::setSampleRate(double v)
{
	secondsPerSample = 1/v;
	setDelayTime(delayTime);
	setAttackTime(attackTime);
	setSustainAmplitude(sustainAmp);
}

void LinearADSR::reset()
{
	currentValue = 0;
	envState = OFF;
	secondsPassed = 0;
}

void LinearADSR::trigger() {}
void LinearADSR::triggerDelay() {}
void LinearADSR::triggerAttack(double /*velocity*/, bool /*updateVelScale*/)
{
	/* skip delay stage is delay time is negligible
	* and skip attack stage if attack time is
	* negliglbe. */
	if (delayTime < secondsPerSample)
		if (attackTime <= secondsPerSample)
		{
			envState = DECAY;
			currentValue = 1.0;
		}
		else
			envState = ATTACK;
	else
		envState = DELAY;
}
void LinearADSR::triggerSustain() {}
void LinearADSR::triggerDecay() {}
void LinearADSR::triggerRelease()
{
	envState = RELEASE;
	setReleaseTime(releaseTime);
}

void LinearADSR::setDelayTime(double v)
{
	delayTime = v;
}

void LinearADSR::setAttackTime(double v)
{
	attackTime = v;
	attackIncrement = secondsPerSample / attackTime;
	attackIncrement = jmin(attackIncrement, 1.0);
}

void LinearADSR::setSustainAmplitude(double v)
{
	sustainAmp = v;
	setDecayTime(decayTime);
	setReleaseTime(releaseTime);
}

void LinearADSR::setDecayTime(double v)
{
	decayTime = v;
	decayDecrement = (1-sustainAmp) * secondsPerSample / decayTime;
}

void LinearADSR::setReleaseTime(double v)
{
	releaseTime = v;
	releaseDecrement = currentValue * secondsPerSample / releaseTime;
}

double LinearADSR::getSample()
{
	switch (envState)
	{
	case OFF:
		break;

	case DELAY:
		secondsPassed += secondsPerSample;
		if (secondsPassed >= delayTime)
			envState = ATTACK;
		break;

	case ATTACK:
		currentValue += attackIncrement;
		if (currentValue >= 1)
		{
			currentValue = 1;
			envState = DECAY;
		}
		break;

	case DECAY:
		currentValue -= decayDecrement;
		if (currentValue <= sustainAmp)
		{
			currentValue = sustainAmp;
			envState = SUSTAIN;
		}
		break;

	case SUSTAIN:
		if (isLooping)
			envState = ATTACK;
		currentValue = sustainAmp;
		break;

	case RELEASE:
		currentValue -= releaseDecrement;
		if (currentValue <= 0)
			reset();
		break;
	}

	return currentValue;
}
