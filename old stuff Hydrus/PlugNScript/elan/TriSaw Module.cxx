#include "library/math.cxx"
#include "library/Oscillators/TriSaw.cxx"

array<double> inputParameters(5);
array<string> inputParametersNames = {"Amplitude", "Frequency", "Shape", "Phase Offset", "Reset"};
array<double> inputParametersMin = {0, 0, 0, -1, 0};
array<double> inputParametersMax = {1, 20000, 1, 1, 1};
array<double> inputParametersDefault = {.5, 30, 0, 0, 0};
array<string> inputParametersUnits = {};
array<string> inputParametersFormats={".2",".2", ".2",".2"};
array<string> inputParametersEnums={"", "", "", "", "running;RESETTING NOW"};
array<int> inputParametersSteps={0, 0, 0, 0, 2};

string name = "TriSaw Module";
string description = "Aliased oscillator that transforms between saw and triangle.";

TriSaw oscillator;

void initialize()
{
	oscillator.setSampleRate(sampleRate);
}

int getTailSize()
{
	return -1;
}

double f = 0;
double amp = 0;
bool doReset = false;

void updateInputParameters()
{
    amp = inputParameters[0];
    
	oscillator.setFrequency(inputParameters[1]);
    oscillator.setShape(inputParameters[2]);
    oscillator.setPhaseOffset(inputParameters[3]);
	doReset = inputParameters[4] != 0;
}

void processBlock(BlockData& data)
{
}

void processSample(array<double>& ioSample)
{
	double output = 0;

	if (doReset)
	{
		oscillator.reset();
		for (uint ch = 0; ch < audioInputsCount; ++ch)
			ioSample[ch] = 0;
		return;
	}	

	output = oscillator.getSample();

	for (uint ch = 0; ch < audioInputsCount; ++ch)
		ioSample[ch] = output * amp;
}