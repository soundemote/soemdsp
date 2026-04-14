#include "library/Randoms/VibratoGenerator.cxx"
#include "library/Oscillators/Phasor.cxx"
#include "library/Parameter.cxx"

array<Parameter> parameters =
{
	Parameter("OscPitch", 0, 127, 30),
	Parameter("OscAmp", 0, 1, .5),
	Parameter("Vibrato Speed", 0, 10, .5),
	Parameter("Vibrato Range", 0, 24, 0),
	Parameter("Vibrato Desync", 0, 1, 0),
	Parameter("Random Speed", 0, 1, 0),
	Parameter("Random Range", 0, 1, 0),
	Parameter("Shape A", -1, 1, 0),
	Parameter("Shape B", -1, 1, 0),
	Parameter("Shape C", 0, 10, 1)
};

array<double> inputParameters(parameters.length);
array<string> inputParametersNames = {};
array<double> inputParametersMin = {};
array<double> inputParametersMax = {};
array<double> inputParametersDefault = {};
array<string> inputParametersUnits = {};
array<string> inputParametersFormats = {};

int parOscPitch = 0;
int parOscAmp = 1;
int parVibratoSpeed = 2;
int parVibratoRange = 3;
int parVibratoDesync = 4;
int parRandomSpeed = 5;
int parRandomRange = 6;
int parShapeA = 7;
int parShapeB = 8;
int parShapeC = 9;

string name = "Vibrato Generator";
string description = "Humanized vibrato";

VibratoGenerator vibrato;
Phasor osc;
Ellipse ellipse;

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
	}

	vibrato.setSeed(5830,23048);
	vibrato.setSampleRate(sampleRate);
	osc.setSampleRate(sampleRate);
}

int getTailSize()
{
	return -1;
}

double amp = 0;
double pitch = 0;
void updateInputParameters()
{
	amp = inputParameters[parOscPitch];
	pitch = inputParameters[parOscPitch]
	vibrato.setFrequency(inputParameters[parVibratoSpeed]);
	vibrato.setVibratoRange(inputParameters[parVibratoRange]);
	vibrato.setVibratoDesync(inputParameters[parVibratoDesync]);
	vibrato.setRandomAmplitudeRange(inputParameters[parRandomRange]);
	vibrato.setRandomFrequencyRange(inputParameters[parRandomSpeed]);
	vibrato.setShapeA(inputParameters[parShapeA]);
	vibrato.setShapeB(inputParameters[parShapeB]);
	vibrato.setShapeC(inputParameters[parShapeC]);
}

void processBlock(BlockData& data)
{
	for(uint i = 0; i < data.samplesToProcess; i++)
	{               
		double vib = vibrato.getSample();
		osc.setFrequency(pitchToFrequency(round(pitch)));

		double output = ellipse.getValueSinToSaw(osc.getSample(), .5) * amp;
		//output = vib;

		for(uint ch = 0; ch < audioOutputsCount; ++ch)
			data.samples[ch][i] = output;
	}
}