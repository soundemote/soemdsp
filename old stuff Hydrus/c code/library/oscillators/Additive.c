#include "Additive.h"

void AdditiveUnit_init(AdditiveMaster* m, AdditiveUnit* o)
{
	o->increment = .001;
	o->phase = 0;
	o->phaseMod = 0;
	o->skewPhase = 0;
	o->randomPhase = 0;
	o->randomPhaseAmp = 0;
	o->amplitude = 0;
	o->delayCounter = 0;
	o->delaySamples = 0;

	o->freqMod = 0;

	o->master = m;
}

void AdditiveUnit_reset(AdditiveUnit* o)
{
    o->phase = 0;
    o->delayCounter = 0;
}

double AdditiveUnit_getSample(AdditiveUnit* o)
{
    if (o->delayCounter++ < o->delaySamples)
        return 0;
	
	if (o->frequency > 20000 || o->increment > 0.5)
		return 0;

    o->phase += o->increment;
    o->phase = wrapPhase(o->phase);

	double finalPhase = wrapPhaseExtreme(o->phase + o->phaseMod + o->randomPhase * o->master->randomPhaseAmp + o->skewPhase + o->master->globalPhaseOffset);

	double incrementAbs = abs(o->increment);
	switch(o->master->waveform)
	{
		default:
		case 0:
			return wSin(finalPhase);
		case 1:
			return PolyBLEP_square(incrementAbs, finalPhase);
		case 2:
			return PolyBLEP_pulse(incrementAbs, finalPhase, o->master->morph);
		case 3:
			return PolyBLEP_pulseCenter(incrementAbs, finalPhase, o->master->morph);
		case 4:
			return PolyBLEP_saw(incrementAbs, finalPhase);
		case 5:
			return PolyBLEP_ramp(incrementAbs, finalPhase);
		case 6:
			return PolyBLEP_tri(incrementAbs, finalPhase);
		case 7:
			return PolyBLEP_trisaw(incrementAbs, finalPhase, o->master->morph);
		case 8:
			return PolyBLEP_triSquare(incrementAbs, finalPhase, o->master->morph);
		case 9:
			return PolyBLEP_triPulse(incrementAbs, finalPhase, o->master->morph);
		case 10:
			return PolyBLEP_rectSinFull(incrementAbs, finalPhase);
	}    
}

void AdditiveMaster_init(AdditiveMaster* o)
{
	o->frequency = 0;
	o->increment = 0;
	o->sampleRate = 0;
	o->numHarmonics = 0;
	o->ampCurve = 0;
	o->signalL = 0;
	o->signalR = 0;
	o->frequencyOffset = 0;

	o->frequencyOffsetCurve = 0;
	o->frequencyOffsetAmp = 0;
	o->stereoMode = MONO;

	o->waveform = SIN;

	o->allHarmonicsSameAmp = false;

	for (int i = 0; i < ADDITIVE_MAX_HARMONICS; ++i)
		AdditiveUnit_init(o, &o->harmonics[i]);

	AdditiveFxDrift_init(&o->driftFx, o);
}

void AdditiveMaster_reset(AdditiveMaster* o)
{
	for (int i = 0; i < o->numHarmonics; ++i)        
		AdditiveUnit_reset(&o->harmonics[i]);

	AdditiveFxDrift_reset(&o->driftFx);	
}

void AdditiveMaster_setStereoMode(AdditiveMaster* o, int v)
{
	o->stereoMode = v;
	AdditiveMaster_update(o);
}

void AdditiveMaster_setSampleRate(AdditiveMaster* o)
{
	AdditiveFxDrift_setSampleRate(&o->driftFx);	
	AdditiveMaster_update(o);
}

void AdditiveMaster_setNumHarmonics(AdditiveMaster* o, int v)
{
	o->numHarmonics = v;
	AdditiveFxDrift_setNumHarmonics(&o->driftFx, o->numHarmonics);
	AdditiveMaster_update(o);
}

