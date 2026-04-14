#include "library/Envelopes/AttackDecayFilter.cxx"
#include "library/Oscillators/SineSaw.cxx"
#include "library/Filters/Lowpass.cxx"
#include "library/Parameter.cxx"
#include "../library/Midi.hxx"

array<Parameter> parameters = 
{
    Parameter("Attack",   .0001, 1, 0.1),
    Parameter("Decay",    .0001, 1, 0.1),
    Parameter("DecayMod",     0, 10, 0),
    Parameter("SmoothingFrequency",     0, 1000, 0),
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

AttackDecayFilter envelope;
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

    oscillator.setSampleRate(sampleRate);
    oscillator.setFrequency(220);
    oscillator.setMorph(1);
    lpf.setSampleRate(sampleRate);
}

void updateInputParameters()
{
    int attackSamples = roundToInt(sampleRate / inputParameters[0]);
    int decaySamples  = roundToInt(sampleRate / inputParameters[1]);

    envelope.setAttackSamples(attackSamples);
    envelope.setDecaySamples(decaySamples);
    // envelope.setDecayModIncrement(inputParameters[2]);
    // envelope.smoother.setFrequency(inputParameters[3]);
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
            oscillator.setFrequency(pitchToFrequency(currentNote));
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
        double envOut = envelope.getSample(noteIsOn) * 0.0000099;
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
