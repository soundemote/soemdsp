#pragma once

typedef struct Highpass
{
    variable v[FLT_NUMVARS];

    double freqToOmega;

	// buffering
	double x1;
	double y1;

	// filter coefficients:
	double b0;
	double b1;
	double a1;
	double w;
    
} Highpass;

//void Highpass_DBG(Highpass* o)
//{
//    LINE
//    LOGF(*o->v[FLT_frequency].dr);
//    LINE
//    LOGF(*o->v[FLT_coeff].dr);
//    LINE
//    LOGF(o->freqToOmega);
//    LOGF(o->x1);
//    LOGF(o->y1);
//    LOGF(o->b0);
//    LOGF(o->b1);
//    LOGF(o->a1);
//    LOGF(o->w);
//    LogLn("");
//}

void Highpass_init(Highpass* o)
{
    for (int i = 0; i < FLT_NUMVARS; ++i)
        variable_init(&o->v[i]);

    o->v[FLT_coeff].dr = &TAU_z_samplerate;

    o->freqToOmega = 0;

	o->x1 = 0;
	o->y1 = 0;

	o->b0 = 1;
	o->b1 = 0;
	o->a1 = 0;
	o->w  = 0;    
}

void Highpass_reset(Highpass* o)
{
    o->x1 = 0;
    o->y1 = 0;
}

void Highpass_update(Highpass* o)
{
    o->freqToOmega = *o->v[FLT_coeff].dr;
    o->w = o->freqToOmega * *o->v[FLT_frequency].dr;

    o->a1 = exp(-o->w);
    o->b0 = 0.5*(1.0 + o->a1);
    o->b1 = -o->b0;
}

void Highpass_incrementChanged(Highpass* o)
{
    o->w = *o->v[FLT_coeff].dr * *o->v[FLT_frequency].dr;

    o->a1 = exp(-o->w);
    o->b0 = 0.5*(1.0 + o->a1);
    o->b1 = -o->b0;
}

void Highpass_copystate(Highpass* o, Highpass* copyto)
{
    copyto->a1 = o->a1;
    copyto->b0 = o->b0;
    copyto->b1 = o->b1;
}

void Highpass_setSampleRate(Highpass* o)
{
    Highpass_update(o);
}

void Highpass_setFrequency(Highpass* o, double v)
{
    o->v[FLT_frequency].dw = v;
    Highpass_update(o);
}

double Highpass_getSample(Highpass* o, double v)
{
    o->y1 = o->b0*v + o->b1*o->x1 + o->a1*o->y1;
    o->x1 = v;

    return o->y1;
}
