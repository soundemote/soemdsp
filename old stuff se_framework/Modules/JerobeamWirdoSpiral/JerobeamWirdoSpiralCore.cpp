#include "JerobeamWirdoSpiralCore.h"

void JerobeamWirdoSpiralCore::processSampleFrame(double *outL, double *outR) 
{
	if (isSilent())
	{
		*outL = 0.0;
		*outR = 0.0;
		return;
	}		

	jbWirdo.getSampleFrame(outL, outR);

	*outL *= amplitude;
	*outR *= amplitude;
}

bool JerobeamWirdoSpiralCore::isSilent()
{	
	return amplitude <= 1.e-6;
}
void JerobeamWirdoSpiralCore::triggerFrequencyChange()
{
	jbWirdo.setFrequency(rosic::pitchToFreq(currentNote + pitchOffset));
}
void JerobeamWirdoSpiralCore::triggerAttack()
{
	if ((!note_is_tied && resetMode == ResetMode::when_not_legato) || resetMode == ResetMode::on_note)
		jbWirdo.Reset();

	triggerFrequencyChange();

	amplitude = 1.0;
}

void JerobeamWirdoSpiralCore::triggerRelease()
{
	amplitude = 0.0;
}

void JerobeamWirdoSpiralCore::Reset()
{	
	jbWirdo.Reset();
}

void JerobeamWirdoSpiralCore::setSampleRate(double newSampleRate)
{
	jbWirdo.setSampleRate(newSampleRate);
}