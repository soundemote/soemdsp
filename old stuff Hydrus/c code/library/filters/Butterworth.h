typedef struct Butterworth
{
    double frequency;

    double a0;
    double a1;
    double a2;
    double b1;
    double b2;

    double ny;

    double x1;
    double x2;    
    double y1;
    double y2;

    double sqrt2;
    double pi_z_sampleRate;
} Butterworth;


void Butterworth_computerCoeff(Butterworth* o, double frequency)
{
    o->frequency = frequency;

    double c = 1.0 / tan(o->pi_z_sampleRate * o->frequency);
    double c2 = c * c;

    o->a0 = 
    o->a2 = 1.0 / (1.0 + o->sqrt2 * c + c2);

    o->a1 = 2.0 * o->a0;
    o->b1 = o->a1 * (1.0 - c2);
    o->b2 = o->a0 * (1.0 - o->sqrt2 * c + c2);
}

void Butterworth_init(Butterworth* o)
{
    o->x1 = o->x2 = o->y1 = o->y2 = 0.0;

    o->pi_z_sampleRate = PI / sampleRate;
    o->sqrt2 = sqrt(2.0);
    o->ny = sampleRate * 0.49;
}

void Butterworth_setfrequency(Butterworth* o, double frequency)
{
    Butterworth_computerCoeff(o, frequency);
}

double Butterworth_getSample(Butterworth* o, double signal)
{
    double tmp = o->a0 * signal + o->a1 * o->x1 + o->a2 * o->x2 - o->b1 * o->y1 - o->b2 * o->y2;
    o->x2 = o->x1; 
    o->x1 = signal; 
    o->y2 = o->y1; 

    o->y1 = tmp;
    return tmp;
}