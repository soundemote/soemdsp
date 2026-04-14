#include "FlowerChildFilterCore.h"

#include "FCFBinaryData.cpp"

void FMDFilterStereoWrapper::getSampleStereo(double InL, double InR, double* OutL, double* OutR)
{
	monoSignal = (InL + InR) * 0.5;

	if (isSilent())
	{
		*OutL = *OutR = 0;
		return;
	}	
		
	double currentFrequency = clamp(frequency + frequencyMod, 0.0, 1.0);

	fmdFilterL->setFrequency(currentFrequency);
	fmdFilterR->setFrequency(currentFrequency);

	double amplitudeEnvelopeValue = clamp(outputAmplitude + amplitudeMod * 20, 0.0, 31.62);

	/* OVERSAMPLING STARTS HERE */

	if (getIsStereo())
	{
		for (int i = 0; i < oversampling; i++)
		{
			*OutL = fmdFilterL->getSample(InL);
			*OutR = fmdFilterR->getSample(InR);

			*OutL *= wetGain;
			*OutR *= wetGain;

			*OutL += InL * dryGain;
			*OutR += InR * dryGain;

			*OutL *= amplitudeEnvelopeValue;
			*OutR *= amplitudeEnvelopeValue;

			*OutL = clipper.getSample(*OutL);
			*OutR = clipper.getSample(*OutR);

			*OutL = antiAliasFilterL.getSampleDirect1(*OutL);
			*OutR = antiAliasFilterR.getSampleDirect1(*OutR);
		}
	}
	else
	{
		for (int i = 0; i < oversampling; i++)
		{
			*OutL = fmdFilterL->getSample(InL);

			*OutL *= wetGain;

			*OutL += InL * dryGain;	

			*OutL *= amplitudeEnvelopeValue;

			*OutL = clipper.getSample(*OutL);

			*OutL = *OutR = antiAliasFilterL.getSampleDirect1(*OutL);

		}
	}

	/*********************************/

	slewedOutputSignal = silenceDetector.getSample(abs(*OutL));
}
