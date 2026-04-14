#include "library/Midi.hxx"

string name="Free Osc";
string description="Free Osc";
array<string> inputParametersNames={"Pitch", "Self FM"};
array<double> inputParameters(inputParametersNames.length);
array<double> inputParametersMin = {-100, -500};
array<double> inputParametersMax = {100, 50};
array<double> inputParametersDefault = {100, 0};
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
	currentMag = currentMag - currentPos * noteHz + noteHz * currentPos * inputParameters[1];
	currentPos = currentPos + currentMag;
	//if (lastSample == 200.0) lastSample = currentPos;
	
	// add some organic noise and feedback mechanics

	// Volume Follower (to level frequency to the proper volume) and stablize the volume
	// perhaps a better way is to some how create waveform seperation between the two..
	// could have a buffer of last x, and call upon it at random.. creaeting natural noise

	// VolFLP = VolFLP * 0.999 + abs(currentPos) * 0.0001;
	// VolFLP += 0.00001 * ((currentPos + 1.0) - VolFLP);
	// average for amp 1 is f(x) = 1.0/(PI/2)

	// multo = multo * (1.0-tra) - (VolFLP-2.0) * tra;
	// multo = multo * 0.8 + 0.2;
	// currentPos *= multo;

	//dcBlock = dcBlock + 0.001 * (in-dcBlock);  

	// -- Derivative Method has issues with amplitude variability due to frequency
	// double retSample = (lastSample - currentPos);
 //    lastSample = currentPos;
	// either derivate iwth a volume adjuster 
	// or introduce 2 oscillators with a phase difference
	// since pitch it integrated, yo ucan simply use that known pitch to scale the derivative accordingly

    // clipping
    // if (currentPos > 0.99) currentPos = 0.99;
    // if (currentPos < -0.99) currentPos = -0.99;

	// if (retSample > 0.99) retSample = 0.99;
 //    if (retSample < -0.99) retSample = -0.99;

	// double out1 = tanh(currentPos * tanScale) * retSample * 15.0;
	//double out1 = tanh(currentPos * tanScale) * 0.8;
	// if (out1 > 0.99) out1 = 0.99;
 //    if (out1 < -0.99) out1 = -0.99;	
    return currentPos;

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