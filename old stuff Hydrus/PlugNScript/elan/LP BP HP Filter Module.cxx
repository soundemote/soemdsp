#include "library/math.cxx"
#include "library/Filters/Lowpass.cxx"
#include "library/Filters/Bandpass.cxx"
#include "library/Filters/Highpass.cxx"
#include "library/Randoms/NoiseGenerator.cxx"

array<double> inputParameters(4);
array<string> inputParametersNames = {"LP / BP / HP", "Frequency", "Width", "Input Amp"};
array<double> inputParametersMin = {0, 0, -5000, 0};
array<double> inputParametersMax = {2, 20000, 5000, 1};
array<double> inputParametersDefault = {0, 10000, 0, .2};
array<string> inputParametersUnits = {"", "Hz", "Hz"};
array<string> inputParametersFormats={"0",".4",".4"};
array<string> inputParametersEnums={"Lowpass;Bandpass;Highpass"};
array<int> inputParametersSteps={3};

string name = "One Pole Filter";
string description = "simple one pole lowpass, bandpass, or highpass filter";

Lowpass lowpass;
Bandpass bandpass;
Highpass highpass;
NoiseGenerator noise;

void initialize()
{
	lowpass.setSampleRate(sampleRate);
	bandpass.setSampleRate(sampleRate);
	highpass.setSampleRate(sampleRate);
}

int getTailSize()
{
	return -1;
}

int filterSelect = 0;
double frequency = 0;
double width = 0;
double amp = 0;

void updateInputParameters()
{
	filterSelect = roundToInt(inputParameters[0]);
	frequency = inputParameters[1];
	width = inputParameters[2];
	amp = inputParameters[3];

	lowpass.setFrequency(frequency);
	bandpass.setFrequency(frequency);
	highpass.setFrequency(frequency);

	bandpass.setWidth(width);

	if (filterSelect == 0)
		print("Congratz! You have selected the lo' to the flo' lowpass filter!");

	if (filterSelect == 1)
		print("Congratz! You have selected the bippity boopity boppity bandpass filter!");

	if (filterSelect == 2)
		print("Congratz! You have selected the hipity hopity highpass filter!");
}

// void processBlock(BlockData& data)
// {
//     for(uint i = 0; i < data.samplesToProcess; i++)
//     {
//        	double output = 0;

// 		double input = data.samples[i] * amp;

// 		switch(filterSelect)
// 		{
// 		case 0:
// 			output = lowpass.getSample(input);
// 			break;
// 		case 1:
// 			output = bandpass.getSample(input);
// 			break;
// 		case 2:
// 			output = highpass.getSample(input);
// 			break;
// 		}

// 		for (uint ch = 0; ch < audioInputsCount; ++ch)
// 			data.samples[ch][i] = output;
//     }

// }

void processSample(array<double>& ioSample)
{
	double input = ioSample[0] * amp;

	double output = 0;

	switch(filterSelect)
	{
	case 0:
		output = lowpass.getSample(input);
		break;
	case 1:
		output = bandpass.getSample(input);
		break;
	case 2:
		output = highpass.getSample(input);
		break;
	}

	for (uint ch = 0; ch < audioInputsCount; ++ch)
		ioSample[ch] = output;
}