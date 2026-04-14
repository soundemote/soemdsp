/*
TODO: get prime power spread algorithm implemented
https://github.com/RobinSchmidt/RS-MET/blob/master/Libraries/RobsJuceModules/rapt/Math/Misc/RatioGenerator.h
*/

#include "library/math.cxx"
#include "library/Oscillators/HypersawUnit 2.0.cxx"
#include "library/Parameter.cxx"
#include "../library/Midi.hxx"

const uint MAXNUMSAWS = 32;

array<Parameter> parameters =
{
    Parameter("Num Saws",   1, MAXNUMSAWS,   6, "", "", MAXNUMSAWS),
    Parameter("Morph",     0, 1,   1),
    Parameter("Amplitude", 0, 1,  .5),
    Parameter("Reset",     0, 1,   0, "", "", 2, "playing;resetting"),
    Parameter("Mono",      0, 1,  0, "", "", 2, "stereo;mono"),
    Parameter("Vibrato_Speed", 0, 10, 0),
    Parameter("Vibrato_Random_Speed", 0, 100, 0),
    Parameter("Vibrato_Rand_Spd_Offset", 0, 10, 0),
    Parameter("Vibrato_Range(IO)", 0, 12, 0),
    Parameter("Vibrato_Random_Range", 0, 10, 0),
    Parameter("Vibrato_Rand_Rng_Offset", 0, 10, 0),
    Parameter("VibratoDesync", 0, 1, 0),
    Parameter("Portamento_Seconds", 0, 3, 0),
    Parameter("Portamento_Random", 0, 100, 0),
    Parameter("DetuneAmount", 0, 1, 0, "", ".4"),
    Parameter("DetuneCurve", -1, 1, 0, "", ".4"),
    Parameter("DriftColor", 0, 1,   0, "", ".3"),
    Parameter("DriftDetail",0, 1,   0, "", ".3"),
    Parameter("DriftAmount(IO)", 0, 100, 0, "", ".3"),
    Parameter("DriftLPF",     0, 1,   1, "", ".3"),
    Parameter("DriftHPF",     0, 1,   0, "", ".3"),
    Parameter("DelayRandom",     0, 5,   0, "", ".3"),
    Parameter("Attack",     0, 5,   0, "", ".3"),
    Parameter("AttackRandom",     0, 5,   0, "", ".3"),
    Parameter("Decay",     0, 5,   0, "", ".3"),
    Parameter("DecayRandom",     0, 5,   0, "", ".3"),
    Parameter("RandomPhaseOffset", 0, 1, .1, "", ".3")
};

int parNumSaws = 0;
int parMorph = 1;
int parAmplitude = 2;
int parReset = 3;
int parMono = 4;
int parVibrato_Speed = 5;
int parVibrato_Random_Speed = 6;
int parVibrato_Rand_Spd_Offset = 7;
int parVibrato_Range = 8;
int parVibrato_Random_Range = 9;
int parVibrato_Rand_Rng_Offset = 10;
int parVibratoDesync = 11;
int parPortamento_Seconds = 12;
int parPortamento_Random = 13; 
int parDetuneAmount = 14;
int parDetuneCurve = 15;
int parPMDriftColor = 16;
int parPMDriftDetail = 17;
int parPMDriftAmount = 18;
int parDriftLPF = 19;
int parDriftHPF = 20;
int parDelayRandom = 21;
int parAttack = 22;
int parAttackRandom = 23;
int parDecay = 24;
int parDecayRandom = 25;
int parRandomPhaseOffset = 26;

array<double> inputParameters(parameters.length);
array<string> inputParametersNames = {};
array<double> inputParametersMin = {};
array<double> inputParametersMax = {};
array<double> inputParametersDefault = {};
array<string> inputParametersUnits = {};
array<string> inputParametersFormats = {};
array<string> inputParametersEnums = {};
array<int>    inputParametersSteps = {};

string name="Hypersaw";
string description="Multiple sawtooths controlled with pitch drift to create a saw string section.";

