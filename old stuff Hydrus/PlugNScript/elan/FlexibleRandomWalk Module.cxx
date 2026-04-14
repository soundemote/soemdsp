#include "library/Randoms/FlexibleRandomWalk.cxx"
#include "library/Dynamics/SoftClipper.cxx"

array<double> inputParameters(3);
array<string> inputParametersNames = {"Color", "Detail", "Amplitude"};
array<double> inputParametersMin = {0, 0, 0};
array<double> inputParametersMax = {1, 20, 1};
array<double> inputParametersDefault = {.001, 5, 1};
array<string> inputParametersUnits = {"", "Hz", ""};
array<string> inputParametersFormats={".4",".2",".2"};

string name = "Flexible Random Walk";
string description = "slowly evolving random value generator for organic drift";

FlexibleRandomWalk flexibleRandomWalk;

void initialize()
{
	flexibleRandomWalk.setSampleRate(sampleRate);
}

int getTailSize()
{
	return -1;
}

double amplitude;
void updateInputParameters()
{
	flexibleRandomWalk.setColor(inputParameters[0]);
	flexibleRandomWalk.setDetail(inputParameters[1]);
	amplitude = inputParameters[2];
}

void processBlock(BlockData& data)
{
	for(uint i = 0; i < data.samplesToProcess; i++)
    {
		for (uint ch = 0; ch < audioOutputsCount; ++ch)
			data.samples[ch][i] = flexibleRandomWalk.getSample() * amplitude;	
	}
}