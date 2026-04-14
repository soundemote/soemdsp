#include "JerobeamNyquistShannon.h"

void JerobeamNyquistShannon::getSampleFrame(double *outL, double *outR)
{
	fphas = waveshape::unitrisaw(phasor.getSample(), tri);

	double stair = floor(fphas*sr)/sr;
	double phas = clip(blend*fmod(fphas*sr, 1), 0.0, 1.0)/sr + stair;

	double wave_x = phas*2-1;
	double wave_y = 0;

	const double actualTone = [&]()
	{
		switch (toneMode)
		{
		case JerobeamNyquistShannon::None:
			return tone;
		case JerobeamNyquistShannon::Note:
			return tone + toneSmoother.getSample(midiNote);
		case JerobeamNyquistShannon::Pitch:
			return tone +  toneSmoother.getSample(pitch-1);
		case JerobeamNyquistShannon::Note_Pitch:
			return tone + toneSmoother.getSample((pitch-1) + midiNote);
		case JerobeamNyquistShannon::Freq:
			return tone + frequencyToPitch;
		case JerobeamNyquistShannon::Note_Freq:
			return tone + toneSmoother.getSample(midiNote*0.5) + frequencyToPitch*0.5;
		case JerobeamNyquistShannon::Pitch_Freq:
			return tone + toneSmoother.getSample(pitch-1) + frequencyToPitch;
		case JerobeamNyquistShannon::Note_Pitch_Freq:default:
			return tone + toneSmoother.getSample((pitch-1) + midiNote*0.5) + frequencyToPitch*0.5;
		}
	}();

	double ps_x_PI = phaseRotator.getSample()*TAU; /*optimization*/
	if (change(fphas) == 1)
		wave_y = sin(actualTone * PI_x_2 * phas + ps_x_PI);
	else
		wave_y = -sin(sr * PI * phas + PI_z_2) * sin(phas * (sr/2-actualTone) * PI_x_2 - ps_x_PI);

	*outL = wave_x;
	*outR = wave_y;

	run_gen(); // for "change" function
}
