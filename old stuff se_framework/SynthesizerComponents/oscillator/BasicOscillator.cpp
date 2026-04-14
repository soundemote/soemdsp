#include "BasicOscillator.h"

double BasicOscillator::getSample()
{
	double phase = phasor.getUnipolarValue();

	switch (waveform)
	{
	case WaveformMode::NONE:
		return clamp(unipolarToBipolar(phaseOffset) + noise.getSampleBipolar()*noiseAmp, -1.0, 1.0);
	case WaveformMode::NOISE_ONLY:
		return clamp(current_value = noise.getSampleBipolar(), -1.0, 1.0);
	case WaveformMode::RAND_SQUARE:
		current_value = clamp(do_RAND_SQUARE_stuff(), -1.0, 1.0);
		break;
	case WaveformMode::RAND_TRI:
		current_value = clamp(do_RAND_TRI_stuff(), -1.0, 1.0);
		break;
	case WaveformMode::RAND_SAW:
		current_value = clamp(do_RAND_SAW_stuff(), -1.0, 1.0);
		break;
	case WaveformMode::RAND_SLOPE_BI:
		current_value = clamp(do_RAND_SLOPE_stuff(), -1.0, 1.0);
		break;
	case WaveformMode::RAND_SLOPE_UNI:
		current_value = clamp(do_RAND_SLOPE_UNI_stuff(), -1.0, 1.0);
		break;
	case WaveformMode::SINE:
		current_value = waveshape::sine(phase);
		do_RAND_SQUARE_stuff();
		break;
	case PHASOR:
	case SAW_UP_NOAA:
		current_value = waveshape::ramp(phase);
		do_RAND_SQUARE_stuff();
		break;
	case SAW_DOWN_NOAA:
		current_value = waveshape::saw(phase);
		do_RAND_SQUARE_stuff();
		break;
	case TRI_NOAA:
		current_value = waveshape::tri(phase);
		do_RAND_SQUARE_stuff();
		break;
	case ELLIPSE:
		current_value = waveshape::ellipse(phase, A, B_sin, B_cos, C);
		do_RAND_SQUARE_stuff();
		break;
	case WaveformMode::TRI:
	case WaveformMode::SAW:
	case WaveformMode::SQUARE:
		current_value = polyBLEP.getSample();
		do_RAND_SQUARE_stuff();
		break;
	}
	
	if (noiseAmp)
	{
		current_freq = frequency + frequency*current_rand_val * noiseAmp;
		phasor.setFrequency(current_freq);
		polyBLEP.setFrequency(current_freq);
		updateTimerInc(current_freq);
	}
	else if (frequencyNeedsUpdating)
	{
		phasor.setFrequency(frequency);
		polyBLEP.setFrequency(frequency);
		frequencyNeedsUpdating = false;
	}

	phasor.increment();

	return current_value;
}

void BasicOscillator::triggerEvent()
{
	timer = timer - 1; // calculate overshoot
	prev_rand_val = current_rand_val;
	current_rand_val = noise.getSampleBipolar();
	rand_val_2 = noise.getSampleBipolar() * 2.0;
}

double BasicOscillator::do_RAND_SQUARE_stuff()
{
	incrememt();
	if (timer >= 1 + phaseOffset)
		triggerEvent();
	return current_rand_val;
}

double BasicOscillator::do_RAND_SAW_stuff()
{
	incrememt();
	if (timer >= 1 + phaseOffset)
		triggerEvent();
	return jmap(timer, 0.0, 1.0, rand_val_2, current_rand_val*2);
}

double BasicOscillator::do_RAND_SLOPE_stuff()
{
	incrememt();
	if (timer >= 1 + phaseOffset)
		triggerEvent();
	return jmap(timer, 0.0, 1.0, (current_rand_val > 0.0 ? -1.0 : 1.0), current_rand_val*2);
}

double BasicOscillator::do_RAND_SLOPE_UNI_stuff()
{
	incrememt();
	if (timer >= 1 + phaseOffset)
		triggerEvent();
	return jmap(timer, 0.0, 1.0, 0.0, current_rand_val*2);
}

void BasicOscillator::triggerTriEvent()
{
	tri_inc = jmin((current_rand_val - prev_rand_val) * sampleRateInv * frequency, 0.5);
}

double BasicOscillator::do_RAND_TRI_stuff()
{
	incrememt();
	if (timer >= 1 + phaseOffset)
	{
		triggerEvent();
		//triggerTriEvent();
	}

	//current_tri_val += tri_inc;

	return jmap(timer, 0.0, 1.0, prev_rand_val*2, current_rand_val*2);
}