HypersawUnit hypersawUnit;
array<HypersawUnit> hypersaw(MAXNUMSAWS);
array<double> detuneValuesIncArray(MAXNUMSAWS);
array<double> detuneValuesIncArrayModified(MAXNUMSAWS);

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

    hypersawUnit.setSampleRate(sampleRate);

    isMono = inputParameters[parMono] != 0.0 ? true : false;
    amplitude = inputParameters[parAmplitude];
    doReset = inputParameters[parReset] != 0.0 ? true : false;

    numSawsChanged();
}

void numSawsChanged()
{
    for (int i = 0; i < numSaws; ++i)
    {
        hypersaw[i].setSampleRate(sampleRate);
        hypersaw[i].setSeed(0+i*5,1000+i*5,2000+i*5,3000+i*5,4000+i*5);
        hypersaw[i].idx = i;
        hypersaw[i].div = 1.0 / numSaws;
        hypersaw[i].phaseDistribution = i * 1.0 / numSaws;

        hypersaw[i].setOscMorph(morph = inputParameters[parMorph]);
        hypersaw[i].setVibratoSpeed(vibSpeed = inputParameters[parVibrato_Speed]);    
        hypersaw[i].setVibratoRandomSpeed(vibRandomSpeed = inputParameters[parVibrato_Random_Speed]);
        hypersaw[i].setVibratoRandomSpeedOffset(vibRandomSpeedOffset = inputParameters[parVibrato_Rand_Spd_Offset]);
        hypersaw[i].setVibratoRange(vibRange = inputParameters[parVibrato_Range]);
        hypersaw[i].setVibratoRandomRange(vibRandomRange = inputParameters[parVibrato_Random_Range]);
        hypersaw[i].vibrato.setRandomRangeOffset(vibRandomRangeOffset = inputParameters[parVibrato_Rand_Rng_Offset]);
        hypersaw[i].setVibratoDesync(vibratoDesync = inputParameters[parVibratoDesync]);
        hypersaw[i].setPortamentoSeconds(portamentoSeconds = inputParameters[parPortamento_Seconds]);
        hypersaw[i].setPortamentoRandomMult(portamentoRandomMult = inputParameters[parPortamento_Random]);
        hypersaw[i].setPitchDetuneAmount(detuneAmount = inputParameters[parDetuneAmount]);
        hypersaw[i].setPitchDetuneCurve(detuneCurve = inputParameters[parDetuneCurve]);
        hypersaw[i].setPMDriftColor(PMDriftColor = inputParameters[parPMDriftColor]);
        hypersaw[i].setPMDriftDetail(PMDriftDetail = inputParameters[parPMDriftDetail]);
        hypersaw[i].setDriftLPF(rationalCurve(driftLPF = inputParameters[parDriftLPF], -.995)*20000);
        hypersaw[i].setDriftHPF(rationalCurve(driftHPF = inputParameters[parDriftHPF], -.995)*5000);
        hypersaw[i].setPMDriftAmount(PMDriftAmount = inputParameters[parPMDriftAmount]);
        hypersaw[i].setRandomDelayRange(delayRandom = inputParameters[parDelayRandom]);
        hypersaw[i].setAttackSeconds(attack = inputParameters[parAttack]);
        hypersaw[i].setAttackSecondsRandomMult(attackRandom = inputParameters[parAttackRandom]);
        hypersaw[i].setReleaseSeconds(decay = inputParameters[parDecay]);
        hypersaw[i].setReleaseSecondsRandomMult(decayRandom = inputParameters[parDecayRandom]);
        hypersaw[i].randomPhaseOffsetRange = inputParameters[parRandomPhaseOffset];
        
        hypersaw[i].reset();
    }
}

double morph = .5;
double vibSpeed = 0;
double vibRandomSpeed = 0;
double vibRandomSpeedOffset = 0;
double vibRange = 0;
double vibRandomRange = 0;
double vibRandomRangeOffset = 0;
double vibratoDesync = 0;
double portamentoSeconds = 0;
double portamentoRandomMult = 0;
double detuneAmount = 0, detuneCurve = 0;
double PMDriftAmount = 1;
double PMDriftColor = 0;
double PMDriftDetail = 0;
double driftLPF = 1;
double driftHPF = 0;
double delayRandom = 0, attack = 0, attackRandom = 0, decay = 0, decayRandom = 0;
double phaseRandom = 0;
int numSaws = 6;

