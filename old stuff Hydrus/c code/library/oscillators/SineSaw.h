/*
Extended DSF Algorithms by Walter H. Hackett IV
- Engineer's Saw to Sine

Antialiased oscillator that is able to morph from sine to square
with a smooth harmonic falloff.
*/

typedef struct {
    variable v[OSC_NUMVARS];

    double leak;
    double output;
    double lastOutput;
    double n; // number of partials
    double k_;
    double k2;
    double k42;
    double x;

} SineSaw;

void SineSaw_init(SineSaw* o) {
    for (int i = 0; i < OSC_NUMVARS; ++i)
        variable_init(&o->v[i]);

    o->output     = 0;
    o->leak       = 1.0;
    o->lastOutput = 0;
    o->n          = 0; // number of partials
    o->k_          = 0;
    o->k2         = 0;
    o->k42        = 0;
    o->x          = 0;
}

// SineSaw_DBG(SineSaw* o)
//{
//	LOGF(*o->v[OSC_frequency].vr);
//	LOGF(*o->v[OSC_increment].vr);
//	LOGF(*o->v[OSC_morph].vr);
//	LOGF(*o->v[OSC_phase].vr);
//
//	LOGF(*o->v[OSC_frequency].vmr);
//	LOGF(*o->v[OSC_increment].vmr);
//	LOGF(*o->v[OSC_morph].vmr);
//	LOGF(*o->v[OSC_phase].vmr);
//
//	LOGF(o->n)
//	LOGF(o->k);
//	LOGF(o->k2);
//	LOGF(o->k42);
//	LOGF(o->x);
//
//	LOGF(o->lastOutput)
//	LOGF(o->output)
//
//	LogLn("");
// }

void SineSaw_reset(SineSaw* o) {
    o->v[OSC_phase].vw = 0;
    o->leak            = 1;
}

void SineSaw_incrementChanged(SineSaw* o) {
    double f           = *o->v[OSC_frequency].vr + *o->v[OSC_frequency].vmr;
    double maxPartials = f == 0.0 ? 1.0 : halfsamplerate / f;
    o->n               = floor(maxPartials);
}

void SineSaw_updateMorph(SineSaw* o) {
    o->k_   = (1.0 - pow(*o->v[OSC_morph].vr, .14)) * 4.0;
    o->k2  = o->k_ * o->k_;
    o->k42 = pow(4, o->k2);
}

void SineSaw_update(SineSaw* o) {
    o->v[OSC_increment].vw = *o->v[OSC_frequency].vr * sampleperiod;

    SineSaw_incrementChanged(o);
}

void SineSaw_setSampleRate(SineSaw* o) {
    SineSaw_update(o);
}

void SineSaw_setFrequency(SineSaw* o, double v) {
    o->v[OSC_frequency].vw = v;
    SineSaw_update(o);
}

// 0 to 1, 0 being sine, 1 being saw
void SineSaw_morphChanged(SineSaw* o) {
    SineSaw_updateMorph(o);
}

double SineRamp_DSF(SineSaw* o) {
    double xn    = o->x * o->n;
    double cosx  = cos(o->x);
    double cosxn = cos(xn);

    return ((o->k42 * cosxn - pow(8, o->k2) * (cosxn * cosx - sin(xn) * sin(o->x))) * pow(2, -o->k2 * (o->n + 1)) +
            cosx * o->k42 - pow(2, o->k2)) /
           (1 - pow(2, 1 + o->k2) * cosx + o->k42);
}

double SineSaw_DSF(SineSaw* o) {
    return SineRamp_DSF(o);

    // double xn    = o->x*o->n;
    // double sinx  = sin(o->x);
    // double cosx  = cos(o->x);
    // double sinxn = sin(xn);
    // double cosxn = cos(xn);
    // double _4pk  = pow(4,o->k);
    // double kn    = o->k*o->n;
    //-(2^(-kn)*_4pk*sinx*cosxn*cosx+2^(-kn)*_4pk*sinxn*cosx^2-2^(-kn)*_4pk*sinxn-2^(-k*(n-1))*cosxn*sinx-cosx*sinx*_4pk+sinx*2^k)/sinx/(1-2^(1+k)*cosx+_4pk)
}

double SineSaw_getSample(SineSaw* o) {
    o->v[OSC_phase].vw += variable_getWithMod(&o->v[OSC_increment]) * 0.9999; // added 0.9999 to help prevent errors
    o->v[OSC_phase].vw = wrapPhase(*o->v[OSC_phase].vr);

    o->x = wrapPhase(variable_getWithMod(&o->v[OSC_phase])) * TAU;

    o->leak   = o->leak * 0.99 + 0.000005;
    o->output = o->output * (1.0 - o->leak) + SineRamp_DSF(o) * variable_getWithMod(&o->v[OSC_increment]);

    o->lastOutput = o->output;

    return o->output = -o->output * 2;
}