void AdditiveMaster_update(AdditiveMaster* o)
{
	double pan = 0;
	double panInc = 0;

	switch(o->stereoMode)
	{
	case PSEUDOSTEREO:
		o->harmonics[0].pan = 0.5;
		
		panInc = 1.0/(double)o->numHarmonics;

		for (int i = 1; i < o->numHarmonics; ++i)
		{
			switch(i%2)
			{
			case 0:
				pan += panInc;		
				o->harmonics[i].pan = 0.5 + pan;
				break;
			case 1:
				o->harmonics[i].pan = 0.5 - pan;
				break;
			}
		}
		break;
	default:
		for (int i = 0; i < o->numHarmonics; ++i)
			o->harmonics[i].pan = 0.5;
		break;
	}
}

double BiasFunction(double harmonic, int totalHarmonics, double bias)
{
	double th = (double) totalHarmonics;
	double f = ((th + 1.0) - harmonic) / th;
	return f / (((( 1.0 / bias ) - 2.0) * (1.0 - f)) + 1.0);
}

double AdditiveMaster_getSample(AdditiveMaster* o)
{
	AdditiveFxDrift_apply(&o->driftFx, o);
	double taperAdjust;
	double waveshapeAdjust = 1;
	double signal = 0;
	o->signalL = 0;
	o->signalR = 0;

	for (int i = 0; i < o->numHarmonics; ++i)
	{
		waveshapeAdjust = o->allHarmonicsSameAmp ? 1.0 : 1.0/(double)(i+1.0);

		signal = AdditiveUnit_getSample(&o->harmonics[i]) * waveshapeAdjust * (BiasFunction(i+1.000001, o->numHarmonics, o->ampCurve));
		o->signalL += signal * o->harmonics[i].pan;
		o->signalR += signal * (1 - o->harmonics[i].pan);
	}

	return o->signalL + o->signalR;
}

void AdditiveFxDrift_init(AdditiveFxDrift* o, AdditiveMaster* master)
{
	o->driftDetail = 0;
	o->driftColor = 0;
	o->driftCurve = 0;
	o->driftAmplitude = 0;
	o->frequency = 1;
	o->numHarmonics = 0;
	o->master = master;

	for (int i = 0; i < o->numHarmonics; ++i)
		RandomWalk_init(&o->drifts[i], MASTER_SEED + i);
}

void AdditiveFxDrift_reset(AdditiveFxDrift* o)
{
	for (int i = 0; i < o->numHarmonics; ++i)
		RandomWalk_reset(&o->drifts[i]);
}

void AdditiveFxDrift_setSampleRate(AdditiveFxDrift* o)
{
	for (int i = 0; i < o->numHarmonics; ++i)
		RandomWalk_setSampleRate(&o->drifts[i]);
}

void AdditiveFxDrift_setNumHarmonics(AdditiveFxDrift* o, int v)
{
	o->numHarmonics = v;
	for (int i = 0; i < o->numHarmonics; ++i)
		RandomWalk_setColor(&o->drifts[i], o->driftColor);

	AdditiveFxDrift_update(o);
}

void AdditiveFxDrift_setDriftDetail(AdditiveFxDrift* o, double v)
{
	o->driftDetail = v;
	AdditiveFxDrift_update(o);
}

void AdditiveFxDrift_setDriftColor(AdditiveFxDrift* o, double v)
{
	o->driftColor = v;
	AdditiveFxDrift_update(o);
}

void AdditiveFxDrift_setDriftCurve(AdditiveFxDrift* o, double v)
{
	o->driftCurve = v;
}

void AdditiveFxDrift_setDriftAmplitude(AdditiveFxDrift* o, double v)
{
	o->driftAmplitude = v;
}

void AdditiveFxDrift_update(AdditiveFxDrift* o)
{	

	for (int i = 0; i < o->numHarmonics; ++i)
	{
		RandomWalk_setDetail(&o->drifts[i], o->master->harmonics[i].frequency * o->driftDetail);
		RandomWalk_setColor(&o->drifts[i], o->driftColor);
	}
}

void AdditiveFxDrift_apply(AdditiveFxDrift* o, AdditiveMaster* m)
{
	int halfHarmonics = o->numHarmonics * 0.5;

	for (int i = 0; i < o->numHarmonics; ++i)
	{
		if (o->master->harmonics[i].frequency > 20000)
			return;

		double value0to1 = (double)i+1/(double)o->numHarmonics;

		double amplitudeWithCurve = rationalCurve(value0to1, o->driftCurve);

		o->master->harmonics[i].phaseMod = RandomWalk_getSample(&o->drifts[i]) * amplitudeWithCurve * o->driftAmplitude;
	}
}