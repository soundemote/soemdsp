#include "library/math.cxx"
#include "library/Filters/FMD Series/SuperLoveFilter.cxx"
#include "library/Filters/Lowpass.cxx"
#include "library/Oscillators/MusicalSaw.cxx"
#include "library/Oscillators/Phasor.cxx"

array<double> inputParameters(15);
array<string> inputParametersNames = {"Saw Gain", "Saw Pitch", "Invert Saw", "In Gain", "Filter Pitch", "FB Gain", "Rails", "Shape", "P1", "P2", "P3", "OutputTap", "Phase Offset", "bleed filter", "bleed amp"};
array<double> inputParametersMin = {0, -20, 0, 0, -50, 0, 0, -1, -80, -80, -80, 0, -1, -50, -30};
array<double> inputParametersMax = {20, 127, 1, 4, 130, 50, 100, 1, 127, 127, 127, 2, 1, 150, +30};
array<double> inputParametersDefault = {.164, 52.07, 0, 1, 45, 0.1, 1, 1, 22.08, 7.08, 36.0864, 0, 0, 0, 0};
array<string> inputParametersSteps = {0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 3};
array<string> inputParametersFormats={".3", ".2", "", ".3",".3", ".3", ".3", ".3", ".3", ".3", ".3", ".3", ".3", ".3"};
array<string> inputParametersEnums={"", "", "invert off;invert on"};

string name = "3-pole Lowpass prototype";
string description = "Initial FMD filter attempt";

MusicalSaw oscillator;
//Phasor oscillator;
SuperloveFilterPrototype filter;
Lowpass bleedFilter;

void initialize()
{
	oscillator.setSampleRate(sampleRate);
	oscillator.setMorph(1);
	filter.setSampleRate(sampleRate);
	bleedFilter.setSampleRate(sampleRate);
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
	doInvertSaw = inputParameters[2] == 0 ? false : true;

	inGain = inputParameters[3];

	filter.setFilterPitch1(inputParameters[8]);
	filter.setFilterPitch2(inputParameters[9]);
	filter.setFilterPitch3(inputParameters[10]);

	filter.setOutputTap(inputParameters[11]);

	filter.setInputGain(inputParameters[0]);
	filter.setFeedbackGain(inputParameters[5]);
	filter.setRails(inputParameters[6]);
	filter.setShape(inputParameters[7]);
	filter.setPhaseOffset(inputParameters[12]);

	oscillator.setFrequency(pitchToFrequency(inputParameters[1]));

	bleedFilter.setFrequency(pitchToFrequency(inputParameters[13]));
}

void processBlock(BlockData& data)
{
}

void processSample(array<double>& ioSample)
{
	double output = 0;

	output = oscillator.getSample();

	filter.setPitch(inputParameters[4] + bleedFilter.getSample(output * inputParameters[14]));

	output *= inGain;

	if (doInvertSaw)
		output = -output;

	output = filter.getSample(output);

	for (uint ch = 0; ch < audioInputsCount; ++ch)
		ioSample[ch] = output;
}