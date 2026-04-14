

#include "library/Midi.hxx"
#include "library/Constants.hxx"

string name="Walter Bass";
string description="Bass Synth";

array<string> inputParametersNames={"Amplitude", "Wave Saw-Squ", "Cutoff", "Resonance", "Drive",
         };
array<double> inputParametersDefault = {
    1.0, 0.0, 0.449, 0.143,
    0.0, 0.05, 0.0, 1.0,
    0.0, 0.15, 0.0, 1.0,
    0.0, 0.2, 0.0, 0.0,
    0.2, 0.0, 0.0
    };
// array<double> inputParametersDefault = {
//     1.0, 0.0, 0.6, 0.0,
//     0.0, 0.05, 0.0, 1.0,
//     0.0, 0.15, 0.0, 1.0,
//     0.0, 0.2, 0.0, 0.0,
//     0.2, 0.0, 0.0
//     };
array<double> inputParameters(inputParametersNames.length);


// Note Array
array<int> noteStatus(128);

// use linked list to iterate over each, or can do linearly and skip the offs
// or can make a new list on each change, linked list pointers

double currentAmplitude=0;
double currentPhase=0;
double currentPitchOffset=0;
uint8  currentNote=0;
const double period=2*PI;

double amplitude = 0;
double sampleValue = 0;

double rnd = 0.0;
double followFilter = 0.0;
double followFilterVelocity = 0.0;
double followFilter2 = 0.0;
double followFilterVelocity2 = 0.0;
double Sandwich = 0.05;
double Reso = 0.0;

double LFOSpeed = 0.0;
int LFOShape = 0;
double LFOLevel = 0.0;
double FLFOSpeed = 0.0;
int FLFOShape = 0;
double FLFOLevel = 0.0;
int waveType = 0;

int waveFo = 1024;
double SawInte = 0.0;
int pair = 0;
double drive= 0.0;
MidiEvent tempEvent;
void handleMidiEvent(const MidiEvent& evt)
{
    switch(MidiEventUtils::getType(evt))
    {
        case kMidiNoteOn:
        {
            //amplitude=double(MidiEventUtils::getNoteVelocity(evt))/127.0;
            currentNote=MidiEventUtils::getNote(evt);
            newFreq(int( pow(2,((double(currentNote-69.0)+currentPitchOffset)/12.0))*440.0  ));
            setADSR(vAttack,4);
            setFADSR(FvAttack,4);
            startingADSRLevel = currentADSRLevel;
            FstartingADSRLevel = FcurrentADSRLevel;
            break;
        }
        case kMidiNoteOff:
        {
            if(currentNote==MidiEventUtils::getNote(evt))
            {
                setADSR(VRelease,1);
                releasingADSRLevel = currentADSRLevel;
                setFADSR(FVRelease,1);
                FreleasingADSRLevel = FcurrentADSRLevel;
            }
                //amplitude=0;
            break;
        }
    }
}

int vAttack = 500000;
int VDelay = 500000;
double VSustain = 0.5;
int VRelease = 500000;
double voiceCurrentLevel = 0.0;

int adsrRun = 0; // decrease to zero
int adsrState = 0; // decrease to zero (3A,2D,1R,0)
double currentADSRLevel = 0.0;
double startingADSRLevel = 0.0;
double releasingADSRLevel = 0.0;

// AH DS R0, each has a time and value  A->1, D->S, R->0, each from current value over so much time
double getCurrentADSR()
{
    if (adsrState > 0)
    {
        switch(adsrState)
        {
            case 4:
                adsrRun--;
                if (adsrRun > vAttack) adsrRun = vAttack;
                if (adsrRun <= 0)
                {
                    adsrRun = VDelay;
                    adsrState--;
                }
                else currentADSRLevel = ((vAttack-adsrRun+0.0) / (vAttack+0.0)) * (1.0 - startingADSRLevel) + startingADSRLevel;
                break;
            case 3:
                adsrRun--;
                if (adsrRun > VDelay) { adsrRun = VDelay;}
                if (adsrRun <= 0) adsrState--;
                else currentADSRLevel = ((adsrRun+0.0) / (VDelay+0.0)) * (1.0 - VSustain) + VSustain;
                
                break;
            case 2:
                currentADSRLevel = VSustain;
                break;
            case 1:
                adsrRun--;
                if (adsrRun > VRelease) adsrRun = VRelease;
                if (adsrRun <= 0) 
                {
                    adsrState--;
                    currentADSRLevel = 0;
                }
                currentADSRLevel = (1.0 - (VRelease-adsrRun+0.0) / (VRelease+0.0)) * releasingADSRLevel;
                break;
        }
    }
    return currentADSRLevel;
}

