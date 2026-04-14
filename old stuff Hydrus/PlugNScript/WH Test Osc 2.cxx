string name="TestOsc";
string description=name;

array<string> inputParametersNames={"F"};
array<double> inputParameters(inputParametersNames.length,0);

int getTailSize() { return -1; }
void processBlock(BlockData& data)
{
    for(uint i=0;i<data.samplesToProcess;i++)
    {
        double ret = processSample2();
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
int amt = 50;
//double inst = (amt * 2.0) + 1.0;
//double inst = (amt * 2.0) + 1.0;
double inst = (amt * 2.0) + 1.0;
int cut = 0;
double multReq = 2.0;

double integration = 0.0;

double processSample()
{
    // a0 = 1 - f
    //     a1 = f - 1
    //     b1 = 1 - f
    //     out = in - ( s += f * ( in - s ) );

    x = (x + freq) % 1.0;
    double ou = (sin(x* pi * inst) / sin(x * pi)) / inst;
    
    //integration = (integration + ou * 0.5) * 0.995;
    integration = (integration + ou * 0.9) * 0.995;
    return  integration;
}


double ma = 0.4;
double mb = 0.6;

double dcB = 0.0;

double processSample2()
{
    // a0 = 1 - f
    //     a1 = f - 1
    //     b1 = 1 - f
    //     out = in - ( s += f * ( in - s ) );
    // (sin(x*31)/tan(x))/31
    x = (x + freq) % 1.0;

    double ou = (sin(x* pi * inst) / sin(x * pi)) / inst;
    //double ou = (sin(x* pi * inst) / sin(x * pi)) / inst;
    //double ou = (sin(x* pi * (inst+1.0)) / tan(x * pi)) / (inst+1.0);
    


    // return cos(x*pi*2);
    double se = (pow (cos(x*pi*2)*0.5+0.5, 5) * 0.031 -0.006);
    //integration = integration * (1.0 - se) + ou * se;
    
    integration = integration * (1.0 - se) + ou * se;
    
    // Overgration oversampling
    // now we must oversample the intergration portion by x2,x4,x8 so a more precision shape can be made from the follower


    // dc blocker
    dcB += 0.005 * (integration - dcB);
    //integration = integration - dcB;
    return (integration - dcB) * 16.0;
    
    // allpass filter to alter the phases

}

void updateInputParameters()
{
    double freqCalc = (pow(inputParameters[0],5.0) + 0.001) * sampleRate * 0.5;
    freq =  freqCalc/sampleRate;
    amt = floor(sampleRate * 0.5 / freqCalc);
    inst =amt * 2.0 + 1.0;
}

    // Basic Integer Pitch Oscillator
    //incr = (incr + 1) % incrMax;
    //double re = ((incr+0.0) / (incrMax + 0.0)) * 2.0 - 1.0;
    //return re;