void updateInputParameters()
{    
    if (numSaws != int(inputParameters[parNumSaws]))
    {
        numSaws = int(inputParameters[parNumSaws]);
        numSawsChanged();
    }

    if (morph != inputParameters[parMorph])
        for (int i = 0; i < numSaws; ++i)
            hypersaw[i].setOscMorph(morph = inputParameters[parMorph]);

    if (vibSpeed != inputParameters[parVibrato_Speed])
        for (int i = 0; i < numSaws; ++i)
            hypersaw[i].setVibratoSpeed(vibSpeed = inputParameters[parVibrato_Speed]);    

    if (vibRandomSpeed != inputParameters[parVibrato_Random_Speed])
        for (int i = 0; i < numSaws; ++i)
            hypersaw[i].setVibratoRandomSpeed(vibRandomSpeed = inputParameters[parVibrato_Random_Speed]);

    if (vibRandomSpeedOffset != inputParameters[parVibrato_Rand_Spd_Offset])
        for (int i = 0; i < numSaws; ++i)
        hypersaw[i].setVibratoRandomSpeedOffset(vibRandomSpeedOffset = inputParameters[parVibrato_Rand_Spd_Offset]);

    if (vibRange != inputParameters[parVibrato_Range])
        for (int i = 0; i < numSaws; ++i)
            hypersaw[i].setVibratoRange(vibRange = inputParameters[parVibrato_Range]);  

    if (vibRandomRange != inputParameters[parVibrato_Random_Range])
        for (int i = 0; i < numSaws; ++i)
            hypersaw[i].setVibratoRandomRange(vibRandomRange = inputParameters[parVibrato_Random_Range]);
        
    if (vibRandomRangeOffset != inputParameters[parVibrato_Rand_Rng_Offset])
        for (int i = 0; i < numSaws; ++i)
            hypersaw[i].vibrato.setRandomRangeOffset(vibRandomRangeOffset = inputParameters[parVibrato_Rand_Rng_Offset]);

    if (vibratoDesync != inputParameters[parVibratoDesync])
        for (int i = 0; i < numSaws; ++i)
            hypersaw[i].setVibratoDesync(vibratoDesync = inputParameters[parVibratoDesync]);  

    //if (portamentoSeconds != inputParameters[parPortamento_Seconds])
        for (int i = 0; i < numSaws; ++i)
            hypersaw[i].setPortamentoSeconds(portamentoSeconds = inputParameters[parPortamento_Seconds]);     

    //if (portamentoRandomMult != inputParameters[parPortamento_Random])
        for (int i = 0; i < numSaws; ++i)
            hypersaw[i].setPortamentoRandomMult(portamentoRandomMult = inputParameters[parPortamento_Random]);    

    if (detuneAmount != inputParameters[parDetuneAmount])      
        for (int i = 0; i < numSaws; ++i)   
            hypersaw[i].setPitchDetuneAmount(detuneAmount = inputParameters[parDetuneAmount]);

    if (detuneCurve != inputParameters[parDetuneCurve])      
        for (int i = 0; i < numSaws; ++i)   
            hypersaw[i].setPitchDetuneCurve(detuneCurve = inputParameters[parDetuneCurve]);

    if (PMDriftColor != inputParameters[parPMDriftColor])
        for (int i = 0; i < numSaws; ++i)
            hypersaw[i].setPMDriftColor(PMDriftColor = inputParameters[parPMDriftColor]);

    if (PMDriftDetail != inputParameters[parPMDriftDetail])
        for (int i = 0; i < numSaws; ++i)   
            hypersaw[i].setPMDriftDetail(PMDriftDetail = inputParameters[parPMDriftDetail]);

    if (driftLPF != inputParameters[parDriftLPF])
        for (int i = 0; i < numSaws; ++i)   
            hypersaw[i].setDriftLPF(rationalCurve(driftLPF = inputParameters[parDriftLPF], -.995)*20000);

    if (driftHPF != inputParameters[parDriftHPF])
        for (int i = 0; i < numSaws; ++i)   
            hypersaw[i].setDriftHPF(rationalCurve(driftHPF = inputParameters[parDriftHPF], -.995)*5000);

    if (PMDriftAmount != inputParameters[parPMDriftAmount])   
        for (int i = 0; i < numSaws; ++i)   
            hypersaw[i].setPMDriftAmount(PMDriftAmount = inputParameters[parPMDriftAmount]);

    if (delayRandom != inputParameters[parDelayRandom])
        for (int i = 0; i < numSaws; ++i)
            hypersaw[i].setRandomDelayRange(delayRandom = inputParameters[parDelayRandom]);

    if (attack != inputParameters[parAttack])
        for (int i = 0; i < numSaws; ++i)
            hypersaw[i].setAttackSeconds(attack = inputParameters[parAttack]);

    if (attackRandom != inputParameters[parAttackRandom])
        for (int i = 0; i < numSaws; ++i)
            hypersaw[i].setAttackSecondsRandomMult(attackRandom = inputParameters[parAttackRandom]);

    if (decay != inputParameters[parDecay])
        for (int i = 0; i < numSaws; ++i)
            hypersaw[i].setReleaseSeconds(decay = inputParameters[parDecay]);

    if (decayRandom != inputParameters[parDecayRandom])
        for (int i = 0; i < numSaws; ++i)
            hypersaw[i].setReleaseSecondsRandomMult(decayRandom = inputParameters[parDecayRandom]);

    if (phaseRandom != inputParameters[parRandomPhaseOffset])
        for (int i = 0; i < numSaws; ++i)
            hypersaw[i].randomPhaseOffsetRange = inputParameters[parRandomPhaseOffset];

    isMono = inputParameters[parMono] != 0.0 ? true : false;
    amplitude = inputParameters[parAmplitude];
    doReset = inputParameters[parReset] != 0.0 ? true : false;    
}



