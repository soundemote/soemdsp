#include "library/math.cxx"
#include "library/Oscillators/MusicalSaw.cxx"

array<double> inputParameters(5);
array<string> inputParametersNames = {"Pitch", "Morph", "Amplitude", "Phase", "Reset"};
array<double> inputParametersMin = {0, 0, 0, -3, 0};
array<double> inputParametersMax = {127, 1, 1, 3, 1};
array<double> inputParametersDefault = {60, .75, .5,0, 0};
array<string> inputParametersUnits = {""};
array<string> inputParametersFormats = {".2",".2",".2",".2"};
array<string> inputParametersEnums={"", "", "", "", "playing;resetting"};
array<int> inputParametersSteps={0, 0, 0, 0, 2};

string name="Musical Saw";
string description="Sawtooth oscillator with a pseudo-filter morph";

MusicalSaw oscillator;

void initialize()
{
    oscillator.setSampleRate(sampleRate);
}

double amplitude = 0;
double output = 0;
bool doReset = false;

void updateInputParameters()
{
    oscillator.setFrequency(pitchToFrequency(inputParameters[0]));
    oscillator.setMorph(inputParameters[1]);
    oscillator.setPhaseOffset(inputParameters[3]);
    amplitude = inputParameters[2];
    doReset = inputParameters[4] != 0.0 ? true : false;
}

void processBlock(BlockData& data)
{
    for(uint i = 0; i < data.samplesToProcess; i++)
    {
        if (doReset)
            oscillator.reset();
        else
            output = oscillator.getSample();

        for(uint ch = 0; ch < audioOutputsCount; ++ch)
            data.samples[ch][i] = output * amplitude;
    }
}

int getTailSize()
{
    return -1;
}
