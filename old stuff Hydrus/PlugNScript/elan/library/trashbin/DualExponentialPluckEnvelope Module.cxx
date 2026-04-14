#include "library/Envelopes/DualLinearPluckEnvelope.cxx"
#include "library/Oscillators/SineSaw.cxx"
#include "library/Oscillators/MusicalSaw.cxx"
#include "library/Filters/Lowpass.cxx"
#include "library/Parameter.cxx"
#include "library/NoteTracker.cxx"
#include "../library/Midi.hxx"


array<Parameter> parameters = 
{
	Parameter("Attack", 0, .1, 0),
	Parameter("Decay", 0, .1, .002),
	Parameter("Sustain", 0, 1, 0),
	Parameter("Release", 0, 1, 0),
	Parameter("DecayModulation", 0,  20, 16.04),
	Parameter("SustainModulation", 0, 1, 0),
	Parameter("ModAttack", 0, 10, 0), 
	Parameter("ModDecay", 0, 1, .29),
	Parameter("ModDelay", 0, 10, 0),
	Parameter("CurveAmount", -1, +1, -.652),

	Parameter("ExpDelay",   .0001, 1, 0.1),
	Parameter("ExpAttack",   .0001, 1, 0.1),
	Parameter("ExpDecay",    .0001, 10, 0.1),
	Parameter("ExpSustain",     0, 1, 0),
	Parameter("ExpRelease",     0, 1, 0),

	Parameter("ExpAttackMod",   .0001, 1, 0.1),
	Parameter("ExpDecayMod",     0, 10, 0),
	Parameter("ExpReleaseMod",     0, 1, 0),

	Parameter("ExpAttackShape",   0, 4, 2, "", "", 5, "EXP;POW3;LOG;SIGMOID;LIN"),
	Parameter("ExpDecayShape",   0, 4, 0, "", "", 5, "EXP;POW3;LOG;SIGMOID;LIN"),
	Parameter("ExpReleaseShape",   0, 4, 0, "", "", 5, "EXP;POW3;LOG;SIGMOID;LIN"),

	Parameter("ExpGlobalSpeed",   .0001, 100, 0.1),
	Parameter("ExpGlobalMod",   .0001, 10, 0.1)
};

int parAttack = 0;
int parDecay = 1;
int parSustain = 2;
int parRelease = 3;
int parDecayModulation = 4;
int parSustainModulation = 5;
int parModAttack = 6;
int parModDecay = 7;
int parModDelay = 8;
int parCurveAmount = 9;
int parExpDelay = 10;
int parExpAttack = 11;
int parExpDecay = 12;
int parExpSustain = 13;
int parExpRelease = 14;
int parExpAttackMod = 15;
int parExpDecayMod = 16;
int parExpReleaseMod = 17;
int parExpAttackShape = 18;
int parExpDecayShape = 19;
int parExpReleaseShape = 20;
int parExpGlobalSpeed = 21;
int ExpGlobalMod = 22;

array<double> inputParameters(parameters.length);
array<string> inputParametersNames = {};
array<double> inputParametersMin = {};
array<double> inputParametersMax = {};
array<double> inputParametersDefault = {};
array<string> inputParametersUnits = {};
array<string> inputParametersFormats = {};
array<string> inputParametersEnums = {};
array<int>    inputParametersSteps = {};

string name = "Dual Linear Pluck envelope";
string description = "A more natural envelope for pluck sounds.";

NoteTracker noteTracker;

DualLinearPluckEnvelope envelope;
MusicalSaw oscillator;
Lowpass lpf;

double currentPitchOffset=0;
double noteIsOn = 0;
double amplitude = 0;
MidiEvent tempEvent;
uint8  currentNote=0;

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
	envelope.setAttackTime(inputParameters[parAttack]);
	envelope.setDecayTime(inputParameters[parDecay]);
	//envelope.setSustainAmplitude(inputParameters[parSustain]);
	//envelope.setReleaseTime(inputParameters[parRelease]);
	//envelope.setReleaseTime(inputParameters[parDecay]);
	envelope.setDecayModulationStrength(inputParameters[parDecayModulation]);
	//envelope.setSustainModulationStrength(inputParameters[parSustainModulation]);
	envelope.setModAttackTime(inputParameters[parModAttack]);
	envelope.setModDecayTime(inputParameters[parModDecay]);
	envelope.setModDelayTime(inputParameters[parModDelay]);
	envelope.curveAmount = inputParameters[parCurveAmount];

	envelope.mainEnv.setDelayTime(inputParameters[parExpDelay]);
	envelope.mainEnv.setAttackTime(inputParameters[parExpAttack]);
	envelope.mainEnv.setDecayTime(inputParameters[parExpDecay]);
	envelope.mainEnv.setSustainAmplitude(inputParameters[parExpSustain]);
	envelope.mainEnv.setReleaseTime(inputParameters[parExpRelease]);

	envelope.mainEnv.setAttackFeedback(inputParameters[parExpAttackMod]);
	envelope.mainEnv.setDecayFeedback(inputParameters[parExpDecayMod]);
	envelope.mainEnv.setReleaseFeedback(inputParameters[parExpDecayMod]);    

	envelope.mainEnv.setAttackShape(roundToInt(inputParameters[parExpAttackShape]));
	envelope.mainEnv.setDecayShape(roundToInt(inputParameters[parExpDecayShape]));
	envelope.mainEnv.setReleaseShape(roundToInt(inputParameters[parExpDecayShape]));

	envelope.mainEnv.setGlobalTime(inputParameters[parExpGlobalSpeed]);
	envelope.mainEnv.setGlobalFeedback(inputParameters[ExpGlobalMod]);
}

void handleMidiEvent(const MidiEvent& evt)
{
	switch(MidiEventUtils::getType(evt))
	{
	case kMidiNoteOn:
		{
			amplitude = double(MidiEventUtils::getNoteVelocity(evt))/127.0;
			currentNote = MidiEventUtils::getNote(evt);
			oscillator.setFrequency(pitchToFrequency(currentNote));
			envelope.triggerAttack();
			oscillator.reset();
			noteIsOn = 1;

			noteTracker.addNote(currentNote, amplitude);

			break;
		}
	case kMidiNoteOff:
		{
			noteTracker.removeNote(MidiEventUtils::getNote(evt));

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

	double signal = 0;
	double envOut = 0;

	for(uint i = 0; i < data.samplesToProcess; i++)
	{        

		envOut = envelope.getSample();
		// lpf.setFrequency(envOut*10000);
		oscillator.setMorph(clamp(envOut, 0, 1));
		 signal = oscillator.getSample();
		// signal = lpf.getSample(signal);

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
