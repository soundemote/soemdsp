#include "library/Envelopes/PluckEnvelope.cxx"
#include "library/Oscillators/SineSaw.cxx"
#include "library/Filters/Lowpass.cxx"
#include "library/Parameter.cxx"

array<Parameter> parameters = 
{
    Parameter("Attack", 0.1, 5, 0.1),
    Parameter("AttackRatio", 0.1, 1, 0.1),
    Parameter("AttackMix", 0, 1, 0.1),
    Parameter("Decay", 0, 1, 0.1),
    Parameter("DecayRatio", 0.1,  1, 0.1),
    Parameter("DecayMix", 0, 1, 0.1),
    Parameter("Length", 1.e-4, 1, 0.1),
    Parameter("LengthModifier", 0, .0001, 0, "", ".6"),
    Parameter("Loop Time", 1.e-4, 100, 0.1),
};

array<double> inputParameters(parameters.length-1);
array<string> inputParametersNames = {};
array<double> inputParametersMin = {};
array<double> inputParametersMax = {};
array<double> inputParametersDefault = {};
array<string> inputParametersUnits = {};
array<string> inputParametersFormats = {};
array<string> inputParametersEnums = {};
array<int>    inputParametersSteps = {};

string name = "Pluck envelope";
string description = "A more natural envelope for pluck sounds.";

PluckEnvelope envelope;
SineSaw oscillator;
Lowpass lpf;

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

    envelope.setSampleRate(sampleRate);
    oscillator.setSampleRate(sampleRate);
    oscillator.setFrequency(220);
    oscillator.setMorph(1);
    lpf.setSampleRate(sampleRate);
}

void updateInputParameters()
{
    envelope.setAttack(inputParameters[0]);
    envelope.setAttackRatio(inputParameters[1]);
    envelope.setAttackMix(inputParameters[2]);

    double decay = inputParameters[3];
    decay = rationalCurve(decay, -.99);
    decay = map(decay, 1, 2000);

    envelope.setDecay(decay);
    envelope.setDecayRatio(inputParameters[4]);
    envelope.setDecayMix(inputParameters[5]);
    envelope.setSeconds(inputParameters[6]);
    envelope.setLengthModifier(inputParameters[7]);
    envelope.setLoopTime(inputParameters[8]);
}

void processBlock(BlockData& data)
{
    for(uint i = 0; i < data.samplesToProcess; i++)
    {       
        double envOut = envelope.getSample();
        lpf.setFrequency(envOut*10000);
        oscillator.setMorph(clamp(envOut, 0, 1));
        double oscOut = lpf.getSample(oscillator.getSample());

        if (audioOutputsCount >= 2)
        {
            data.samples[0][i] = envOut;
            data.samples[1][i] = envOut;
        }
        else
        {
            for(uint ch = 0; ch < audioOutputsCount; ++ch)
                data.samples[ch][i] = envOut;
        }
    }
}

int getTailSize()
{
    return -1;
}
