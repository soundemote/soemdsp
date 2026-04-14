// Extended DSF Algorithms by Walter H. Hackett IV
// - Engineer's Square to Sine 

#include "library/math.cxx"
#include "library/Oscillators/SineSquare.cxx"

array<double> inputParameters(5);
array<string> inputParametersNames = {"Frequency", "Morph", "Amplitude", "PhaseOffset", "Reset"};
array<double> inputParametersMin = {0, 0, 0, -1, 0};
array<double> inputParametersMax = {1270, 1, 1, 1, 1};
array<double> inputParametersDefault = {30, 0, .5, 0, 0};
array<string> inputParametersUnits = {""};
array<string> inputParametersFormats = {".3",".3",".3", ".3"};
array<string> inputParametersEnums={"", "", "", "", "playing;resetting"};
array<int> inputParametersSteps={0, 0, 0, 0, 2};

string name="Engineer's Square to Sine";
string description="oscillator that can morph from sine to square";

SineSquare oscillator;

void initialize()
{
    oscillator.setSampleRate(sampleRate);
}

double amplitude = 0;
bool doReset = false;

void updateInputParameters()
{
    oscillator.setFrequency(inputParameters[0]);
    oscillator.setMorph(inputParameters[1]);
    amplitude = inputParameters[2];
    oscillator.setPhaseOffset(inputParameters[3]);
    doReset = inputParameters[4] != 0.0 ? true : false;
}

void processBlock(BlockData& data)
{
}

void processSample(array<double>& ioSample)
{
    double output = 0;

    if (doReset)
        oscillator.reset();
    else
        output = oscillator.getSample() * amplitude;

    for(uint ch = 0; ch < audioOutputsCount; ++ch)
        ioSample[ch] = output;
}

int getTailSize()
{
    return -1;
}
