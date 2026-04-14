/*
Extended DSF Algorithms by Walter H. Hackett IV
- Engineer's Saw to Sine

Antialiased oscillator that is able to morph from sine to square
with a smooth harmonic falloff.
*/

typedef struct SineSquare
{
	variable v[OSC_NUMVARS];
	
	double output;
	double lastOutput;	

	double leak;
	double n; // number of partials
	double k;
	double k2;
	double k42;
	double x;
} SineSquare;

void SineSquare_init(SineSquare* o)
{
	for (int i = 0; i < OSC_NUMVARS; ++i)
		variable_init(&o->v[i]);

	o->output = 0;

	o->output = 0;
	o->leak = 1.0;
	o->lastOutput = 0;
	o->n = 0;
	o->k = 0;
	o->x = 0;
}

void SineSquare_reset(SineSquare* o)
{
	o->v[OSC_phase].vw = *o->v[OSC_phase].vmr;

	o->leak = 1;
}

void SineSquare_incrementChanged(SineSquare* o, double maxPartials)
{
	//TODO: THERE'S LIKE NO FREQUENCY MODULATION TAKEN INTO ACCOUNT YO... LIKE... WTF? COME ON YO YO MAN YO. LIKE GET WITH IT DOOD.
    //double maxPartials = 0.5 * (*o->v[OSC_frequency].vr == 0.0 ? 0.0 : halfSampleRate / *o->v[OSC_frequency].vr);
	//double maxPartials = 0.5 * sampleRate / (*o->v[OSC_increment].vr * 2 * sampleRate);

    o->n = max(floor(maxPartials), 1);
}

// 0 to 1, 0 being sine, 1 being square
void SineSquare_morphChanged(SineSquare* o)
{
    o->k = 1 - (1.0 / (pow((variable_getWithMod(&o->v[OSC_morph]) / 2.0 + 0.25), 14) * 10000 + 1)) + .000000000001;
}

double SineSquare_DSF(SineSquare* o)
{
    double pow_k_n_p_1 = pow(o->k, o->n + 1);
    return 8.0*((pow_k_n_p_1*o->k*cos(o->x*(2*o->n-1))-pow_k_n_p_1*cos(o->x*(2*o->n+1))-o->k*cos(o->x)*(o->k-1))/o->k/(1+o->k*o->k-2*o->k*cos(2*o->x)));
}

double SineSquare_getSample(SineSquare* o)
{        
    o->v[OSC_phase].vw += variable_getWithMod(&o->v[OSC_increment]) * 0.9999; // added 0.9999 to help prevent errors
    o->v[OSC_phase].vw = wrapPhase(variable_getWithMod(&o->v[OSC_phase]));

    o->x = wrapPhase(variable_getWithMod(&o->v[OSC_phase])) * TAU;

    o->leak = o->leak * 0.99 + 0.000005;
    o->output = o->output * (1.0 - o->leak) + SineSquare_DSF(o) * variable_getWithMod(&o->v[OSC_increment]);
    
    o->lastOutput = o->output;

    return o->output;
}

