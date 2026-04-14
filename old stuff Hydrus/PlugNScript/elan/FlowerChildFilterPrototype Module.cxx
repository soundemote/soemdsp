#include "library/math.cxx"
#include "library/Filters/FMD Series/FlowerChildFilter.cxx"
#include "library/Oscillators/MusicalSaw.cxx"
#include "library/Oscillators/Phasor.cxx"

array<double> inputParameters(15);
array<string> inputParametersNames = {"Saw Gain", "Saw Pitch", "Invert Saw", "In Gain", "Master Pitch", "PM Amount", "FM Amount", "FB Gain", "Rails", "ResonatorGain", "Squareness", "P1", "P2", "OutputTap", "Phase Offset"};
array<double> inputParametersMin = {0, -20, 0, 0, -50, 0,-10, -50, -10, 0, 0, -50, -50, 0, -1};
array<double> inputParametersMax = {20, 127, 1, 4, 150, 1,10,  50, 10, 10, 1, 150, 150, 1, 1};
array<double> inputParametersDefault = {.1, 30, 0, 1, 45, 0.1, 0, 1, 1, 1, 0, 0, 0, 1, 0};
array<string> inputParametersSteps = {0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3};
array<string> inputParametersFormats={".3", ".2", "", ".3",".3", ".3",".3",  ".3", ".3", ".3", ".3", ".3", ".3", "", ".3"};
array<string> inputParametersEnums={"", "", "invert off;invert on"};

string name = "FlowerChild prototype";
string description = "Initial FMD filter attempt";

MusicalSaw oscillator;
FlowerChildFilterPrototype filter;

void initialize()
{
	oscillator.setSampleRate(sampleRate);
	oscillator.setMorph(1);
	filter.setSampleRate(sampleRate);
}

int getTailSize()
{
	return -1;
}

double sawGain = 0;
double sawPitch = 0;
bool doInvertSaw = false;

double inGain = 0;
double pitch = 0;
double fbGain = 0;
double shape = 0;
double phaseOffset = 0;

void updateInputParameters()
{
	filter.setInputGain(inputParameters[0]);
	oscillator.setFrequency(pitchToFrequency(inputParameters[1]));
	doInvertSaw = inputParameters[2] == 0 ? false : true;
	inGain = inputParameters[3];	
	filter.setPitch(inputParameters[4]);
	filter.setPMAmount(inputParameters[5]);
	filter.setFMAmount(inputParameters[6]);
	filter.setFeedbackGain(inputParameters[7]);
	filter.setRailValue(inputParameters[8]);
	filter.setResonatorGain(inputParameters[9]);
	filter.setSquareness(inputParameters[10]);
	filter.setFilterPitch1(inputParameters[11]);
	filter.setFilterPitch2(inputParameters[12]);
	filter.setOutputTap(inputParameters[13]);
	filter.setPhaseOffset(inputParameters[14]);	
}

void processBlock(BlockData& data)
{
}

void processSample(array<double>& ioSample)
{
	double output = 0;

	output = oscillator.getSample() * inGain;

	if (doInvertSaw)
		output = -output;

	output = filter.getSample(output);

	for (uint ch = 0; ch < audioInputsCount; ++ch)
		ioSample[ch] = output;
}