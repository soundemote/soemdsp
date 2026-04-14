typedef struct PolyBLEP
{
	variable v[OSC_NUMVARS];

	double output;
	
} PolyBLEP;

PolyBLEP_DBG(PolyBLEP* o)
{
	LOGF(*o->v[OSC_amplitude].dr)
	LOGF(*o->v[OSC_frequency].dr)
	LOGF(*o->v[OSC_increment].dr)
	LOGF(*o->v[OSC_morph].dr)
	LOGF(*o->v[OSC_phase].dr)
	LOGF(*o->v[OSC_waveform].dr)
	LogLn("");
}

void PolyBLEP_init(PolyBLEP* o)
{
	for (int i = 0; i < OSC_NUMVARS; ++i)
		variable_init(&o->v[i]);

	o->v[OSC_waveform].dw = SIN;
	o->v[OSC_amplitude].dw = 1.0;
	o->v[OSC_morph].dw = 0.5;
}

void PolyBLEP_reset(PolyBLEP* o)
{
	o->v[OSC_phase].dw = wrapPhaseExtreme(*o->v[OSC_phase].dmr);
}

double blep(double t, double dt)
{
	if (t < dt)
	{
		return -squared(t / dt - 1);
	}
	else if (t > 1 - dt)
	{
		return squared((t - 1) / dt + 1);
	}
	else
	{
		return 0;
	}
}

double blamp(double t, double dt)
{
	if (t < dt)
	{
		t = t / dt - 1;
		return -_1z3 * pow(t, 3);
	}
	else if (t > 1 - dt)
	{
		t = (t - 1) / dt + 1;
		return _1z3 * pow(t, 3);
	}
	else
	{
		return 0;
	}
}

double PolyBLEP_tri(double incrementAbs, double t)
{
	double t1 = t + 0.25;
	t1 = wrapPhase(t1);
	double t2 = t + 0.75;
	t2 = wrapPhase(t2);

	double y = t * 4;

	if (y >= 3)
	{
		y -= 4;
	}
	else if (y > 1)
	{
		y = 2 - y;
	}

	return y + 4 * incrementAbs * (blamp(t1, incrementAbs) - blamp(t2, incrementAbs));
}

double PolyBLEP_ramp(double incrementAbs, double t)
{
	double t1 = t + 0.5;
	t1 = wrapPhase(t1);

	double y = t1 * 2 - 1;

	y -= blep(t1, incrementAbs);

	return y;
}

double PolyBLEP_saw(double incrementAbs, double t)
{
	double t1 = t;
	t1 = wrapPhase(t1);

	double y = 1 - 2 * t1;

	y += blep(t1, incrementAbs);

	return y;
}

double PolyBLEP_square(double incrementAbs, double t)
{
	double t1 = t + 0.5;
	t1 = wrapPhase(t1);

	double y = t < 0.5 ? 1 : -1;

	y += blep(t, incrementAbs) - blep(t1, incrementAbs);

	return y;
}

double PolyBLEP_rectSinHalf(double incrementAbs, double t)
{
	double t1 = t + 0.5;
	t1 = wrapPhase(t1);

	double y = (t < 0.5 ? 2 * sin(TAU * t) - _2_z_PI : -_2_z_PI);

	y += TAU * incrementAbs * (blamp(t, incrementAbs) + blamp(t1, incrementAbs));

	return y;
}

double PolyBLEP_rectSinFull(double incrementAbs, double t)
{
	double t1 = t + 0.25;
	t1 = wrapPhase(t1);

	double y = 2 * sin(PI * t1) - _4_z_PI;

	y += TAU * incrementAbs * blamp(t1, incrementAbs);

	return y;
}

double PolyBLEP_trisaw(double incrementAbs, double t, double morph) 
{
	double pw = clamp(morph, 0.0001, 0.9999 );

	double t1 =  t + 0.5 * pw;
	t1 = wrapPhase(t1);
	double t2 = t + 1 - 0.5 * pw;
	t2 = wrapPhase(t2);

	double y = t * 2;

	if (y >= 2 - pw) {
		y = (y - 2) / pw;
	} else if (y >= pw) {
		y = 1 - (y - pw) / (1 - pw);
	} else {
		y /= pw;
	}

	y += incrementAbs / (pw - pw * pw) * (blamp(t1, incrementAbs) - blamp(t2, incrementAbs));

	return y;
}

double PolyBLEP_triPulse(double incrementAbs, double t, double morph)
{
	double t1 = t + 0.75 + 0.5 * morph;
	t1 = wrapPhase(t1);

	double y;
	if (t1 >= morph)
	{
		y = -morph;
	}
	else
	{
		y = 4 * t1;
		y = (y >= 2 * morph ? 4 - y / morph - morph : y / morph - morph);
	}

	if (morph > 0)
	{
		double t2 =  t1 + 1 - 0.5 * morph;
		t2 = wrapPhase(t2);
		double t3 = t1 + 1 - morph;
		t3 = wrapPhase(t3);
		y += 2 * incrementAbs / morph * (blamp(t1, incrementAbs) - 2 * blamp(t2, incrementAbs) + blamp(t3, incrementAbs));
	}

	return y;
}

