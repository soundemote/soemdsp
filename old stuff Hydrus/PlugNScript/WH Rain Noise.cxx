// Rain Noise by Walter H. Hackett IV

string name="Rain Noise";
string description=name;

array<double> inputParameters(1,0);
array<string> inputParametersNames={"Drops"};

double amt = 10000;
int timeA = 100000;

void processBlock(BlockData& data)
{
    for(uint i=0;i<data.samplesToProcess;i++)
    {
        timeA = timeA + 5;
        data.samples[0][i]=processSample();
        data.samples[1][i]=processSample();
    }
}

double processSample()
{
    double dbl = amt * rand() + 1.0;
    return sin(timeA * dbl) / dbl;
}

void updateInputParameters()
{
    amt = (1-pow(inputParameters[0],.020)) * 20000 + 0.5;
}

int getTailSize() { return -1; }
