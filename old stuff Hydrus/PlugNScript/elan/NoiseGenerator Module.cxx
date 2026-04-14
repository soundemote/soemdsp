#include "library/Filters/Bandpass.cxx"
#include "library/Randoms/NoiseGenerator.cxx"

array<double> inputParameters(3);
array<string> inputParametersNames = {"BP Freq", "BP Width", "Amplitude"};
array<double> inputParametersMin = {0, 0, 0};
array<double> inputParametersMax = {40000, 20000, 1};
array<double> inputParametersDefault = {150, 1, .5};
array<string> inputParametersUnits = {"Hz", ""};
array<string> inputParametersFormats={".3", ".2", ".2"};

string name = "Drift Generator";
string description = "slowly evolving random value generator for organic drift";

NoiseGenerator noise;
Bandpass bandpass;

void initialize()
{
    bandpass.setSampleRate(sampleRate);
}

int getTailSize()
{
	return -1;
}

double amplitude = 0;
void updateInputParameters()
{
	bandpass.setFrequency(inputParameters[0]);
    bandpass.setWidth(inputParameters[1]);
	amplitude = inputParameters[2];
}

void processBlock(BlockData& data)
{
	for(uint i = 0; i < data.samplesToProcess; i++)
    {
		for (uint ch = 0; ch < audioOutputsCount; ++ch)
			data.samples[ch][i] = noise.getSampleBipolar() * amplitude;	
	}
}