double PolyBLEP_trapezoid(double incrementAbs, double t) 
{
	double y = 4 * t;

	if (y >= 3)
		y -= 4;
	else if (y > 1)
		y = 2 - y;
	
	y = max(-1, min(1, 2 * y));

	double t1 = t + 0.125;
		t1 = wrapPhase(t1);
	double t2 = t1 + 0.5;
		t2 = wrapPhase(t2);

	// Triangle #1
	y += 4 * incrementAbs * (blamp(t1, incrementAbs) - blamp(t2, incrementAbs));

	t1 = t + 0.375;
		t1 = wrapPhase(t1);
	t2 = t1 + 0.5;
		t2 = wrapPhase(t2);

	// Triangle #2
	y += 4 * incrementAbs * (blamp(t1, incrementAbs) - blamp(t2, incrementAbs));

		return y;
}

double PolyBLEP_triSquare(double incrementAbs, double t, double morph)
{
	double pw = min(0.9999, morph);
	double scale = 1 / (1 - pw);

	double y = 4 * t;
	if (y >= 3) {
		y -= 4;
	} else if (y > 1) {
		y = 2 - y;
	}
	y = max(-1, min(1, scale * y));

	double t1 = t + 0.25 - 0.25 * pw;
		t1 = wrapPhase(t1);
	double t2 = t1 + 0.5;
		t2 = wrapPhase(t2);

	// Triangle #1
	y += scale * 2 * incrementAbs * (blamp(t1, incrementAbs) - blamp(t2, incrementAbs));

	t1 = t + 0.25 + 0.25 * pw;
		t1 = wrapPhase(t1);
	t2 = t1 + 0.5;
		t2 = wrapPhase(t2);

	// Triangle #2
	return y + scale * 2 * incrementAbs * (blamp(t1, incrementAbs) - blamp(t2, incrementAbs));
}

double PolyBLEP_pulseCenter(double incrementAbs, double t, double morph) 
{
	double t1 = t + 0.875 + 0.25 * (morph - 0.5);
		t1 = wrapPhase(t1);

	double t2 = t + 0.375 + 0.25 * (morph - 0.5);
	t2 = wrapPhase(t2);

	// Square #1
	double y = t1 < 0.5 ? 1 : -1;

	y += blep(t1, incrementAbs) - blep(t2, incrementAbs);

	t1 += 0.5 * (1 - morph);
	t1 = wrapPhase(t1);

	t2 += 0.5 * (1 - morph);
	t2 = wrapPhase(t2);

	// Square #2
	y += t1 < 0.5 ? 1 : -1;

	y += blep(t1, incrementAbs) - blep(t2, incrementAbs);

	return 0.5 * y;
}

double PolyBLEP_pulse(double incrementAbs, double t, double morph) 
{
	double t1 = t + 1 - morph;
	t1 = wrapPhase(t1);

	double y = -2 * morph;

	if (t < morph)
		y += 2;

	y += blep(t, incrementAbs) - blep(t1, incrementAbs);

	return y;
}

double PolyBLEP_get(enum WAVEFORM waveform, double incrementAbs, double phase, double morph)
{
	switch (waveform)
	{
	default:
	case SIN:
		return sin(phase * TAU);
	case COS:
		return cos(phase * TAU);
	case SQUARE:
		return PolyBLEP_square(incrementAbs, phase);
	case PULSE:
		return PolyBLEP_pulse(incrementAbs, phase, morph);
	case PULSE_CENTER:
		return PolyBLEP_pulseCenter(incrementAbs, phase, morph);
	case SAW:
		return PolyBLEP_saw(incrementAbs, phase);
	case RAMP:
		return PolyBLEP_ramp(incrementAbs, phase);
	case TRI:
		return PolyBLEP_tri(incrementAbs, phase);
	case TRISAW:
		return PolyBLEP_trisaw(incrementAbs, phase, morph);
	case TRISQUARE:
		return PolyBLEP_triSquare(incrementAbs, phase, morph);
	case TRIPULSE:
		return PolyBLEP_triPulse(incrementAbs, phase, morph);
	case RECTIFIED_SIN_HALF:
		return PolyBLEP_rectSinHalf(incrementAbs, phase);
	case RECTIFIED_SIN_FULL: 
		return PolyBLEP_rectSinFull(incrementAbs, phase);
	case TRAPEZOID:
		return PolyBLEP_trapezoid(incrementAbs, phase);
	}
}

double PolyBLEP_getSample(PolyBLEP* o)
{
	double p = wrapPhaseExtreme(*o->v[OSC_phase].dr + *o->v[OSC_phase].dmr);
	double i = *o->v[OSC_increment].dr + *o->v[OSC_increment].dmr;
	double m = *o->v[OSC_morph].dr + *o->v[OSC_morph].dmr;

	o->output = PolyBLEP_get((int)*o->v[OSC_waveform].dr, i, p, m);

	o->v[OSC_phase].dw += i;

	return o->output * *o->v[OSC_amplitude].dr;
}