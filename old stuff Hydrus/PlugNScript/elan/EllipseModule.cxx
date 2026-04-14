#include "library/math.cxx"
#include "library/Oscillators/Phasor.cxx"
#include "library/Oscillators/Ellipse.cxx"

array<double> inputParameters(8);
array<string> inputParametersNames = {"Amplitude", "Frequency",  "Ellipse Offset", "Ellipse Angle", "Ellipse Scale","Phase Offset", "Shape", "Select Shape"};
array<double> inputParametersMin = {0,     0,-1,-1,  0,  -1, -1, 0};
array<double> inputParametersMax = {1, 10000, 1,  1,10,   1, 1, 2};
array<double> inputParametersDefault = {.5, 440, 0, 0, 0, 0, 0};
array<string> inputParametersUnits = {};
array<string> inputParametersFormats={".3",".3", ".3",".3",".3",".3", ".3", ""};
array<string> inputParametersEnums={"","","","","","","","Ellipse;SinToSquare;SinToSaw"};
array<int> inputParametersSteps={0,0,0,0,0,0,0,3};

string name = "Xoxos Ellipse Oscillator";
string description = "Aliased oscillator that transforms between many common shapes using an ellipse equation.";

Phasor phasor;
Ellipse oscillator;

void initialize()
{
	oscillator.setSampleRate(sampleRate);
}

int getTailSize()
{
	return -1;
}

double f = 0;
double amp = 0;
double A;
double B_sin;
double B_cos;
double C;
double shape = 0;
int shapeSelect = 0;

void updateInputParameters()
{
    amp = inputParameters[0];
    
	phasor.setFrequency(inputParameters[1]);
    phasor.setPhaseOffset(inputParameters[5]);

	A = inputParameters[2];
	B_sin = sin(inputParameters[3]);
	B_cos = cos(inputParameters[3]);
	C = inputParameters[4];

	shape = inputParameters[6];
	shapeSelect = roundToInt(inputParameters[7]);
}

double output = 0;

void processBlock(BlockData& data)
{
	for(uint i = 0; i < data.samplesToProcess; i++)
    {
		switch(shapeSelect)
		{
			case 0:
				output = oscillator.getValue(phasor.getSample(), A, B_sin, B_cos, C);
				break;
			case 1:
				output = oscillator.getValueSinToSquare(phasor.getSample(), shape);
				break;
			case 2:
			default:
				output = oscillator.getValueSinToSaw(phasor.getSample(), shape);
				break;
		}

        for(uint ch = 0; ch < audioOutputsCount; ++ch)
            data.samples[ch][i] = output * amp;
    }
}


