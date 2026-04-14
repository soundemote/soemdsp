#include "library/Midi.hxx"

string name="Free Osc";
string description="Free Osc";
array<string> inputParametersNames={"Morph", "FrequencyOffset"};
array<double> inputParametersMin = {0, -5000};
array<double> inputParametersMax = {1, 5000};
array<double> inputParametersDefault = {0, 0};
array<double> inputParameters(inputParametersNames.length);
double currentPos = 1.0;
double currentMag = 0.0;
double noteHz = 0.00125;
double tanScale = 1.0;
double morph = 1.0;
const double PI = 3.1415926535897932384626433832795;
const double halfPI = PI/2.0;
double srQuarter = sampleRate * 0.25;

double midiNoteToHz(double x)
{
	currentPos = 1.0;
	currentMag = 0.0;
	double base = 440.0 * pow(2.0, (x - 69.0) / 12.0);

	base += inputParameters[1];
	
	tanScale = ((srQuarter / (log10(base) * base)) * halfPI) * 0.25;
	
	base = base / sampleRate;
	base = base * 4.0;
	return cos((1.0-base) * 3.14159265358979) * 2.0 + 2.0;
}

double lastSample = 200.0;
double VolFLP = 0.5;
double multo = 1.0;
const double tra = 0.001;

double perSample()
{
	currentMag = currentMag - currentPos * noteHz;
	currentPos = currentPos + currentMag;
	if (lastSample == 200.0) lastSample = currentPos;
	
    return tanh(currentPos * (tanScale*morph)) * currentMag * tanScale * .25;
}

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
		        {
		        	noteHz = midiNoteToHz(MidiEventUtils::getNote(data.inputMidiEvents[nextEventIndex]));

		        }
			}
            nextEventIndex++;
        }

        double sampleValue = perSample();
        data.samples[0][i]=sampleValue;
		data.samples[1][i]=sampleValue;
    }
}


void updateInputParameters()  
{ 
	 morph = inputParameters[0];
}
