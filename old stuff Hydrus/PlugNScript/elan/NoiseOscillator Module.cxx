#include "library/Oscillators/SineSaw.cxx"
#include "library/Oscillators/MusicalSaw.cxx"
#include "library/Oscillators/Phasor.cxx"
#include "library/Oscillators/PolyBLEP.cxx"
#include "library/Oscillators/TanhSaw.cxx"
#include "library/Filters/Lowpass.cxx"
#include "library/Filters/Highpass.cxx"
#include "library/Parameter.cxx"
#include "../library/Midi.hxx"
#include "library/Randoms/SampleAndHold.cxx"


MusicalSaw sineSawOsc;
PolyBLEP polyBlepOsc;
Phasor phasorOsc;
TanhSaw tanhSawOsc;
SampleAndHold FMsnh;
SampleAndHold PMsnh;
Lowpass lpf1, lpf2;
Highpass hpf1, hpf2;


array<Parameter> parameters = 
{
    Parameter("Oscillator", 0, 3, 0, "", "", 4, "SineSaw;PolyBLEP;Phasor;tanhSawOsc"),
	Parameter("Pitch", 0, 140, 0),
    Parameter("OscMorph", 0, 1, 1),
    Parameter("OscAmp", 0, 1, .5),
    Parameter("Linear FM", 0, 1000, 0),
    Parameter("Expo FM", 0, 12, 0),
    Parameter("PM", 0, 10, 0),
    Parameter("LPF Pitch", -50, 150, 150),
    Parameter("HPF Pitch", -50, 150, -50)
};

int parOscillator = 0;
int parPitch = 1;
int parOscMorph = 2;
int parOscAmp = 3;
int parFMNoiseAmpLinear = 4;
int parFMNoiseAmpExpo = 5;
int parPMNoiseAmp = 6;
int parLPFPitch = 7;
int parHPFPitch = 8;

array<double> inputParameters(parameters.length);
array<string> inputParametersNames = {};
array<double> inputParametersMin = {};
array<double> inputParametersMax = {};
array<double> inputParametersDefault = {};
array<string> inputParametersUnits = {};
array<string> inputParametersFormats = {};
array<string> inputParametersEnums = {};
array<int>    inputParametersSteps = {};

string name = "Noise Oscillator";
string description = "An oscillator that transitions to noise with higher frequency";

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

    sineSawOsc.setSampleRate(sampleRate);
    polyBlepOsc.setSampleRate(sampleRate);
    phasorOsc.setSampleRate(sampleRate); 
    tanhSawOsc.setSampleRate(sampleRate);

    FMsnh.setSampleRate(sampleRate);
    PMsnh.setSampleRate(sampleRate);

    polyBlepOsc.setWaveform(WAVEFORM::SAWTOOTH);
    sineSawOsc.setMorph(1);

    lpf1.setSampleRate(sampleRate);
    lpf2.setSampleRate(sampleRate);
    hpf1.setSampleRate(sampleRate);
    hpf2.setSampleRate(sampleRate);
}

double oscAmp = 0;
double FMNoiseAmpLinear = 0;
double FMNoiseAmpExpo = 0;
double PMnoiseAmp = 0;
double masterFreq = 0;
double masterPitch = 0;
double morph = 0;

void updateInputParameters()
{   
    masterPitch = inputParameters[parPitch];
    masterFreq  = pitchToFrequency(masterPitch);
    morph       = inputParameters[parOscMorph];

    tanhSawOsc.setFrequency(masterFreq);

    sineSawOsc.setMorph(morph);
    tanhSawOsc.setMorph(morph);

    FMsnh.setFrequency(masterFreq);
    PMsnh.setFrequency(masterFreq);

    double lpfFreq = pitchToFrequency(inputParameters[parLPFPitch]);
    double hpfFreq = pitchToFrequency(inputParameters[parHPFPitch]);

    lpf1.setFrequency(lpfFreq);
    lpf2.setFrequency(lpfFreq);
    hpf1.setFrequency(hpfFreq);
    hpf2.setFrequency(hpfFreq);

    oscAmp = inputParameters[parOscAmp];
    FMNoiseAmpLinear = inputParameters[parFMNoiseAmpLinear];
    FMNoiseAmpExpo = inputParameters[parFMNoiseAmpExpo];
    PMnoiseAmp = inputParameters[parPMNoiseAmp];
}

double amplitude;
int currentNote;
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

    double oscSignal = 0;
    double FMsnhSignal = 0;
    double PMsnhSignal = 0;

    for(uint i = 0; i < data.samplesToProcess; i++)
    {
        FMsnh.increment();
        FMsnhSignal = lpf1.getSample(hpf1.getSample(FMsnh.getSmoothedValue()));

        PMsnh.increment();
        PMsnhSignal = lpf2.getSample(hpf2.getSample(PMsnh.getSmoothedValue()));

        double f = pitchToFrequency(masterPitch + FMsnhSignal * FMNoiseAmpExpo) + FMsnhSignal * FMNoiseAmpLinear;

        switch(roundToInt(inputParameters[parOscillator]))
        {
        case 0:
            sineSawOsc.setFrequency(f);  
            sineSawOsc.setPhaseOffset(PMsnhSignal * PMnoiseAmp);
            oscSignal = sineSawOsc.getSample();
            break;
        case 1:
            polyBlepOsc.setFrequency(f);  
            polyBlepOsc.setPhaseOffset(PMsnhSignal * PMnoiseAmp);
            oscSignal = polyBlepOsc.getSample() ;
            break;
        case 2:
            phasorOsc.setFrequency(f);  
            phasorOsc.setPhaseOffset(PMsnhSignal * PMnoiseAmp);
            oscSignal = phasorOsc.getSample();
            oscSignal = oscSignal* 2 - 1;
            break;
        case 3:        
            f = pitchToFrequency(masterPitch + FMsnhSignal * FMNoiseAmpExpo);
            //tanhSawOsc.setFrequency(f);
            tanhSawOsc.setPositionOffset(FMsnhSignal * FMNoiseAmpLinear);
            tanhSawOsc.setMagnitudeOffset(PMsnhSignal * PMnoiseAmp);
            oscSignal = tanhSawOsc.getSample();
            break;
        }

        oscSignal *= oscAmp;

        if (audioOutputsCount >= 2)
        {
            data.samples[0][i] = oscSignal;
            data.samples[1][i] = oscSignal;
        }
        else
        {
            for(uint ch = 0; ch < audioOutputsCount; ++ch)
                data.samples[ch][i] = oscSignal;
        }
    }
}

int getTailSize()
{
    return -1;
}
