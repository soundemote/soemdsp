#include "JerobeamKeplerBouwkampCore.h"

void JerobeamKeplerBouwkampCore::processSampleFrame(double *outL, double *outR) 
{
	if (isSilent())
	{
		*outL = 0.0;
		*outR = 0.0;
		return;
	}		

	jbKepler.getSampleFrame(outL, outR);

	*outL *= amplitude;
	*outR *= amplitude;
}

bool JerobeamKeplerBouwkampCore::isSilent()
{	
	return amplitude <= 1.e-6;
}
void JerobeamKeplerBouwkampCore::triggerFrequencyChange()
{
	jbKepler.setFrequency(rosic::pitchToFreq(currentNote + pitchOffset));
}
void JerobeamKeplerBouwkampCore::triggerAttack()
{
	if ((!note_is_tied && resetMode == ResetMode::when_not_legato) || resetMode == ResetMode::on_note)
		jbKepler.Reset();

	triggerFrequencyChange();

	amplitude = 1.0;
}

void JerobeamKeplerBouwkampCore::triggerRelease()
{
	amplitude = 0.0;
}

void JerobeamKeplerBouwkampCore::Reset()
{	
	jbKepler.Reset();
}

void JerobeamKeplerBouwkampCore::setSampleRate(double newSampleRate)
{
	jbKepler.setSampleRate(newSampleRate);
}