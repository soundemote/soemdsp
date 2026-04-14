#include "JerobeamBlubbCore.h"

void JerobeamBlubbCore::processSampleFrame(double *outL, double *outR) 
{
	if (isSilent())
	{
		*outL = 0.0;
		*outR = 0.0;
		return;
	}		

	jbBlubb.getSampleFrame(outL, outR);

	*outL *= outputAmp;
	*outR *= outputAmp;
}

bool JerobeamBlubbCore::isSilent()
{	
	return outputAmp <= 1.e-6;
}
void JerobeamBlubbCore::triggerFrequencyChange()
{
	jbBlubb.setFrequency(pitchToFreq(currentNote + pitchOffset));
}
void JerobeamBlubbCore::triggerAttack()
{
	if ((!note_is_tied && resetMode == ResetMode::when_not_legato) || resetMode == ResetMode::on_note)
		jbBlubb.Reset();

	triggerFrequencyChange();

	//jbBlubb.triggerAttack();
}

void JerobeamBlubbCore::triggerRelease()
{
	//jbBlubb.triggerRelease();
}

void JerobeamBlubbCore::Reset()
{	
	jbBlubb.Reset();
}

void JerobeamBlubbCore::setSampleRate(double newSampleRate)
{
	jbBlubb.setSampleRate(newSampleRate);
}