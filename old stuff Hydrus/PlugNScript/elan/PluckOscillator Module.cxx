#include "../library/Midi.hxx"
#include "library/Parameter.cxx"
#include "library/Oscillators/PluckOscillator.cxx"

array<Parameter> parameters = 
{
    Parameter("decayMultiplier", 0, 1, .5),
    Parameter("Morph", 0, 1, .5),
};

array<double> inputParameters(parameters.length);
array<string> inputParametersNames = {};
array<double> inputParametersMin = {};
array<double> inputParametersMax = {};
array<double> inputParametersDefault = {};
array<string> inputParametersUnits = {};
array<string> inputParametersFormats = {};
array<string> inputParametersEnums = {};
array<int>    inputParametersSteps = {};

string name = "Pluck oscillator";
string description = "An oscillator/envelope hybrid.";

PluckOscillator pluckOsc;

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

    pluckOsc.setSampleRate(sampleRate);
}

void updateInputParameters()
{   
    osc.setMorph(currentAmplitude);
    pluckOsc.amplitudeMultiplier = inputParameters[0];


}

double currentAmplitude=0;
double currentPhase=0;
double currentPitchOffset=0;
uint8  currentNote=0;
const double period=2*PI;
double AdAmplitude = 0;
double AttackDecay = 0;
double amplitude = 0;
double output = 0;
bool doReset = false;
bool isMono = false;
double pitchToFreq = 0;
MidiEvent tempEvent;

double noteIsOn = 0;

void handleMidiEvent(const MidiEvent& evt)
{
    switch(MidiEventUtils::getType(evt))
    {
    case kMidiNoteOn:
        {
            amplitude = double(MidiEventUtils::getNoteVelocity(evt))/127.0;
            currentNote = MidiEventUtils::getNote(evt);
            pluckOsc.triggerAttack();
            pluckOsc.setFrequency(pitchToFrequency(currentNote));

            print("currentNote:"+currentNote);

            noteIsOn = 1;
            break;
        }
    case kMidiNoteOff:
        {
            if(currentNote==MidiEventUtils::getNote(evt))
            {
                amplitude=0;   
                noteIsOn = 0;
            }          
            break;
        }
    case kMidiPitchWheel:
        {
            currentPitchOffset=2*double(MidiEventUtils::getPitchWheelValue(evt))/8192.0;
            break;
        }
    }
}

void processBlock(BlockData& data)
{
    uint nextEventIndex=0;

    MidiEventUtils::setType(tempEvent, kMidiPitchWheel);

    // manage MIDI events
    while(nextEventIndex != data.inputMidiEvents.length)
    {            
        handleMidiEvent(data.inputMidiEvents[nextEventIndex]);
        nextEventIndex++;
    }

    for(uint i = 0; i < data.samplesToProcess; i++)
    {               
        double envOut = pluckOsc.getSample();

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
