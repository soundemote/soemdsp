#include "library/math.cxx"
#include "library/Filters/FMD Series/YellowjacketFilter.cxx"
#include "library/Oscillators/MusicalSaw.cxx"
#include "library/Oscillators/Phasor.cxx"

array<double> inputParameters(16);
array<string> inputParametersNames =   {"Saw Gain", "Saw Pitch", "Invert Saw", "In Gain", "Master Pitch", "Filter Pitch Offset", "Osc Pitch Offset", "feedbackGain", "SoftClipping Rail", "fModAmp", "oscSinToSquare", "oscGain", "oscSelfModGain", "oscFbGain", "railValue", "inputGainPostClamp"};
array<double> inputParametersMin =     {     0, -20, 0, 0,   -70,  -150, -150,      0, .001,   0,  0,    0,   0,     0,   0,    0};
array<double> inputParametersMax =     {    20, 127, 1, 4,   120,   150,  150,     10,   30,   10, 1,   10, 100,    10,  30,   10};
array<double> inputParametersDefault = {11.988,  30, 0, 1, 48.63, 66.26,    0, .07745,   30, 1.94, 0, .635,  20, 1.389,   7, 1.04};

array<string> inputParametersSteps = {0, 0, 2};
array<string> inputParametersFormats={".3", ".2", "", ".3", ".3", ".3", ".3", ".3",  ".3", ".3", ".3",".3", ".3", ".3", ".3", ".3"};
array<string> inputParametersEnums={"", "", "invert off;invert on"};

string name = "Yellowjacket Filter prototype";
string description = "Initial FMD filter attempt";

MusicalSaw oscillator;
YellowjacketFilterPrototype filter;

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

double sawGain = 1;
double sawPitch = 30;
bool doInvertSaw = false;
double inGain = 1;

void updateInputParameters()
{
	filter.setInputGain(inputParameters[0]);
	oscillator.setFrequency(pitchToFrequency(inputParameters[1]));
	doInvertSaw = inputParameters[2] == 0 ? false : true;
	inGain = inputParameters[3];	
	filter.setMasterPitch(inputParameters[4]);
	filter.setFilterPitchOffset(inputParameters[5]);
	filter.setOscPitchOffset(inputParameters[6]);
    filter.feedbackGain = inputParameters[7];
    filter.setSoftClipperWidth(inputParameters[8]);
    filter.fModAmp = inputParameters[9];
    filter.oscSinToSquare = inputParameters[10];
    filter.oscGain = inputParameters[11];
    filter.oscSelfModGain = inputParameters[12];
    filter.oscFbGain = inputParameters[13];    
    filter.railValue = inputParameters[14];
    filter.inputGainPostClamp = inputParameters[15];
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