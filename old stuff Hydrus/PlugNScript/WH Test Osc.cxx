string name="TestOsc";
string description=name;

array<string> inputParametersNames={"F"};
array<double> inputParameters(inputParametersNames.length,0);

int getTailSize() { return -1; }
void processBlock(BlockData& data)
{
    for(uint i=0;i<data.samplesToProcess;i++)
    {
        double ret = processSample();
        data.samples[0][i]=ret;
        data.samples[1][i]=ret;
    }
}

int incr = 0;
int incrMax = 3;

double x=0.0;
double freq = 0.00205;
// double freq = 0.0041;
double pi = 3.14159268358979;
double tau = pi * 2.0;
int amt = 41;
//double inst = (amt * 2.0) + 1.0;
//double inst = (amt * 2.0) + 1.0;
double inst = (amt * 2.0) + 1.0;
int cut = 0;
double multReq = 2.0;
double returnNext(double x, int step)
{
    double st = (step + 0.0) / (amt+0.0);
    double ramp = (st * 2.0 - 1.0);
    // double ramp = sin(st * tau * 2.0);
    double x1 = (x + st) % 1.0;
    //return ( (sin(x1 * pi * inst) / sin(x1 * pi)) / inst) * ramp;
    return ( (sin(x1 * pi * inst * multReq) / sin(x1 * pi * multReq)) / inst*multReq)  * ramp  * 0.5;
    //return (sin(x1 * pi * inst) / sin(x1 * pi)) / inst;


    // perhaps we can rephrase it to the proper smapling frqeuency and then use fewer samples for things that do not requires so many
    // such as sine wave
}

double integration = 0.0;

double processSample()
{
    x = (x + freq) % 1.0;
    double retu = 0.0;
    for (int i = 0; i < amt; ++i) retu += returnNext(x,i);
    return retu;
    //return retu / (amt+0.0);
}

void updateInputParameters()
{
    freq = pow(inputParameters[0],0.5) * 0.01 + 0.0000000001;
}




    // Basic Integer Pitch Oscillator
    //incr = (incr + 1) % incrMax;
    //double re = ((incr+0.0) / (incrMax + 0.0)) * 2.0 - 1.0;
    //return re;