void setADSR(int Run, int State)
{
    adsrRun = Run;
    adsrState = State;
}


int FvAttack = 500000;
int FVDelay = 500000;
double FVSustain = 0.5;
int FVRelease = 500000;


int FadsrRun = 0; // decrease to zero
int FadsrState = 0; // decrease to zero (3A,2D,1R,0)
double FcurrentADSRLevel = 0.0;
double FstartingADSRLevel = 0.0;
double FreleasingADSRLevel = 0.0;
// AH DS R0, each has a time and value  A->1, D->S, R->0, each from current value over so much time
double getCurrentFADSR()
{
    if (FadsrState > 0)
    {
        switch(FadsrState)
        {
            case 4:
                FadsrRun--;
                if (FadsrRun > FvAttack) FadsrRun = FvAttack;
                if (FadsrRun <= 0)
                {
                    FadsrRun = FVDelay;
                    FadsrState--;
                }
                else FcurrentADSRLevel = ((FvAttack-FadsrRun+0.0) / (FvAttack+0.0)) * (1.0 - FstartingADSRLevel) + FstartingADSRLevel;
                break;
            case 3:
                FadsrRun--;
                if (FadsrRun > FVDelay) { FadsrRun = FVDelay;}
                if (FadsrRun <= 0) FadsrState--;
                else FcurrentADSRLevel = ((FadsrRun+0.0) / (FVDelay+0.0)) * (1.0 - FVSustain) + FVSustain;
                
                break;
            case 2:
                FcurrentADSRLevel = FVSustain;
                break;
            case 1:
                FadsrRun--;
                if (FadsrRun > FVRelease) FadsrRun = FVRelease;
                if (FadsrRun <= 0) 
                {
                    FadsrState--;
                    FcurrentADSRLevel = 0;
                }
                FcurrentADSRLevel = (1.0 - (FVRelease-FadsrRun+0.0) / (FVRelease+0.0)) * FreleasingADSRLevel;
                break;
        }
    }
    return FcurrentADSRLevel;
}

void setFADSR(int FRun, int FState)
{
    FadsrRun = FRun;
    FadsrState = FState;
}


double parabolOsc(double x)
{
    double fit = ((2.0*x) % 2.0) - 1.0;
    return 4.0 * fit * (1.0-abs(fit));
}

int rndNext = 0;
double rndAcc = 0.0;
double getNoise()
{
    rndNext = (rndNext + 109) % 123094;
    rndAcc = parabolOsc(parabolOsc(((rndNext+rndAcc+10))*134987.489798+1987.19687)*1987.4987+98497.19879);
    return rndAcc;
}

double LFOPos = 0.0;
double FLFOPos = 0.0;
double getLFO()
{
    LFOPos = (LFOPos + LFOSpeed) % 1.0;
    double outy = 1.0;
    switch(LFOShape)
    {
        case 0:
           outy = cos(LFOPos * PI * 2) * LFOLevel + 1.0 * (1.0 - LFOLevel);
           break;
        case 1:
           outy = ((1-LFOPos) * 2 - 1) * LFOLevel + 1.0 * (1.0 - LFOLevel);
           break;
        case 2:
           outy = ((1-LFOPos) * (1-LFOPos) * 2 - 1)  * LFOLevel + 1.0 * (1.0 - LFOLevel);
           break;
        case 3:
           outy = (abs(LFOPos * 2 - 1) * 2 - 1) * LFOLevel + 1.0 * (1.0 - LFOLevel);
           break;
        case 4:
           if (LFOPos >= 0.5) outy =  LFOLevel + 1.0 * (1.0 - LFOLevel);
           else outy =  (0.0 - LFOLevel) + 1.0 * (1.0 - LFOLevel);
           break;
    }
    return outy * 0.5 + 0.5;
}

