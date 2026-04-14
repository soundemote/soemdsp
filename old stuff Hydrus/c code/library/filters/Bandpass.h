/*
One pole bandpass filter created with simple lowpass and highpass filters with controls for frequency and width.
*/

typedef struct 
{
	double frequency;
	double halfWidth;
	double output;

	Lowpass lowpass;
	Highpass highpass;
} Bandpass;

void Bandpass_update(Bandpass* o)
{
	Lowpass_setFrequency(&o->lowpass, max(o->frequency + o->halfWidth, 0));
	Highpass_setFrequency(&o->highpass, max(o->frequency - o->halfWidth, 0));
}

void Bandpass_init(Bandpass* o)
{
	o->frequency = 0;
	o->halfWidth = 0;
	o->output = 0;

	Lowpass_init(&o->lowpass);
	Highpass_init(&o->highpass);
}

void Bandpass_reset(Bandpass* o)
{
	Lowpass_reset(&o->lowpass);
	Highpass_reset(&o->highpass);
}

void Bandpass_setSampleRate(Bandpass* o)
{
	Lowpass_setSampleRate(&o->lowpass);
	Highpass_setSampleRate(&o->highpass);
}

// total frequency range
void Bandpass_setWidth(Bandpass* o, double v)
{
	o->halfWidth = v * 0.5;
}

void Bandpass_setFrequency(Bandpass* o, double v)
{
	o->frequency = v;
	Bandpass_update(o);
}

double Bandpass_getSample(Bandpass* o, double v)
{	
	Highpass_getSample(&o->highpass, Lowpass_getSample(&o->lowpass, v));
}