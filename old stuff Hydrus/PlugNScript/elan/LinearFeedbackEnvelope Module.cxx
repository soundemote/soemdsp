#include "library/Envelopes/LinearFeedbackEnvelope.cxx"
#include "library/Oscillators/SineSaw.cxx"
#include "library/Oscillators/MusicalSaw.cxx"
#include "library/Oscillators/PolyBLEP.cxx"
#include "library/Filters/Lowpass.cxx"
#include "library/Parameter.cxx"
#include "../library/Midi.hxx"

array<Parameter> parameters = 
{
    Parameter("Delay",   .0001, 1, 0.1),
    Parameter("Attack",   .0001, 1, 0.1),

    Parameter("Decay",    .0001, 10, 0.1),

    Parameter("Sustain",     0, 1, 0),
    Parameter("Release",     0, 1, 0),

    Parameter("AttackMod",   .0001, 1, 0.1),
    Parameter("DecayMod",     0, 10, 0),
    Parameter("ReleaseMod",     0, 1, 0),

    Parameter("AttackShape",   0, 4, 2, "", "", 5, "EXP;POW3;LOG;SIGMOID;LIN"),
    Parameter("DecayShape",   0, 4, 0, "", "", 5, "EXP;POW3;LOG;SIGMOID;LIN"),
    Parameter("ReleaseShape",   0, 4, 0, "", "", 5, "EXP;POW3;LOG;SIGMOID;LIN"),

    Parameter("GlobalSpeed",   .0001, 100, 0.1),
    Parameter("GlobalMod",   .0001, 10, 0.1),

    Parameter("isLooping",   0, 1, 0, "", "", 2, "not looping;is looping!"),
    Parameter("isOneShot",   0, 1, 0, "", "", 2, "not one shot;is one shot!"),
};

uint parDelay = 0;
uint parAttack = 1;
uint parDecay = 2;
uint parSustain = 3;
uint parRelease = 4;

uint parAttackMod = 5;
uint parDecayMod = 6;
uint parReleaseMod = 7;

uint parAttackShape = 8;
uint parDecayShape = 9;
uint parReleaseShape = 10;

uint parGlobalSpeed = 11;
uint parGlobalMod = 12;

uint parIsLooping = 13;
uint parIsOneShot = 14;

array<double> inputParameters(parameters.length);
array<string> inputParametersNames = {};
array<double> inputParametersMin = {};
array<double> inputParametersMax = {};
array<double> inputParametersDefault = {};
array<string> inputParametersUnits = {};
array<string> inputParametersFormats = {};
array<string> inputParametersEnums = {};
array<int>    inputParametersSteps = {};

string name = "Linear Feedback Envelope";
string description = "Basic analog-style envelope.";

LinearFeedbackEnvelope envelope;
MusicalSaw oscillator;
Lowpass lpf, lpf2, lpf3;

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
    envelope.setNoiseSeed(5264);

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
    envelope.setDelayTime(inputParameters[parDelay]);
    envelope.setAttackTime(inputParameters[parAttack]);
    envelope.setDecayTime(inputParameters[parDecay]);
    envelope.setSustainAmplitude(inputParameters[parSustain]);
    envelope.setReleaseTime(inputParameters[parRelease]);
    envelope.setAttackFeedback(inputParameters[parAttackMod]);
    envelope.setDecayFeedback(inputParameters[parDecayMod]);
    envelope.setReleaseFeedback(inputParameters[parReleaseMod]);
    envelope.setAttackShape(roundToInt(inputParameters[parAttackShape]));
    envelope.setDecayShape(roundToInt(inputParameters[parDecayShape]));
    envelope.setReleaseShape(roundToInt(inputParameters[parReleaseShape]));
    envelope.setGlobalTime(inputParameters[parGlobalSpeed]);
    envelope.setGlobalFeedback(inputParameters[parGlobalMod]);

    envelope.setDoLoop(inputParameters[parIsLooping] == 0 ? false : true);
    envelope.setDoOneShot(inputParameters[parIsOneShot] == 0 ? false : true);
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

        output = envOut = envelope.getSample();

        double f = pitchToFrequency(map(envOut, -20, 130));
        lpf.setFrequency(f);
        lpf2.setFrequency(f);
        lpf3.setFrequency(f);

        oscillator.setMorph(clamp(envOut, 0, 1));
        output = oscillator.getSample();
        output = lpf.getSample(output);
        //output = lpf2.getSample(output);
        //output = lpf3.getSample(output);

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
