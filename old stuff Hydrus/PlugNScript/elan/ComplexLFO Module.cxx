#include "library/math.cxx"
#include "library/Parameter.cxx"
#include "../library/Midi.hxx"
#include "library/Oscillators/ComplexLFO.cxx"
#include "library/Filters/Lowpass.cxx"
#include "library/Oscillators/MusicalSaw.cxx"

array<Parameter> parameters =
{
    Parameter("RandomizeWaveform", 0, 1, 0, "", "", 2, "running;RANDOMIZING!!"),
    Parameter("LoopTime", .125, 16, 1),
    Parameter("Frequency", 0, 100, 1),
    Parameter("Seed", 0, 999999, 42),
    Parameter("LFO All Level", -10, 10, 1),
    Parameter("LFO 1 Level", -4, 4, 1),
    Parameter("LFO 2 Level", -4, 4, 1),
    Parameter("LFO 3 Level", -4, 4, 1),
    Parameter("LFO 4 Level", -4, 4, 1),
    Parameter("Sin Off", 0, 1, 0),
    Parameter("Tri Off", 0, 1, 0),
    Parameter("Ramp Off", 0, 1, 0),
    Parameter("Saw Off", 0, 1, 0),
    Parameter("Square Off", 0, 1, 0),
    Parameter("OscAmp", 0, 1, .5),
    Parameter("OscPitch", 0, 127, .5),
    Parameter("OscMorph", 0, 1, 1),
    Parameter("FilterPitch", -50, 150, 0)
};

int parRandomizeWaveform = 0;
int parLoopTime = 1;
int parFrequency = 2;
int parSeed = 3;
int parLFOAllLevel = 4;
int parLFO1Level = 5;
int parLFO2Level = 6;
int parLFO3Level = 7;
int parLFO4Level = 8;
int parSinOff = 9;
int parTriOff = 10;
int parRampOff = 11;
int parSawOff = 12;
int parSquareOff = 13;
int parOscAmp = 14;
int parOscPitch = 15;
int parOscMorph = 16;
int parFilterPitch = 17;

array<double> inputParameters(parameters.length);
array<string> inputParametersNames = {};
array<double> inputParametersMin = {};
array<double> inputParametersMax = {};
array<double> inputParametersDefault = {};
array<string> inputParametersUnits = {};
array<string> inputParametersFormats = {};
array<string> inputParametersEnums = {};
array<int>    inputParametersSteps = {};

string name = "Complex LFO";
string description = "A tempo-synced LFO that creates complex patterns";

ComplexLFO lfo;
MusicalSaw oscillator;
Lowpass filter;

double inGain = 1;
bool isRandomizing = false;

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
    lfo.setSampleRate(sampleRate);
    lfo.setSeed(7469);
    lfo.randomizePattern();
}

int getTailSize()
{
	return -1;
}

void updateInputParameters()
{
    lfo.setLoopTime(inputParameters[parLoopTime]);
    lfo.setFrequency(inputParameters[parFrequency]);
    lfo.setSeed(inputParameters[parSeed]);

    lfo.setOscillatorAmplitude(0, inputParameters[parLFO1Level]);
    lfo.setOscillatorAmplitude(1, inputParameters[parLFO2Level]);
    lfo.setOscillatorAmplitude(2, inputParameters[parLFO3Level]);
    lfo.setOscillatorAmplitude(3, inputParameters[parLFO4Level]);
    
    lfo.setDisableSin(inputParameters[parSinOff] != 0 ? true : false);
    lfo.setDisableTri(inputParameters[parTriOff] != 0 ? true : false);
    lfo.setDisableSaw(inputParameters[parRampOff] != 0 ? true : false);
    lfo.setDisableRamp(inputParameters[parSawOff] != 0 ? true : false);
    lfo.setDisableSquare(inputParameters[parSquareOff] != 0 ? true : false);

    oscillator.setMorph(inputParameters[parOscMorph]);

    inGain = inputParameters[parOscAmp];

    oscillator.setFrequency(pitchToFrequency(inputParameters[parOscPitch]));

    isRandomizing = inputParameters[parRandomizeWaveform] > 0;
    if (isRandomizing)
        lfo.randomizePattern();
}

void processBlock(BlockData& data)
{
	 for(uint i = 0; i < data.samplesToProcess; i++)
    {       
        double lfoSignal = 0;
        if (!isRandomizing)
		    lfoSignal = lfo.getSample() * inputParameters[parLFOAllLevel];

        oscillator.setMorph(clamp(lfoSignal, 0, 1));
		double output = oscillator.getSample() * inGain;  

        double filter_p2f = pitchToFrequency(inputParameters[parFilterPitch] + lfoSignal * 10);      

        filter.setFrequency(filter_p2f);

		output = filter.getSample(output);

        for(uint ch = 0; ch < audioOutputsCount; ++ch)
             data.samples[ch][i] = output;
    }
}