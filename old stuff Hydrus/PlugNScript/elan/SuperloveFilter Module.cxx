#include "library/math.cxx"
#include "library/Filters/FMD Series/SuperLoveFilter.cxx"
#include "library/Oscillators/MusicalSaw.cxx"

array<double> inputParameters(6);
array<string> inputParametersNames = {"Preset", "Input Gain", "Cutoff", "Resonance", "Noise", "Output Gain"};
array<double> inputParametersMin = {0, 0, 0, 0, 0, 0};
array<double> inputParametersMax = {6, 3, 1, 1, 1, 3};
array<double> inputParametersDefault = {0, 0.75, .5, .5, .2, 1};
array<string> inputParametersFormats={"0", ".4", ".4", ".4", ".4", ".4",};
array<int> inputParametersSteps={7};
array<string> inputParametersEnums={"Hot;Crunchy;Howling;Woody;Squelchy1;Squelchy2;Quirky"};

string name = "FMD Superlove Series";
string description = "Initial FMD filter attempt";

SuperloveFilter filter;

void initialize()
{
	filter.setSampleRate(sampleRate);
}

int getTailSize()
{
	return -1;
}

double inGain = 0;
double outGain = 0;

void updateInputParameters()
{
	inGain = inputParameters[1];
    outGain = inputParameters[5];

    filter.setPreset(inputParameters[0]);

	filter.setCutoff(inputParameters[2]);
	filter.setResonance(inputParameters[3]);
	filter.setNoiseAmplitude(inputParameters[4]);
}

void processBlock(BlockData& data)
{
}

void processSample(array<double>& ioSample)
{
	double output = filter.getSample(ioSample[0] * inGain);

	for (uint ch = 0; ch < audioInputsCount; ++ch)
		ioSample[ch] = output * outGain;
}