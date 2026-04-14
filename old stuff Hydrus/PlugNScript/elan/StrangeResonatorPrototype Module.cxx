#include "library/math.cxx"
#include "library/Filters/FMD Series/StrangeResonator.cxx"
#include "library/Oscillators/MusicalSaw.cxx"
#include "library/Oscillators/Phasor.cxx"

array<double> inputParameters(24);
array<string> inputParametersNames = {"Saw Gain", "Saw Pitch", "Invert Saw", "In Gain", "Master Pitch", "Filter Pitch Offset", "Osc Pitch Offset", "PM1 Amount", "PM2 Amount", "FM1 Amount","FM2 Amount", "Osc1Ratio", "Osc2Ratio", "Post Filter FB Gain", "Osc1SinToSquare", "Osc2SinToSquare", "Osc 1 Ph", "Osc 2 Ph", "Osc 1 f", "Osc 2 f", "osc1FeedbackAmp", "osc1SelfModAmp", "osc2FeedbackAmp", "osc2SelfModAmp"};
array<double> inputParametersMin =     { 0,  -20, 0, 0,  -70, -150, -150,    -3, -10,    0,      0,  -10, -10,    0,      0, 0,  0,  0, -30, -30,  0,  0,  0,  0};
array<double> inputParametersMax =     {20,  127, 1, 4,  120,  150,  150,     3,  10,  300,   1300,  100, 100,    1,      1, 1,  1,  1,  30,  30, 40, 40, 40, 40};
array<double> inputParametersDefault = {.2,   30, 0, 1,   20, -36,   0,  .234,   0,  268,  1214, 1.01,   0,  .001, .00749, 0,  0,  0,    0,  0, .1, .5,  3, 10};

array<string> inputParametersSteps = {0, 0, 2};
array<string> inputParametersFormats={".3", ".2", "", ".3", ".3", ".3", ".3", ".3",  ".3", ".3", ".3",".3", ".3", ".3", ".3", ".3", ".3", ".3", ".3", ".3", ".3", ".3", ".3", ".3"};
array<string> inputParametersEnums={"", "", "invert off;invert on"};

string name = "Strange Resonator prototype";
string description = "Initial FMD filter attempt";

MusicalSaw oscillator;
StrangeResonatorPrototype filter;

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
	filter.setMasterPitch(inputParameters[4]);
	filter.setFilterPitchOffset(inputParameters[5]);
	filter.setOscPitchOffset(inputParameters[6]);
	filter.Pm1Amount = inputParameters[7];
	filter.Pm2Amount = inputParameters[8];
	filter.Fm1Amount = inputParameters[9];
	filter.Fm2Amount = inputParameters[10];
	filter.osc1Ratio = inputParameters[11];
	filter.osc2Ratio = inputParameters[12];	
	filter.postFilterGain = inputParameters[13];
	filter.osc1SinToSquare = inputParameters[14];
	filter.osc2SinToSquare = inputParameters[15];
	filter.osc1Ph = inputParameters[16];
	filter.osc2Ph = inputParameters[17];
	filter.osc1F  = inputParameters[18];
	filter.osc2F  = inputParameters[19];
	filter.osc1SelfModAmp = inputParameters[20];
	filter.osc1FeedbackAmp = inputParameters[21];
	filter.osc2SelfModAmp = inputParameters[22];
	filter.osc2FeedbackAmp = inputParameters[23];
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