/*
Phase changes at a frequency equal to that of the oscillator is the sweet spot for the most modulation without crackle or other unwanted artifacts
*/

#include "../library/Midi.hxx"
#include "library/MultipointGraph.cxx"
#include "library/Oscillators/Phasor.cxx"
#include "library/Parameter.cxx"

array<Parameter> parameters =
{    
    Parameter("Frequency", 0, 10, 1),
    Parameter("Amplitude", 0, 1, .5),
};

int parFrequency = 0;
int parAmplitude = 1;

array<double> inputParameters(parameters.length-1);
array<string> inputParametersNames = {};
array<double> inputParametersMin = {};
array<double> inputParametersMax = {};
array<double> inputParametersDefault = {};
array<string> inputParametersUnits = {};
array<string> inputParametersFormats = {};
array<string> inputParametersEnums = {};
array<int>    inputParametersSteps = {};

string name = "Multipoint Graph";
string description = "Testing if multipoint graph is working";

MultipointGraph graph;
Phasor phasor;

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

    graph.addNode(0,  0, .0);
    graph.addNode(.5, .5, .5);
    graph.addNode(1, 1, -.5);
}

void updateInputParameters()
{
    phasor.setFrequency(inputParameters[parFrequency]);
    amplitude = inputParameters[parAmplitude];
}

double amplitude = 0;
double midiAmplitude = 0;
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
            midiAmplitude = double(MidiEventUtils::getNoteVelocity(evt))/127.0;        
            currentNote = MidiEventUtils::getNote(evt);
            break;
        }
    case kMidiNoteOff:
        {
            if(currentNote == MidiEventUtils::getNote(evt))
                midiAmplitude = 0.0;             
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

        output = graph.getValue(phasor.getSample());

        for(uint ch = 0; ch < audioOutputsCount; ++ch)
            data.samples[ch][i] = output * amplitude * midiAmplitude;
    }
}

int getTailSize()
{
    return -1;
}
