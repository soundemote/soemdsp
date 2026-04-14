#include "library/Midi.hxx"
#include "library/Constants.hxx"

array<double> inputParameters(1,0);
array<string> inputParametersNames={"Size"};
string name="RanWalk";
string description=name;

const double tau = 3.14159265358979 * 2.0;
double valueL = 0.0;
double valueR = 0.0;
double stepSize = 0.0;

void processBlock(BlockData& data)
{
	for(uint i=0; i<data.samplesToProcess; ++i)
	{
		double rL = rand(-1, 1);
		valueL = valueL + (rL - valueL * stepSize) * stepSize;

		double rR = rand(-1, 1);
		valueR = valueR + (rR - valueR * stepSize) * stepSize;

		data.samples[0][i] = valueL;
		data.samples[1][i] = valueR;
	}
}

void updateInputParameters()
{
	stepSize = pow(inputParameters[0],4);
}

int getTailSize()
{
	return -1;
}
