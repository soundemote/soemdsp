#include "library/math.cxx"
#include "library/Oscillators/PolyBLEP.cxx"

array<double> inputParameters(8);
array<string> inputParametersNames = {"Frequency", "Morph", "Amplitude", "Phase Offset", "Reset", "Waveform", "PM Feedback", "FM Feedback"};
array<double> inputParametersMin =     {         -2500, 0,  0, -1, 0,  0, -.5, -1};
array<double> inputParametersMax =     { sampleRate*.2, 1,  1,  1, 1, 13,  .5,  1};
array<double> inputParametersDefault = {  440, 0, .5,  0, 0,  0,   0,  0};
array<string> inputParametersUnits = {""};
array<string> inputParametersFormats = {".3",".3",".3", ".3", "", ""};
array<string> inputParametersEnums={"", "", "", "", "playing;resetting", "SINE;COSINE;TRIANGLE;SQUARE;RECTANGLE;SAWTOOTH;RAMP;MODIFIED_TRIANGLE;MODIFIED_SQUARE;HALF_WAVE_RECTIFIED_SINE;FULL_WAVE_RECTIFIED_SINE;TRIANGULAR_PULSE;TRAPEZOID_FIXED;TRAPEZOID_VARIABLE"};
array<int> inputParametersSteps={0, 0, 0, 0, 2, 14};

string name="PolyBLEP oscillators";
string description="Polynomial Band Limited / antialiased oscillators";

PolyBLEP oscillator;

void initialize()
{
    oscillator.setSampleRate(sampleRate);
}

double amplitude = 0;
double output = 0;
bool doReset = false;

void updateInputParameters()
{
    oscillator.setMorph(inputParameters[1]);
    amplitude = inputParameters[2];
    oscillator.setPhaseOffset(inputParameters[3]);
    doReset = inputParameters[4] != 0.0 ? true : false;
    oscillator.setWaveform(roundToInt(inputParameters[5]));
}

void processBlock(BlockData& data)
{
}

void processSample(array<double>& ioSample)
{
    oscillator.setPhaseOffset(output * inputParameters[6] + inputParameters[3]);

    double f = inputParameters[0];
    double fFb = inputParameters[7];
    oscillator.setFrequency(f + output * map(fFb, -1, 1, -f*2, f*2));

    if (doReset)
        oscillator.reset();
    else
        output = oscillator.getSample();

    for(uint ch = 0; ch < audioOutputsCount; ++ch)
        ioSample[ch] = output * amplitude;
}

int getTailSize()
{
    return -1;
}