double getFLFO()
{
    FLFOPos = (FLFOPos + FLFOSpeed) % 1.0;
    double outy = 1.0;
    switch(FLFOShape)
    {
        case 0:
           outy = cos(FLFOPos * PI * 2) * FLFOLevel + 0.0 * (1.0 - FLFOLevel);
           break;
        case 1:
           outy = ((1-FLFOPos) * 2 - 1) * FLFOLevel + 1.0 * (0.0 - FLFOLevel);
           break;
        case 2:
           outy = ((1-FLFOPos) * (1-FLFOPos) * 2 - 1)  * FLFOLevel + 0.0 * (1.0 - FLFOLevel);
           break;
        case 3:
           outy = (abs(FLFOPos * 2 - 1) * 2 - 1) * FLFOLevel + 0.0 * (1.0 - FLFOLevel);
           break;
        case 4:
           if (FLFOPos >= 0.5) outy =  FLFOLevel + 0.0 * (1.0 - FLFOLevel);
           else outy =  (0.0 - FLFOLevel) + 0.0 * (1.0 - FLFOLevel);
           break;
    }
    return outy;
}

void newFreq(double newF)
{
    waveFo = int(sampleRate/(newF+1.0));
}

double max(double a,double b)
{
    if (a > b) return a;
    return b;
}

double min(double x, double y)
{
    if (x < y) return x;
    return y;
}

double sign(double x)
{
    if (x < 0) return -1;
    if (x > 0) return 1;
    return 0;
}

void processBlock(BlockData& data)
{
    uint nextEventIndex=0;

    MidiEventUtils::setType(tempEvent,kMidiPitchWheel);
    for(uint i=0;i<data.samplesToProcess;i++)
    {
        while(nextEventIndex!=data.inputMidiEvents.length && data.inputMidiEvents[nextEventIndex].timeStamp<=double(i))
        {
            handleMidiEvent(data.inputMidiEvents[nextEventIndex]);
            nextEventIndex++;
        }

        double sampleValue= perSample(data.samples[0][i]);

        for(uint channel=0;channel<audioOutputsCount;channel++)
        {
            if (channel < 2) data.samples[channel][i]=sampleValue;
            else data.samples[channel][i]=0;
        }
    }
}

double currentmag;
double currentpos;
double sinespeed;

double dcBlock;
double dcBlock2;
double dcBlock3;
double dcBlock4;

double last;

double filterRingMod = 1.0;
double filterRingModVel;
double baechange = 0.0;
double lastplace = 0.0;

double perSample(double input1)
{
    
    // how about treating the offset like a base offset to zero to a standard oscillator
    //ok, so as the offset increases the primary cutoff speed decreases

    double shoooffset = followFilter - input1;

    double cutoffTotal = Sandwich;

    double where = (shoooffset)-(lastplace)  ;
    lastplace =shoooffset;

    baechange = (baechange + abs(where) * 0.5) * 0.5;

    if (baechange > 1.0) baechange = 1.0;

    cutoffTotal *= abs(shoooffset);


    // samplevalue becomes the new baseline, so then
    // as the baseline is changing in location, the speed must decrease with it

    followFilterVelocity = followFilterVelocity - shoooffset * cutoffTotal;
    // followFilterVelocity = followFilterVelocity - input1 * cutoffTotal;
    followFilter = (followFilter + followFilterVelocity) * 0.5;
    followFilter *= 0.99999;

    double sampleValue = followFilter;
    if (sampleValue > 5.0) sampleValue = 5.0;
    if (sampleValue < -5.0) sampleValue = -5.0;

    return sampleValue * amplitude;
}

void updateInputParameters()
{  
    amplitude=inputParameters[0]*2;
    waveType = int(inputParameters[1]*1.99);
    Sandwich=inputParameters[2]*inputParameters[2];
    Sandwich*=Sandwich;
    Reso=inputParameters[3]*inputParameters[3]*1+0.5;
    drive = inputParameters[4] * inputParameters[4] * 0.15;
}

int getTailSize() {return -1;}