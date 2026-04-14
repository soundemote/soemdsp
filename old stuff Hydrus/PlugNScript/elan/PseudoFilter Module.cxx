#include "library/Randoms/FlexibleRandomWalk.cxx"
#include "library/Randoms/NoiseGenerator.cxx"

array<double> inputParameters(2);
array<string> inputParametersNames = {"Frequency", "Randomness"};
array<double> inputParametersMin = {0, 0};
array<double> inputParametersMax = {20000, 2};
array<double> inputParametersDefault = {20000, 0};
array<string> inputParametersUnits = {"Hz"};
array<string> inputParametersFormats={".2", ".2"};

string name = "Pseudo Filter";
string description = "sinusoidal interpolator sent through a linear smoother to create smoothly interpolated values";

LinearSmoother linearSmoother;
NoiseGenerator noiseGenerator;

void initialize()
{
	samplePeriod = 1.0/double(sampleRate);
	linearSmoother.setSampleRate(sampleRate);
	increment = samplePeriod * frequency;
}

int getTailSize()
{
	return -1;
}

void updateInputParameters()
{
	frequency = inputParameters[0];
	randomness = inputParameters[1];
	linearSmoother.setSmoothingTime(1.0/frequency);
	increment = samplePeriod * frequency;
}

void processBlock(BlockData& data)
{
}

double frequency = 0;
double randomness = 0;
double lastSampledValue = 0;
double currentSampledValue = 0;
double phase = 0;
double samplePeriod = 0;
double increment = 0;
double output = 0;

void processSample(array<double>& ioSample)
{
	double r = noiseGenerator.getSampleUnipolar();
	phase += clamp(r * randomness, 0, 1);

	if (phase > 1)
	{
		phase -= 1;
		lastSampledValue = currentSampledValue;
		currentSampledValue = ioSample[0];
	}

	double sinusoidSmoother = map(cos(phase*PI + PI), -1, 1, lastSampledValue, currentSampledValue);
	output = linearSmoother.getSample(sinusoidSmoother);

	for (uint ch = 0; ch < audioInputsCount; ++ch)
	{
		ioSample[ch] = output;
	}
}