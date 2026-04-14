/*
Phase changes at a frequency equal to that of the oscillator is the sweet spot for the most modulation without crackle or other unwanted artifacts
*/

#include "library/math.cxx"
#include "library/Oscillators/NoiseSaw.cxx"
#include "library/Parameter.cxx"
#include "../library/Midi.hxx"

array<Parameter> parameters =
{    
    Parameter("Osc_Morph",     0, 1,   .5),
    Parameter("Noise_Pitch", -50, 147, 0),
    Parameter("SH_Smooth_Pitch", -50, 147, 0),
    Parameter("HP_Frequency",    -50, 147,  0),
    Parameter("LP_Frequency",    -50, 147,  0),
    Parameter("RW_Color",        0,   1, 0.5),
    Parameter("RW_Detail",     -50, 147, 0),
    Parameter("Noise_AmpPM",     0,   1, 0),
    Parameter("Smooth_AmpPM",    0,   1, 0),
    Parameter("Walk_AmpPM",    .001, 10, 0),
    Parameter("Noise_AmpFM",     0,   1,  0),
    Parameter("Smooth_AmpFM",     0,  1,  0),
    Parameter("Walk_AmpFM",       0,  1,  0),
};

uint parOsc_Morph = 0;
uint parNoise_Pitch = 1;
uint parSH_Smooth_Pitch = 2;
uint parHP_Frequency = 3;
uint parLP_Frequency = 4;
uint parRW_Color = 5;
uint parRW_Detail = 6;
uint parNoise_AmpPM = 7;
uint parSmooth_AmpPM = 8;
uint parWalk_AmpPM = 9;
uint parNoise_AmpFM = 10;
uint parSmooth_AmpFM = 11;
uint parWalk_AmpFM = 12;

array<double> inputParameters(parameters.length-1);
array<string> inputParametersNames = {};
array<double> inputParametersMin = {};
array<double> inputParametersMax = {};
array<double> inputParametersDefault = {};
array<string> inputParametersUnits = {};
array<string> inputParametersFormats = {};
array<string> inputParametersEnums = {};
array<int>    inputParametersSteps = {};

string name = "Noise saw";
string description = "Trying to find a better random source for the hypersaw";

NoiseSaw osc;

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

    osc.setSampleRate(sampleRate);
}

void updateInputParameters()
{
    osc.setMorph(inputParameters[parOsc_Morph]);
    osc.setNoiseFilterFrequency(pitchToFrequency(inputParameters[parNoise_Pitch]));
    osc.setSmoothFrequency(pitchToFrequency(inputParameters[parSH_Smooth_Pitch]));

    osc.setHighpassFrequency(pitchToFrequency(inputParameters[parHP_Frequency]));
    osc.setLowpassFrequency(pitchToFrequency(inputParameters[parLP_Frequency]));
    osc.walk.setColor(inputParameters[parRW_Color]);
    osc.walk.setDetail(pitchToFrequency(inputParameters[parRW_Detail]));

    osc.setNoiseAmplitudePM(inputParameters[parNoise_AmpPM]);
    osc.setSmoothAmplitudePM(inputParameters[parSmooth_AmpPM]);
    osc.setWalkAmplitudePM(inputParameters[parWalk_AmpPM]);

    osc.setNoiseAmplitudeFM(inputParameters[parNoise_AmpFM]);
    osc.setSmoothAmplitudeFM(inputParameters[parSmooth_AmpFM]);
    osc.setWalkAmplitudeFM(inputParameters[parWalk_AmpFM]);
}

double amplitude = 0;
double output;

MidiEvent tempEvent;
uint8 currentNote;
double currentPitchOffset;

void handleMidiEvent(const MidiEvent& evt)
{
    switch(MidiEventUtils::getType(evt))
    {
    case kMidiNoteOn:
        {
            amplitude = double(MidiEventUtils::getNoteVelocity(evt))/127.0;
        
            currentNote = MidiEventUtils::getNote(evt);
            osc.setFrequency(pitchToFrequency(currentNote));
            break;
        }
    case kMidiNoteOff:
        {
            if(currentNote == MidiEventUtils::getNote(evt))
                amplitude = 0.0;             
            break;
        }
    case kMidiPitchWheel:
        {
            currentPitchOffset = 2 * double(MidiEventUtils::getPitchWheelValue(evt))/8192.0;
            break;
        }
    }
}

void processBlock(BlockData& data)
{
    uint nextEventIndex = 0;

    MidiEventUtils::setType(tempEvent, kMidiPitchWheel);
    for(uint i = 0; i < data.samplesToProcess; i++)
    {
        while(nextEventIndex != data.inputMidiEvents.length)
        {
            
            handleMidiEvent(data.inputMidiEvents[nextEventIndex]);
            nextEventIndex++;
        }

        output = osc.getSample();
        //output = osc.noiseSignal;
        //output = osc.smoothSignal;
        //output = osc.walkSignal;

        for(uint ch = 0; ch < audioOutputsCount; ++ch)
            data.samples[ch][i] = output * amplitude;
    }
    
}

int getTailSize()
{
    return -1;
}
