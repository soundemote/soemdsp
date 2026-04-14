#include "library/math.cxx"
#include "library/Oscillators/AdditiveSupersaw.cxx"
#include "library/Parameter.cxx"
#include "../library/Midi.hxx"
#include "library/Envelopes/LinearFeedbackPluckEnvelope.cxx"

array<Parameter> parameters = 
{
    Parameter("PitchOffset", -50, 50, 0),
    Parameter("Amplitude", 0, 1, .5),
    Parameter("NumHarmonics", 0, 1000, 5),
    Parameter("DriftColor", 0, 10, 0),
    Parameter("DriftDetail", 0, 10, 0),
    Parameter("DriftCurve", -1, 1, 0),
    Parameter("DriftAmp", 0, 10, 0),
    Parameter("Dispersion", 0, 1000, 0),
    // Parameter("frequencyOffset", -000, 10000, 0),
    // Parameter("frequencyOffsetCurve", -1, 1, 0),
    Parameter("frequencyOffsetAmp", 0, 1, 0),

    Parameter("Decay", .1, 1, .3),
    Parameter("Release", 0, 1, 0),
    Parameter("AttackEnergy", .001, 1.8, .7),
    Parameter("DecayEnergy", .01, 3, 1.05), 
    Parameter("Sustain", 0, 1.4, 1.05),
    Parameter("EnvelopeFrequency", 0, 100, 20),
    Parameter("EnvelopeCurve", -1, 1, 0)
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

string name = "Additive Supersaw";
string description = "An attempt to make a supersaw out of a single sawtooth.";

AdditiveSupersaw aS_L;
AdditiveSupersaw aS_R;
LinearFeedbackEnvelope envelope;

void initialize()
{
    for (uint i = 0; i < parameters.length; ++i)
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

    makeSinWavetable();
    envelope.setSampleRate(sampleRate);
    aS_L.setSampleRate(sampleRate);
    aS_R.setSampleRate(sampleRate);    
}

int parPitchOffset = 0;
int parAmplitude = 1;
int parNumHarmonics = 2;
int parDriftColor = 3;
int parDriftDetail = 4;
int parDriftCurve = 5;
int parDriftAmp = 6;
int parDispersion = 7;
int parFrequencyOffsetAmp = 8;

uint parDecay = 9;
uint parRelease = 10;
uint parAttackEnergy = 11;
uint parDecayEnergy = 12;
uint parSustain = 13;
uint parEnvelopeFrequency = 14;
uint parEnvelopeCurve = 15;

void updateInputParameters()
{
    pitchOffset = inputParameters[parPitchOffset];
    amplitude   = inputParameters[parAmplitude];

    if (numHarmonics != int(inputParameters[parNumHarmonics]))
    {
        aS_L.setNumHarmonics(numHarmonics = int(inputParameters[parNumHarmonics]));
        //aS_R.setNumHarmonics(numHarmonics = int(inputParameters[parNumHarmonics]));
    }

    if (driftColor != inputParameters[parDriftColor])
    {
        aS_L.driftModulator.setDriftColor(driftColor = inputParameters[parDriftColor]);
        //aS_R.driftModulator.setDriftColor(driftColor = inputParameters[parDriftColor]);
    }

    if (driftDetail != inputParameters[parDriftDetail])
    {
        aS_L.driftModulator.setDriftDetail(driftDetail = inputParameters[parDriftDetail]);
        //aS_R.driftModulator.setDriftDetail(driftDetail = inputParameters[parDriftDetail]);
    }

    if (driftCurve != inputParameters[parDriftCurve])
    {
        aS_L.driftModulator.setDriftCurve(driftCurve = inputParameters[parDriftCurve]);
        //aS_R.driftModulator.setDriftCurve(driftCurve = inputParameters[parDriftCurve]);
    }

    if (driftAmp != inputParameters[parDriftAmp])
    {
        aS_L.driftModulator.setDriftAmplitude(driftAmp = inputParameters[parDriftAmp]);
        //aS_R.driftModulator.setDriftAmplitude(driftAmp = inputParameters[parDriftAmp]);
    }

    if (dispersion != inputParameters[parDispersion])
    {
        aS_L.setDispersion(dispersion = inputParameters[parDispersion]);
        //aS_R.setDispersion(dispersion = inputParameters[parDispersion]);
    }

    if (frequencyOffsetAmp != inputParameters[parFrequencyOffsetAmp])
    {
        aS_L.frequencyOffsetAmp = (frequencyOffsetAmp = inputParameters[parFrequencyOffsetAmp]);
        //aS_R.frequencyOffsetAmp = (frequencyOffsetAmp = inputParameters[parFrequencyOffsetAmp]);
    }

    envelope.setDecay(inputParameters[parDecay]);
    envelope.setRelease(inputParameters[parRelease]);
    envelope.setDecayModStart(inputParameters[parAttackEnergy]);
    envelope.setDecayModEnd(inputParameters[parDecayEnergy]);
    envelope.setFinalDecay(inputParameters[parSustain]);
    envelope.setDecayModFrequency(inputParameters[parEnvelopeFrequency]);
    envelope.setDecayModCurve(inputParameters[parEnvelopeCurve]);
}

double pitchOffset = 0;
double frequency = 0;
double amplitude = 0;
int numHarmonics = 0;
double driftColor = 0;
double driftDetail = 0;
double driftCurve = 0;
double driftAmp = 0;
double signal = 0;
double dispersion = 0;
double frequencyOffsetAmp = 0;

double currentAmplitude = 0;
double currentPhase = 0;
double currentPitchOffset = 0;
uint8  currentNote = 0;
MidiEvent tempEvent;
double noteIsOn = 0;

double attackVal = 0;
double attackInc = .01;

void handleMidiEvent(const MidiEvent& evt)
{
    switch(MidiEventUtils::getType(evt))
    {
    case kMidiNoteOn:
        {
            currentAmplitude = double(MidiEventUtils::getNoteVelocity(evt))/127.0;
            currentNote = MidiEventUtils::getNote(evt);
            noteIsOn = 1;

            // for (int i = 0; i < aS_L.numHarmonics; ++i)
            // {
            //     aS_L.harmonics[i].reset();
            //     //aS_R.harmonics[i].reset();
            // }
                
            frequency = pitchToFrequency(currentNote + pitchOffset);
            aS_L.setFrequency(frequency);
            //aS_R.setFrequency(frequency);

            envelope.triggerAttack(1);
            attackVal = 0;

            break;
        }
    case kMidiNoteOff:
        {
            if(currentNote==MidiEventUtils::getNote(evt))
            {
                currentAmplitude = 0;   
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

    for (uint i = 0; i < data.samplesToProcess; i++)
    {    
        double signalL = 0;
        double signalR = 0;

        if (!envelope.isIdle())
        {
            aS_L.getSample(signalL, signalR);
            //signalR = aS_R.getSample();    
            //double envSignal = envelope.getSample();
            //signalL *= envSignal * attackVal;
            //signalR *= envSignal * attackVal;

            //if (attackVal < 1)
                //attackVal += attackInc;
            
        }

        if (audioOutputsCount >= 2)
        {
            data.samples[0][i] = signalL * amplitude;
            data.samples[1][i] = signalR * amplitude;
        }
        else
        {
            for(uint ch = 0; ch < audioOutputsCount; ++ch)
                data.samples[ch][i] = (signalL+signalR)*amplitude;
        }
    }
}

int getTailSize()
{
    return -1;
}
