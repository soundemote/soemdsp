#include "library/Oscillators/SineSaw.cxx"
#include "library/Oscillators/MusicalSaw.cxx"
#include "library/Filters/Lowpass.cxx"
#include "library/Parameter.cxx"
#include "library/NoteTracker.cxx"
#include "library/Dynamics/Rotator3D.cxx"
#include "library/Oscillators/ChaosGenerators.cxx"
#include "../library/Midi.hxx"


SineSaw oscillator;
Lorentz chaos;
NoteTracker noteTracker;
Rotator3D rotator;

double amplitude = 0;
uint8  currentNote=0;
MidiEvent tempEvent;


array<Parameter> parameters = 
{
	Parameter("Chaos Freq", 0, 100, .1),
	Parameter("X Amp Output", -1, 1, 0),
    Parameter("Y Amp Output", -1, 1, 0),
    Parameter("Z Amp Output", -1, 1, 0),
    Parameter("X Note Output", -127, 127, 0),
    Parameter("Y Note Output", -127, 127, 0),
    Parameter("Z Note Output", -127, 127, 0),
	Parameter("Rotate X", 0, 1, 0),
	Parameter("Rotate Y", 0, 1, 0),
	Parameter("Rotate Z", 0, 1, 0),
    Parameter("Osc Morph", 0, 1, 0)
};

int parChaosFreq  = 0;
int parXAmpOutput  = 1;
int parYAmpOutput  = 2;
int parZAmpOutput  = 3;
int parXNoteOutput  = 4;
int parYNoteOutput  = 5;
int parZNoteOutput  = 6;
int parRotateX  = 7;
int parRotateY  = 8;
int parRotateZ  = 9;
int parOscMorph = 10;

array<double> inputParameters(parameters.length);
array<string> inputParametersNames = {};
array<double> inputParametersMin = {};
array<double> inputParametersMax = {};
array<double> inputParametersDefault = {};
array<string> inputParametersUnits = {};
array<string> inputParametersFormats = {};
array<string> inputParametersEnums = {};
array<int>    inputParametersSteps = {};

string name = "Chaos-based Arpeggiator";
string description = "A more natural selection of melodies and rhythms";

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
    chaos.setSampleRate(sampleRate);
    chaos.reset();
}

double xAmpAmp, yAmpAmp, zAmpAmp, xNoteAmp, yNoteAmp, zNoteAmp;

void updateInputParameters()
{
    chaos.setFrequency(inputParameters[parChaosFreq]);

    xAmpAmp = inputParameters[parXAmpOutput];
    yAmpAmp = inputParameters[parYAmpOutput];
    zAmpAmp = inputParameters[parZAmpOutput];
    xNoteAmp = inputParameters[parXNoteOutput];
    yNoteAmp = inputParameters[parYNoteOutput];
    zNoteAmp = inputParameters[parZNoteOutput];

    rotator.setAngleX(inputParameters[parRotateX]);
    rotator.setAngleY(inputParameters[parRotateY]);
    rotator.setAngleZ(inputParameters[parRotateZ]);
    
    oscillator.setMorph(inputParameters[parOscMorph]);
}

void handleMidiEvent(const MidiEvent& evt)
{
    switch(MidiEventUtils::getType(evt))
    {
    case kMidiNoteOn:
        {
            amplitude = double(MidiEventUtils::getNoteVelocity(evt))/127.0;
            currentNote = MidiEventUtils::getNote(evt);

            noteTracker.addNote(currentNote, amplitude);

            break;
        }
    case kMidiNoteOff:
        {
            noteTracker.removeNote(currentNote);

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

    double signal = 0;

    for(uint i = 0; i < data.samplesToProcess; i++)
    {
        chaos.inc();

        double xChaos = chaos.getX();
        double yChaos = chaos.getY();
        double zChaos = chaos.getZ();

        rotator.apply(xChaos, yChaos, zChaos);

        double ampXSignal = xChaos * xAmpAmp;
        double ampYSignal = yChaos * yAmpAmp;
        double ampZSignal = zChaos * zAmpAmp;

        double noteXSignal = xChaos * xNoteAmp;
        double noteYSignal = xChaos * yNoteAmp;
        double noteZSignal = xChaos * zNoteAmp;

        int numNotes = noteTracker.currentNotes.length;
        int indexToPlay = 0;
        int octaveAdjust = 0;

        double finalNoteSignal = (noteXSignal + noteYSignal + noteZSignal)/3.0;

        int semitoneToPlay = noteTracker.getNote(finalNoteSignal);


        oscillator.setFrequency(pitchToFrequency(semitoneToPlay));
        signal = oscillator.getSample();

        if (audioOutputsCount >= 2)
        {
            data.samples[0][i] = signal;
            data.samples[1][i] = signal;
        }
        else
        {
            for(uint ch = 0; ch < audioOutputsCount; ++ch)
                data.samples[ch][i] = signal;
        }
    }
}

int getTailSize()
{
    return -1;
}
