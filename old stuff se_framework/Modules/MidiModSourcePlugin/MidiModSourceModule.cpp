#include "MidiModSourceModule.h"

MidiModSourceModule::MidiModSourceModule(juce::CriticalSection * lockToUse, MetaParameterManager * metaManagerToUse, ModulationManager * modulationManager, MidiMaster * midiMasterToUse) :
	BasicModule(lockToUse, metaManagerToUse, modulationManager)
{
	ScopedLock scopedLock(*lock);
	midiMasterPtr = midiMasterToUse;	
	BasicModule::setupParametersAndModules();
}

void MidiModSourceModule::setupModulesAndModulation()
{
}

void MidiModSourceModule::createParameters()
{
	parSource.initCombobox("Midi Source", 0, MidiSrcSelMenuItems);
	parCCSelection.initCombobox("Midi CC", 2, BasicModule::MidiControllerNames);
	parKeytrackCenter.initSlider("Center Key", 0, 127, 64);
	parSmoothing.initSlider("Smooth", 1.e-3, 3.0, 0.01);
	parAmplitude.initSlider("Amp", -2, +2, 0.0);
	parOffset.initSlider("DC", -1, +1, 0);

	parKeytrackCenter.stringConvertFunc = &BasicModule::midiNoteToStringWithNum;

	parSource.addMenuItems(MidiSrcSelMenuItems);
	parCCSelection.addMenuItems(BasicModule::MidiControllerNames);

	paramManager.instantiateParameters(this);
}

void MidiModSourceModule::setupCallbacks()
{
	parSource.setCallback([this](double v)
	{
		switch ((MidiSrcSel)(int)v)
		{
		case MidiModSourceModule::pitchwheel:
			midiModSource.setMidiSource(MidiModSource::Source::pitchwheel);
			break;
		case MidiModSourceModule::modwheel:
			midiModSource.setMidiSource(MidiModSource::Source::cc);
			midiModSource.setCurrentCC(1);
			break;
		case MidiModSourceModule::cc:
			midiModSource.setMidiSource(MidiModSource::Source::cc);
			midiModSource.setCurrentCC((int)parCCSelection);
			break;
		case MidiModSourceModule::velocity:
			midiModSource.setMidiSource(MidiModSource::Source::velocity);
			break;
		case MidiModSourceModule::channelpressure:
			midiModSource.setMidiSource(MidiModSource::Source::channelpressure);
			break;
		case MidiModSourceModule::keytrack:
			midiModSource.setMidiSource(MidiModSource::Source::keytrack);
			break;
		}

		updateDisplayName();
	});

	parCCSelection.setCallback([this](double v)
	{
		switch ((int)parSource)
		{
		case MidiModSourceModule::modwheel:
			updateDisplayName();
			midiModSource.setCurrentCC(1);
			break;
		case MidiModSourceModule::cc:
			midiModSource.setCurrentCC((int)v);
			break;
		}
		updateDisplayName();
	});

	parKeytrackCenter.setCallback([this](double v) { midiModSource.setKeytrackCenterKey((int)v); });
	parSmoothing.setCallback([this](double v) { midiModSource.setSmoothing(v); });
	parAmplitude.setCallback([this](double v) { midiModSource.setGain(v); });
	parOffset.setCallback([this](double v) { midiModSource.setOffset(v); });
}

void MidiModSourceModule::setInitPatchValues()
{
	parAmplitude.setValue(1.0);
}

void MidiModSourceModule::setHelpTextAndLabels()
{
}

void MidiModSourceModule::triggerVelocityChange()
{
	midiModSource.setVelocity(getMidiState().velocity);
}

void MidiModSourceModule::triggerNoteChange()
{
	midiModSource.setKeytrackValue(getMidiState().note);
}

void MidiModSourceModule::triggerPitchBendChange()
{ 
	midiModSource.setPitchBend(getMidiState().pitchBend);
}

void MidiModSourceModule::triggerChannelPressureChange()
{ 
	midiModSource.setChannelPressure(getMidiState().channelPressure);
}

void MidiModSourceModule::triggerMidiControllerChange()
{
	midiModSource.setCCValue(getMidiState().ccIndex, getMidiState().ccValue);
}

void MidiModSourceModule::processSampleFrame(double * left, double * right)
{
  	*left = *right = midiModSource.getSample();
}

void MidiModSourceModule::processBlock(double **inOutBuffer, int numChannels, int numSamples)
{
	jassert(numChannels == 2);

	double left, right;
	for (int n = 0; n < numSamples; n++)
	{
		processSampleFrame(&left, &right);

		if (inOutBuffer[0]) inOutBuffer[0][n] = left;
		if (inOutBuffer[1]) inOutBuffer[1][n] = left;
	}
}
