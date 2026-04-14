#include "library/Envelopes/LinearFeedbackPluckEnvelope.cxx"
#include "library/Oscillators/SineSaw.cxx"
#include "library/Oscillators/MusicalSaw.cxx"
#include "library/Oscillators/PolyBLEP.cxx"
#include "library/Filters/Lowpass.cxx"
#include "library/Parameter.cxx"
#include "../library/Midi.hxx"

array<Parameter> parameters = 
{
    Parameter("Decay", .1, 1, .3),
    Parameter("Release", 0, 1, 0),
    Parameter("AttackEnergy", .001, 1.8, .7),
    Parameter("DecayEnergy", .01, 3, 1.05), 
    Parameter("Sustain", 0, 1.4, 1.05),
    Parameter("EnvelopeFrequency", 0, 100, 20),
    Parameter("EnvelopeCurve", -1, 1, 0)
};

uint parDecay = 0;
uint parRelease = 1;
uint parAttackEnergy = 2;
uint parDecayEnergy = 3;
uint parSustain = 4;
uint parEnvelopeFrequency = 5;
uint parEnvelopeCurve = 6;

array<double> inputParameters(parameters.length);
array<string> inputParametersNames = {};
array<double> inputParametersMin = {};
array<double> inputParametersMax = {};
array<double> inputParametersDefault = {};
array<string> inputParametersUnits = {};
array<string> inputParametersFormats = {};
array<string> inputParametersEnums = {};
array<int>    inputParametersSteps = {};

string name = "Linear Feedback Pluck Envelope";
string description = "Envelope optimized for creating pluck sounds.";

LinearFeedbackEnvelope envelope;
MusicalSaw oscillator;
//PolyBLEP oscillator;
Lowpass lpf,lpf2,lpf3;

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
    //oscillator.setWaveform(5);
    
    lpf.setSampleRate(sampleRate);
    lpf2.setSampleRate(sampleRate);
    lpf3.setSampleRate(sampleRate);
}

void updateInputParameters()
{
    envelope.setDecay(inputParameters[parDecay]);
    envelope.setRelease(inputParameters[parRelease]);
    envelope.setDecayModStart(inputParameters[parAttackEnergy]);
    envelope.setDecayModEnd(inputParameters[parDecayEnergy]);
    envelope.setFinalDecay(inputParameters[parSustain]);
    envelope.setDecayModFrequency(inputParameters[parEnvelopeFrequency]);
    envelope.setDecayModCurve(inputParameters[parEnvelopeCurve]);
}

double currentAmplitude = 0;
double currentPhase = 0;
double currentPitchOffset= 0;
uint8  currentNote=0;
double AdAmplitude = 0;
double AttackDecay = 0;
double amplitude = 0;
double output = 0;
bool doReset = false;
bool isMono = false;
double pitchToFreq = 0;
MidiEvent tempEvent;
double velocityNormalized = 0;
double noteIsOn = 0;

void handleMidiEvent(const MidiEvent& evt)
{
    switch(MidiEventUtils::getType(evt))
    {
    case kMidiNoteOn:
        {
            noteIsOn = 1; 

            velocityNormalized = amplitude = double(MidiEventUtils::getNoteVelocity(evt))/127.0;

            currentNote = MidiEventUtils::getNote(evt);

            oscillator.reset();
            oscillator.setFrequency(pitchToFrequency(currentNote));

            envelope.triggerAttack(velocityNormalized);
            break;
        }
    case kMidiNoteOff:
        {
            if(currentNote==MidiEventUtils::getNote(evt))
            {
                amplitude=0;
                noteIsOn = 0;
                envelope.triggerRelease();
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
        double output, envOut;

        envOut = envelope.getSample();

        double f = pitchToFrequency(map(envOut, -20, 130));
        lpf.setFrequency(f);
        // lpf2.setFrequency(f);
        // lpf3.setFrequency(f);

        oscillator.setMorph(clamp(envOut, 0, 1));
        output = oscillator.getSample();
        output = lpf.getSample(output);
        // output = lpf2.getSample(output);
        // output = lpf3.getSample(output);

        if (audioOutputsCount >= 2)
        {
            data.samples[0][i] = output;
            data.samples[1][i] = output;
        }
        else
        {
            for(uint ch = 0; ch < audioOutputsCount; ++ch)
                data.samples[ch][i] = output;
        }
    }
}

int getTailSize()
{
    return -1;
}
