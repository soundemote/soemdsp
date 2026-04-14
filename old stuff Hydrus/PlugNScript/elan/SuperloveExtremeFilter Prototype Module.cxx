#include "library/math.cxx"
#include "library/Filters/FMD Series/SuperLoveExtremeFilter.cxx"
#include "library/Oscillators/MusicalSaw.cxx"
#include "library/Oscillators/Phasor.cxx"
#include "library/Parameter.cxx"
#include "library/Oscillators/SineSaw.cxx"

array<Parameter> parameters = 
{
    Parameter("Saw Gain", 0, 20, 1),
    Parameter("Saw Pitch", -20, 127, 30),
    Parameter("Invert Saw", 0, 1, 0),
    Parameter("In Gain", 0, 1, 1),
    
    Parameter("FB Gain", -10, 10, 0.1),
    Parameter("Shape", -1, 1, 0),
    Parameter("PhaseOffset", -3, 3, 0),
    Parameter("Rails", 0, 10, 0),

	Parameter("Highpass freq offset", -10000, 10000, 0),
	Parameter("Lowpass freq offset", -20000, 20000, 0),
	Parameter("Bandwidth freq offset", -10000, 10000, 0),
	Parameter("Center Pitch", -50, 150, 0),
};

int parSawGain = 0;
int parSawPitch = 1;
int parInvertSaw = 2;
int parInGain = 3;
int parFBGain = 4;
int parShape = 5;
int parPhaseOffset = 6;
int parRails = 7;
int parHighpassFreqOffset = 8;
int parLowpassFreqOffset = 9;
int parBandwidthFreqOffset = 10;
int parCenterPitch = 11;

array<double> inputParameters(parameters.length-1);
array<string> inputParametersNames = {};
array<double> inputParametersMin = {};
array<double> inputParametersMax = {};
array<double> inputParametersDefault = {};
array<string> inputParametersUnits = {};
array<string> inputParametersFormats = {};
array<string> inputParametersEnums = {};
array<int>    inputParametersSteps = {};

string name = "Simple screaming resonant 2-pole LP BP HP filter";
string description = "Initial FMD filter attempt";

MusicalSaw oscillator;
//Phasor oscillator;
//PolyBLEP oscillator;
SuperloveExtremeFilterPrototype filter;

void initialize()
{
	for (int i = 0; i < int(parameters.length); ++i)
    {
        inputParametersNames.insertLast(parameters[i].name);
        inputParametersMin.insertLast(parameters[i].minVal);
        inputParametersMax.insertLast(parameters[i].maxVal);
        inputParametersDefault.insertLast(parameters[i].defaultVal);
        inputParametersUnits.insertLast(parameters[i].units);
        inputParametersFormats.insertLast(parameters[i].format);
        inputParametersEnums.insertLast(parameters[i].enums);
        inputParametersSteps.insertLast(parameters[i].steps);
    }

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
	oscillator.setFrequency(pitchToFrequency(inputParameters[parSawPitch]));
	doInvertSaw = inputParameters[parInvertSaw] == 0 ? false : true;
	inGain = inputParameters[parSawGain];
	filter.setInputGain(inputParameters[parInGain]);

	filter.setHighpassFreqOffset(inputParameters[parHighpassFreqOffset]);
	filter.setLowpassFreqOffset(inputParameters[parLowpassFreqOffset]);
	filter.setBandwidthFreqOffset(inputParameters[parBandwidthFreqOffset]);
	filter.setPitch(inputParameters[parCenterPitch]);

	filter.setFeedbackGain(inputParameters[parFBGain]);
	filter.setShape(inputParameters[parShape]);
	filter.setPhaseOffset(inputParameters[parPhaseOffset]);
	filter.setRails(inputParameters[parRails]);
}

void processBlock(BlockData& data)
{
	 for(uint i = 0; i < data.samplesToProcess; i++)
    {       
		double output = 0;

		output = oscillator.getSample() * inGain;

		if (doInvertSaw)
			output = -output;

		output = filter.getSample(output);

        for(uint ch = 0; ch < audioOutputsCount; ++ch)
             data.samples[ch][i] = output;

    }
}