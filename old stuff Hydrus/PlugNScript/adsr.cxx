#include "library/Midi.hxx"
#include "classADSR.cxx"
#include "elan/library/Oscillators/SineSaw.cxx"
#include "elan/library/Filters/Lowpass.cxx"

string name="ADSR";
string description="ADSR";
array<string> inputParametersNames={"Attack", "Decay", "Sustain", "Release", "Resonance", "lpf pitch", "env send"};
array<double> inputParameters(inputParametersNames.length);

double currentPos = 1.0;
double currentMag = 0.0;
double noteHz = 4.0;

//ADSR_Analog adsr;
//ADSR_Analog_Resonant adsr;
//ADSR_Analog_Spike adsr;
ADSR_Formula_One adsr;
//AD_Formula_One adsr;

Lowpass lpf1, lpf2;

SineSaw osc;

void initialize()
{
    osc.setSampleRate(sampleRate);
    lpf1.setSampleRate(sampleRate);
    lpf2.setSampleRate(sampleRate);
}

double midiNoteToHz(double x)
{
	currentPos = 1.0;
	currentMag = 0.0;
	double base = 440.0 * pow(2.0, (x - 69.0) / 12.0);
	base = base / sampleRate;
	base = base * 4.0;
	return cos((1.0-base) * 3.14159265358979) * 2.0 + 2.0;
}

double perSample()
{
    currentMag = currentMag - currentPos * noteHz;
    currentPos = currentPos + currentMag;
    return currentPos;
}

int midinote = 0;
void processBlock(BlockData& data)
{
    uint nextEventIndex=0;
    for(uint i=0;i<data.samplesToProcess;i++)
    {
        while(nextEventIndex!=data.inputMidiEvents.length && data.inputMidiEvents[nextEventIndex].timeStamp<=double(i))
        {
            switch(MidiEventUtils::getType(data.inputMidiEvents[nextEventIndex]))
            {
                case kMidiNoteOn:
                    midinote = MidiEventUtils::getNote(data.inputMidiEvents[nextEventIndex]);
                	noteHz = midiNoteToHz(midinote);
                    osc.reset();
                  adsr.startAttack();
                  break;
                case kMidiNoteOff:
                  adsr.startRelease();
                  break;
            }
            nextEventIndex++;
        }

        double envSignal = adsr.getAmplitude() * inputParameters[6];

        osc.setMorph(envSignal);
        osc.setFrequency(pitchToFrequency(midinote));

        double frq = pitchToFrequency((map(inputParameters[5], -50, 150) + envSignal * 100));

        lpf1.setFrequency(frq);
        lpf2.setFrequency(frq);

        double oscSignal = lpf1.getSample(osc.getSample());

        //double sampleValue = lpf1.getSample(lpf2.getSample());
        print(envSignal+"");
        
        data.samples[0][i]=oscSignal;
		data.samples[1][i]=oscSignal;
    }
}

void updateInputParameters() 
{
    adsr.setAttack(inputParameters[0]);
    adsr.setDecay(inputParameters[1]);
    adsr.setSustain(inputParameters[2]);
    adsr.setRelease(inputParameters[3]);
    adsr.setResonance(inputParameters[4]);
}
