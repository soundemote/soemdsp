#include "library/Randoms/VibratoGenerator.cxx"
#include "library/Oscillators/Phasor.cxx"
#include "library/Oscillators/ChaosGenerators.cxx"
#include "library/Dynamics/SoftClipper.cxx"
#include "library/Oscillators/SineSaw.cxx"
#include "library/Dynamics/Rotator3D.cxx"
#include "library/Parameter.cxx"
#include "library/Filters/Highpass.cxx"

array<Parameter> parameters = 
{
	Parameter("Algorithm", 0, 16, 0, "", "", 17, "Lorentz;Aizawa;Bouali;ChenLee;DequanLi;DenTSUCS2;DenGenesioTesi;Hadley;Halvorsen;HyperchaoticQi;Dadra;DenSprottLinzJ;LiuChen;MultiSprottC;SprottLinz;Thomas;LorenzMod1"),
	Parameter("OscPitch", 0, 127, 30),
	Parameter("OscAmp", 0, 1, 0),
	Parameter("Chaos Freq", 0, 1, 0),
	Parameter("X to Freq", -5, 5, 0),
	Parameter("Y to chaosAmp", -1, 1, 0),
	Parameter("Z to Shape", -.1, 1, 0),
	Parameter("X/Y/Z Output", 0, 5, .5),
	Parameter("Rotate X", 0, 1, 0),
	Parameter("Rotate Y", 0, 1, 0),
	Parameter("Rotate Z", 0, 1, 0)
};

int parAlgorithm = 0;
int parOscPitch = 1;
int parOscAmp = 2;
int parChaos_Freq = 3;
int parX_to_Freq = 4;
int parY_to_Amp = 5;
int parZ_to_Shape = 6;
int parXYZ_Output = 7;
int parRotate_X = 8;
int parRotate_Y = 9;
int parRotate_Z = 10;

array<double> inputParameters(parameters.length);
array<string> inputParametersNames = {};
array<double> inputParametersMin = {};
array<double> inputParametersMax = {};
array<double> inputParametersDefault = {};
array<string> inputParametersUnits = {};
array<string> inputParametersFormats = {};
array<string> inputParametersEnums = {};
array<int>    inputParametersSteps = {};

string name = "Chaos Generators";
string description = "A series of chaos algorithms I found on the internet.";

Attractor @chaos = Lorentz();

Rotator3D rotator;
SineSaw osc;
SoftClipper softClip;
Highpass hpf1, hpf2, hpf3;
double chaosAmp = 0;
double oscAmp = 0;
double x_out, y_out, z_out;

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

	chaos.setSampleRate(sampleRate);
	osc.setSampleRate(sampleRate);
	softClip.setWidth(1);
	hpf1.setSampleRate(sampleRate);
	hpf2.setSampleRate(sampleRate);
	hpf3.setSampleRate(sampleRate);

	hpf1.setFrequency(1);
	hpf2.setFrequency(1);
	hpf3.setFrequency(1);
}

int getTailSize()
{
	return -1;
}

int algo = 0;

void updateInputParameters()
{
	if (algo != int(inputParameters[parAlgorithm]))
	{
		switch(algo = int(inputParameters[parAlgorithm]))
		{
		case 0:
			@chaos = Lorentz();
			break;
		case 1:
			@chaos = Aizawa();
			break;
		case 2:
			@chaos = Bouali();
			break;
		case 3:
			@chaos = ChenLee();
			break;
		case 4:
			@chaos = DequanLi();
			break;
		case 5:
			@chaos = DenTSUCS2();
			break;
		case 6:
			@chaos = DenGenesioTesi();
			break;
		case 7:
			@chaos = Hadley();
			break;
		case 8:
			@chaos = Halvorsen();
			break;
		case 9:
			@chaos = HyperchaoticQi();
			break;
		case 10:
			@chaos = Dadra();
			break;
		case 11:
			@chaos = DenSprottLinzJ();
			break;
		case 12:
			@chaos = LiuChen();
			break;
		case 13:
			@chaos = MultiSprottC();
			break;
		case 14:
			@chaos = SprottLinz();
			break;
		case 15:
			@chaos = Thomas();
			break;
		case 16:
		default:
			@chaos = LorenzMod1();
			break;
		}
	}	

	osc.setFrequency(pitchToFrequency(parOscPitch));

	double f = rationalCurve(inputParameters[parChaos_Freq], -.9) * 8000;
	chaos.setFrequency(f);

	hpf1.setFrequency(f*.001);
	hpf2.setFrequency(f*.001);
	hpf3.setFrequency(f*.001);

	chaosAmp = inputParameters[parXYZ_Output];
	oscAmp   = inputParameters[parOscAmp];

	rotator.setAngleX(inputParameters[parRotate_X]);
	rotator.setAngleY(inputParameters[parRotate_Y]);
	rotator.setAngleZ(inputParameters[parRotate_Z]);
}

void processBlock(BlockData& data)
{
	for(uint i = 0; i < data.samplesToProcess; i++)
	{        
		chaos.inc();

		x_out = hpf1.getSample(chaos.getX());
		y_out = hpf2.getSample(chaos.getY());
		z_out = hpf3.getSample(chaos.getZ());

		rotator.apply(x_out, y_out, z_out);

		double pitchMod =       x_out * inputParameters[parX_to_Freq];
		double ampMod   = clamp(y_out * inputParameters[parY_to_Amp],       -1, 1);
		double shapeMod = clamp(z_out * inputParameters[parZ_to_Shape] * .1, 0, 1);
		
		osc.setFrequency(pitchToFrequency(round(inputParameters[0]) + pitchMod));   
		osc.setMorph(shapeMod);

		double oscSignal = osc.getSample() * (oscAmp + ampMod);

		if (audioOutputsCount >= 2)
		{
			data.samples[0][i] = softClip.getSample(oscSignal + x_out * chaosAmp * .1);
			data.samples[1][i] = softClip.getSample(oscSignal + y_out * chaosAmp * .1);
		}
		else
		{
			for(uint ch = 0; ch < audioOutputsCount; ++ch)
				data.samples[ch][i] = softClip.getSample(oscSignal + z_out * chaosAmp * .1);
		}
	}
}