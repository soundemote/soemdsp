#include "library/math.cxx"
#include "library/Filters/OnePoleFilter.cxx"

array<double> inputParameters(2);
array<string> inputParametersNames = {"LP / HP", "Frequency"};
array<double> inputParametersMin = {1, 0};
array<double> inputParametersMax = {2, sampleRate};
array<double> inputParametersDefault = {1, 10000};
array<string> inputParametersUnits = {"", "Hz"};
array<string> inputParametersFormats={"0",".4"};
array<string> inputParametersEnums={"Lowpass IIT;Highpass MZT"};
array<int> inputParametersSteps={2};

string name = "One Pole Filter";
string description = "1-pole lowpass IIT and highpas MZT filters";

OnePoleFilter filter;

void initialize()
{
	filter.setSampleRate(sampleRate);
}

int getTailSize()
{
	return -1;
}

int filterSelect = 0;
double frequency = 0;
double width = 0;

void updateInputParameters()
{
	filterSelect = roundToInt(inputParameters[0]);
	frequency = inputParameters[1];

	filter.setFrequency(frequency);
	filter.setMode(filterSelect);

	if (filterSelect == 0)
		print("You selected Lowpass IIT filter.");

	if (filterSelect == 1)
		print("You selected the Highpass MZT filter.");
}

void processBlock(BlockData& data)
{
}

void processSample(array<double>& ioSample)
{
	double output = 0;

	output = filter.getSample(ioSample[0]);

	for (uint ch = 0; ch < audioInputsCount; ++ch)
		ioSample[ch] = output;
}