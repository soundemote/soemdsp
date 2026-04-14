#include "JerobeamBoingCore.h"

void JerobeamBoingCore::processSampleFrame(double *outL, double *outR) 
{
	if (isSilent())
	{
		*outL = 0.0;
		*outR = 0.0;
		return;
	}		

	jbBoing.getSampleFrame(outL, outR);

	*outL *= amplitude;
	*outR *= amplitude;
}

bool JerobeamBoingCore::isSilent()
{	
	return amplitude <= 1.e-6;
}
void JerobeamBoingCore::triggerFrequencyChange()
{
	jbBoing.setFrequency(rosic::pitchToFreq(currentNote + pitchOffset));
}
void JerobeamBoingCore::triggerAttack()
{
	if ((!note_is_tied && resetMode == ResetMode::when_not_legato) || resetMode == ResetMode::on_note)
		jbBoing.Reset();

	triggerFrequencyChange();

	amplitude = 1.0;
}

void JerobeamBoingCore::triggerRelease()
{
	amplitude = 0.0;
}

void JerobeamBoingCore::Reset()
{	
	jbBoing.Reset();
}

void JerobeamBoingCore::setSampleRate(double newSampleRate)
{
	jbBoing.setSampleRate(newSampleRate);
}