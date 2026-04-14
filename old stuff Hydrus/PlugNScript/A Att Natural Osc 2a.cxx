#include "library/Midi.hxx"

string name="Free Osc";
string description="Free Osc";
// array<string> inputParametersNames={"A", "B", "C", "D"};
// array<double> inputParameters(inputParametersNames.length);
double currentPos = 1.0;
double currentMag = 0.0;
double noteHz = 0.00125;
double tanScale = 1.0;
const double PI = 3.1415926535897932384626433832795;
const double halfPI = PI/2.0;
double srQuarter = sampleRate * 0.25;

double midiNoteToHz(double x)
{
	currentPos = 1.0;
	currentMag = 0.0;
	double base = 440.0 * pow(2.0, (x - 69.0) / 12.0);
	
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
	
	

	//double out1 = tanh(currentPos * tanScale) * 0.8;
	double out1 = tanh(currentPos * tanScale) * currentMag * tanScale * .25;
	// if (out1 > 0.99) out1 = 0.99;
 //    if (out1 < -0.99) out1 = -0.99;	
    return out1;

    //return currentPos;
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
	 // noteHz =  midiNoteToHz(inputParameters[1] * 127 + 1);
}

/*
== Notes ==
both derivative and integral of a cos/sin should create the 90 phasew offset


// Octave Up formula  f(x) = x * (4-x)
noteHz = pow(inputParameters[0], 5) * 4.0;
noteHz2 =   noteHz * (4.0 - noteHz) ;

//Octave Down formula f(x) = 2-sqrt(4-x)
noteHz = pow(inputParameters[0], 5) * 4.0;
noteHz2 =   2 - pow(4.0 - noteHz,0.5) ;

double midiNoteToHz(double x)
{
	double base = 440.0 * pow(2.0, (x - 69.0) / 12.0);
	base = base / sampleRate;
	base = base * 4.0;
	return cos((1.0-base) * 3.14159265358979) * 2.0 + 2.0;
}

*/