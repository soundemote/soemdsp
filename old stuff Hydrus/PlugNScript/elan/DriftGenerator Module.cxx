#include "library/Randoms/DriftGenerator.cxx"
#include "../library/Midi.hxx"

array<double> inputParameters(2);
array<string> inputParametersNames = {"Color", "Amplitude"};
array<double> inputParametersMin = {0, 0};
array<double> inputParametersMax = {20, 1};
array<double> inputParametersDefault = {150, 1};
array<string> inputParametersUnits = {"Hz", ""};
array<string> inputParametersFormats={".2", ".2"};

string name = "Drift Generator";
string description = "slowly evolving random value generator for organic drift";

DriftGenerator driftGenerator;

void initialize()
{
	driftGenerator.setSampleRate(sampleRate);
}

int getTailSize()
{
	return -1;
}

void updateInputParameters()
{
	driftGenerator.setColor(inputParameters[0]);
	driftGenerator.setAmplitude(inputParameters[1]);
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

void handleMidiEvent(const MidiEvent& evt)
{
    switch(MidiEventUtils::getType(evt))
    {
    case kMidiNoteOn:
        {
            amplitude = double(MidiEventUtils::getNoteVelocity(evt))/127.0;
            currentNote = MidiEventUtils::getNote(evt);
            break;
        }
    case kMidiNoteOff:
        {
            if(currentNote==MidiEventUtils::getNote(evt))
                amplitude=0;             
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
    for(uint i = 0; i < data.samplesToProcess; i++)
    {       
        double output = driftGenerator.getSample();

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
