#pragma once

/*
Simple one pole lowpass filter
*/

typedef struct 
{    
    variable v[FLT_NUMVARS];

    double output;
    double a0;
    double b1;

} Lowpass;

double Lowpass_calculate_b1(double increment)
{
    return exp(-TAU * increment);
}

void Lowpass_init(Lowpass* o)
{
    for (int i = 0; i < FLT_NUMVARS; ++i)
        variable_init(&o->v[i]);

    o->output = 0;
    o->a0 = 0;
    o->b1 = 0;
}

void Lowpass_incrementChanged(Lowpass* o)
{
    o->b1 = -*o->v[FLT_coeff].vr;
    o->a0 = 1.0+o->b1;
}

void Lowpass_reset(Lowpass* o)
{
    o->output = 0;
}

double Lowpass_getSample(Lowpass* o, double v)
{
    o->output += o->a0 * (v - o->output);
    return o->output;
}