// void recalculateDetune()
// {
//     for (uint i = 0; i < numSaws; ++i)
//         detuneValuesIncArray[i] = primePower(i);

//     double minRatio = 1 - 0.5 * detune;
//     double maxRatio = 1 + 0.5 * detune;

//     transformRange(detuneValuesIncArray, detuneValuesIncArray, minRatio, maxRatio);

//     for (uint i = 0; i < numSaws; ++i)
//     {
//         hypersaw[i].setIncrement(detuneValuesIncArray[i] * hypersaw[i].currentFrequency / sampleRate);
//     }
// }


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
            //AdAmplitude = double(MidiEventUtils::getNoteVelocity(evt))/127.0;
            currentNote = MidiEventUtils::getNote(evt);

            for (int i = 0; i < numSaws; ++i)
            {
                hypersaw[i].triggerAttack();
                hypersaw[i].setPitch(currentNote);    
            }        

            break;
        }
    case kMidiNoteOff:
        {
            if(currentNote==MidiEventUtils::getNote(evt))
            {
                //AdAmplitude=0.0;
                for (int i = 0; i < numSaws; ++i)
                    hypersaw[i].triggerRelease();     
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

    bool allEnvsIdle = false;
    for (int x = 0; x < numSaws; ++x)
    {
        allEnvsIdle = false;
        if (!hypersaw[x].env.isIdle())
            break;
        allEnvsIdle = true;
    }

    for(uint i = 0; i < data.samplesToProcess; i++)
    {
        // manage MIDI events
        while(nextEventIndex != data.inputMidiEvents.length)
        {
            
            handleMidiEvent(data.inputMidiEvents[nextEventIndex]);
            nextEventIndex++;
        }
        
        if (doReset)
        {
            hypersawUnit.reset();
            for (int x = 0; x < numSaws; ++x)
                hypersaw[x].reset();
        }
        else
        {
            if (allEnvsIdle)
                return;
            
            double leftChannelValue = 0;
            double rightChannelValue = 0;

            for (int x = 0; x < numSaws; ++x)
            {
                if (x % 2 == 0)
                    leftChannelValue += hypersaw[x].getSample();
                else
                    rightChannelValue += hypersaw[x].getSample();
            }
            
            if (!isMono)
            {
                data.samples[0][i] = leftChannelValue * amplitude;
                data.samples[1][i] = rightChannelValue * amplitude;
            }
            else
            {
                data.samples[0][i] = (leftChannelValue+rightChannelValue) * amplitude;
                data.samples[1][i] = (leftChannelValue+rightChannelValue) * amplitude;
            }
        }
    }
}

int getTailSize()
{
    return -1;
}
