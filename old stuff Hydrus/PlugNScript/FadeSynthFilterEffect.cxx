

#include "library/Midi.hxx"
#include "library/Constants.hxx"

string name="Walter Bass";
string description="Bass Synth";

array<string> inputParametersNames={"Amplitude", "Wave Saw-Squ", "Cutoff", "Resonance", "Drive",
        "Amp Attack", "Amp Delay", "Amp Sustain", "Amp Release", 
        "Filter Attack", "Filter Delay", "Filter Sustain", "Filter Release",
        "LFO Speed", "LFO Shape", "LFO Level",
        "Filter LFO Speed", "Filter LFO Shape", "Filter LFO Level"
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
double perSample(double input1)
{
    double quin;

    sampleValue = 0;
    if (waveFo == 0) waveFo = 1;
    pair = (pair + 1) % waveFo;

    //SawInte = ((pair+0.0)/(waveFo+0.0));

    //SawInte = SawInte*2-1;

    

     switch(waveType)
     {
         case 0: 
            if (pair == 0)
            {
                // currentmag = 0.0 + (rand() - 0.5) * 0.00005;
                currentmag = 0.003 + (rand() - 0.5) * 0.00005;
                currentpos = 0.7 + (rand() - 0.5) * 0.005; 
                // currentmag = 0.0; 
                // currentpos = 0.7;  
                sinespeed = 0.00011;
            }

            sinespeed = sinespeed * 0.99999;
            currentmag = currentmag - currentpos * sinespeed;
            //currentmag = currentmag + (rand() - 0.5) * 0.00003;
            currentmag = currentmag + (rand() - 0.5) * 0.00001;
            //currentpos = currentpos + (rand() - 0.5) * 0.001;
            
            currentpos = (currentpos + currentmag) * 0.997;

            SawInte = currentpos;
            break;
        default:
            if (pair == 0)
            {
                currentmag = 0.0 + (rand() - 0.5) * 0.00005; 
                //currentpos = 0.7 + (rand() - 0.5) * 0.05; 
                currentpos = 0.7 + (rand() - 0.5) * 0.005; 
                //sinespeed = 0.00005;
                sinespeed = 0.00011;
            }

            if (pair == waveFo >> 1)
            {
                currentmag = 0.0 + (rand() - 0.5) * 0.00005; 
                //currentpos = -0.7 + (rand() - 0.5) * 0.05; 
                currentpos = -0.7 + (rand() - 0.5) * 0.005; 
                //sinespeed = 0.00005;
                sinespeed = 0.00011;
            }

            sinespeed = sinespeed * 0.99999;
            currentmag = currentmag - currentpos * sinespeed;
            //currentmag = currentmag + (rand() - 0.5) * 0.00003;
            currentmag = currentmag + (rand() - 0.5) * 0.00001;

            currentpos = (currentpos + currentmag) * 0.997;

            SawInte = currentpos;

            break;
    }


    // switch(waveType)
    // {
    //     case 0: // Saw
    //         SawInte = SawInte*2-1;    
    //     break;
    //     case 1: // Square
    //         if (SawInte >= 0.5) SawInte = 1.0;
    //         else SawInte = -1.0;
    //     break;
    //     case 2: // Parabole Curve Saw
    //         SawInte = SawInte * SawInte * 2 - 1;
    //     break;
    //     case 3: // Triangle / Square Falloff
    //         SawInte = abs(SawInte*2-1)*2-1;
    //     break;
    //     case 4: // U / Saw Falloff
    //         quin = SawInte*2-1;
    //         SawInte = (quin*quin)*2-1;
    //     break;
    //     case 5: // Walter Wave
    //         quin = SawInte * 2 % 1;
    //         SawInte = SawInte - 0.5;
    //         if (SawInte<0) SawInte = -0.25;
    //         if (SawInte>=0) SawInte = +0.25;
    //         SawInte = SawInte - ((pow(quin*(-quin +1),.1) *2-1) - 0.65);
    //     break;
    //     case 6: // Parabol Wave
    //         SawInte = parabolOsc(SawInte);
    //     break;
    //     case 7:
    //         SawInte = getNoise();
    //     break;
    // }
    
    SawInte = input1;
    // DC Blocker
    // out = in - (s += f * (in-s)); // basic hp
    dcBlock = dcBlock + 0.001 * (SawInte-dcBlock);    
    SawInte = SawInte - dcBlock;
    // double yo = 0.01;
    // double out1 = SawInte;
    // dcBlock = dcBlock + yo * (out1-dcBlock);    
    // out1 = out1 - dcBlock;
    // dcBlock2 = dcBlock2 + yo * (out1-dcBlock2);
    // out1 = out1 - dcBlock2;
    // dcBlock3 = dcBlock3 + yo * (out1-dcBlock3);
    // out1 = out1 - dcBlock3;
    // dcBlock4 = dcBlock4+ yo * (out1-dcBlock4);
    // SawInte = out1 - dcBlock4;

    double sampleValue = SawInte;
    //sampleValue = SawInte * getCurrentADSR() * getLFO();
    //double cutoffTotal = Sandwich * getCurrentFADSR() + getFLFO();
    double cutoffTotal = Sandwich + getFLFO();
    //if (cutoffTotal > 1.0) cutoffTotal = 1.0;
    if (cutoffTotal < 0.0) cutoffTotal = 0.0;

    // how about treating the offset like a base offset to zero to a standard oscillator

    followFilterVelocity = followFilterVelocity - (followFilter - sampleValue) * cutoffTotal;
    
     followFilter = (  ((followFilter*3) / (0.5+abs(followFilter*3))) * 1.0 ) * drive + followFilter * (1-drive);

    double hamnt = 0.25;
    double hamnt2 = 0.12;
    // if ((followFilterVelocity > 0.0 && followFilter > 0.0) || (followFilterVelocity < 0.0 && followFilter < 0.0))
    //  followFilterVelocity =  followFilterVelocity * (1.0-abs( (hamnt*followFilter) / (1.0 + abs(hamnt*followFilter) ) ) );

    //followFilterVelocity = ((2*followFilterVelocity) / (1+abs(2*followFilterVelocity))) * drive + followFilterVelocity * (1-drive);


    followFilter = (followFilter + followFilterVelocity) * Reso * 0.99;

    //if (abs(followFilter) > 1.0) followFilter *= 0.9;

    followFilter =  followFilter * (1.0-abs( (hamnt2*followFilter) / (1.0 + abs(hamnt2*followFilter) ) ) );

    followFilter *= 0.99999;
    if (abs(followFilterVelocity) > 1.0) followFilterVelocity *= 0.9;
    
    followFilterVelocity = followFilterVelocity * 0.9999;
    
    sampleValue = followFilter;
    
    sampleValue = ((sampleValue*10) / (1+abs(10*sampleValue))) * (drive / 0.15)  + sampleValue * (1-(drive/0.15));

    sampleValue = min((tanh(abs(sampleValue)*2-1)*0.5)+0.5,abs(sampleValue))*sign(sampleValue);

    dcBlock2 = dcBlock2 + 0.001 * (sampleValue-dcBlock2);    
    sampleValue = sampleValue - dcBlock2;

/////////////////////////////////////////
    // followFilterVelocity2 += ((sampleValue - followFilter2) * cutoffTotal);
    // followFilter2 = (followFilterVelocity2 + followFilter2) * Reso;
    // if (abs(followFilter2) > 0.5) followFilterVelocity2 *= 0.2;
    // if (abs(followFilterVelocity2) > 0.5) followFilterVelocity2 *= 0.2;
    // sampleValue = followFilter2;

    // followFilterVelocity += ((sampleValue - followFilter) * cutoffTotal);
    // followFilter = (followFilterVelocity + followFilter) * Reso;
    // sampleValue = followFilter;

    // followFilterVelocity2 += ((sampleValue - followFilter2) * cutoffTotal);
    // followFilter2 = (followFilterVelocity2 + followFilter2) * Reso;
    // sampleValue = followFilter2;

    // followFilterVelocity *= 0.99;
    // followFilterVelocity2 *= 0.99; 

    if (sampleValue > 5.0) sampleValue = 5.0;
    if (sampleValue < -5.0) sampleValue = -5.0;

    return sampleValue * amplitude;
}

void updateInputParameters()
{  
    amplitude=inputParameters[0]*2;
    waveType = int(inputParameters[1]*1.99);
    Sandwich=inputParameters[2] *inputParameters[2] *inputParameters[2] *inputParameters[2] * 4;
    Reso=inputParameters[3]*inputParameters[3]*1+0.5;
    drive = inputParameters[4] * inputParameters[4] * 0.15;

    vAttack=int( pow(inputParameters[5],2.0) * sampleRate * 10.0 )+1;
    VDelay=int( pow(inputParameters[6],2.0) * sampleRate * 10.0 )+1;
    VSustain=inputParameters[7];
    VRelease=int( pow(inputParameters[8],2.0) * sampleRate * 10.0 )+1;

    FvAttack=int( pow(inputParameters[9],2.0) * sampleRate * 10.0 )+1;
    FVDelay=int( pow(inputParameters[10],2.0) * sampleRate * 10.0 )+1;
    FVSustain=inputParameters[11];
    FVRelease=int( pow(inputParameters[12],2.0) * sampleRate * 10.0 )+1;
    
    LFOSpeed = (inputParameters[13]*30.0+0.01) / sampleRate;
    LFOShape = int(inputParameters[14]*4.99);
    LFOLevel = inputParameters[15];
    
    FLFOSpeed = (inputParameters[16]*30.0+0.01) / sampleRate;
    FLFOShape = int(inputParameters[17]*4.99);
    FLFOLevel = inputParameters[18];
}

int getTailSize() {return -1;}