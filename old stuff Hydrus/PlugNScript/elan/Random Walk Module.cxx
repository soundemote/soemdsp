#include "library/Randoms/RandomWalk.cxx"

array<double> inputParameters(3);
array<string> inputParametersNames = {"Color", "Range", "Bias"};
array<double> inputParametersMin = {0, 0, -1};
array<double> inputParametersMax = {20000, 1, 1};
array<double> inputParametersDefault = {1, .5, 0};
array<string> inputParametersUnits = {"Hz"};
array<string> inputParametersFormats={".4", ".4", ".4"};

string name = "Elan Random Walk";
string description = "slowly evolving random value generator for organic drift";

RandomWalk randomWalk;

void initialize()
{
	randomWalk.setSampleRate(sampleRate);
}

int getTailSize()
{
	return -1;
}

void updateInputParameters()
{
	randomWalk.setColor(inputParameters[0]);
	randomWalk.setRange(inputParameters[1]);
	randomWalk.setBias(inputParameters[2]);
}

void processBlock(BlockData& data)
{
	for(uint i = 0; i < data.samplesToProcess; i++)
    {
		for (uint ch = 0; ch < audioOutputsCount; ++ch)
			data.samples[ch][i] = randomWalk.getSample();	
	}
}