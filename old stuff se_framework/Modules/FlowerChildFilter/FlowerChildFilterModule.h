#pragma once

using namespace se;

class FlowerChildFilterModule : public BasicModule, public MidiSlave
{
	friend class FlowerChildFilterEditor;

public:
	FlowerChildFilterModule(juce::CriticalSection * lockToUse, jura::MetaParameterManager * metaManagerToUse);

	jura::ModulationManager modulationManager;
	vector<jura::ModulatableParameter2 *> parametersForSmoothing;
	jura::rsSmoothingManager smootyMan;

	/* Core DSP objects */
	FMDFilterStereoWrapper FlowerChildFilter;

	OscilloscopeModule* oscilloscopeModule;

	enum EnvelopeType
	{
		SLEW_LIMITER,
		ENVELOPE_FOLLOWER,
		ADSR_LEGATO,
		ADSR_ALWAYS,
	};

	// plugin
	AudioModuleEditor* createEditor(int type = 0) override;

	void setStateFromXml(const juce::XmlElement& xmlState, const juce::String& stateName, bool markAsClean) override;

	XmlElement* getStateAsXml(const juce::String& stateName, bool markAsClean) override;

	// host
	void setSampleRate(double newSampleRate) override;
	void processBlock(double** inOutBuffer, int numChannels, int numSamples) override;

	// midi
	void handleMidiMessage(MidiMessage message) override
	{
		ScopedLock scopedLock(*lock);

		MIDIMASTER.handleMidiMessage(message);
	}

	void triggerAttack() override
	{
		if (cutoffEnvelopeType == ADSR_ALWAYS || cutoffEnvelopeType == ADSR_LEGATO)
			cutoffADSR.triggerOn(false);

		if (amplitudeEnvelopeType == ADSR_ALWAYS || amplitudeEnvelopeType == ADSR_LEGATO)
			amplitudeADSR.triggerOn(false);
	}
	void triggerNoteOnLegato() override
	{
		if (cutoffEnvelopeType == ADSR_ALWAYS)
			cutoffADSR.triggerOn();
		if (cutoffEnvelopeType == ADSR_ALWAYS)
			amplitudeADSR.triggerOn();
	}
	void triggerNoteOffLegato() override
	{
	}
	void triggerRelease() override
	{
		cutoffADSR.triggerOff();
		amplitudeADSR.triggerOff();
	}

protected:

	/* Core DSP objects */
	MidiMaster MIDIMASTER;

	EnvelopeFollower cutoffSlewLimiter;
	EnvelopeFollower amplitudeSlewLimiter;

	CurveEnvelope cutoffADSR;
	CurveEnvelope amplitudeADSR;

	EnvelopeType cutoffEnvelopeType = EnvelopeType::SLEW_LIMITER;
	EnvelopeType amplitudeEnvelopeType = EnvelopeType::SLEW_LIMITER;

	double ampModGain = 0;
	double freqModGain = 0;
	double fMod = 0;
	double aMod = 0;

	int freqEnvMode = 0;
	int ampEnvMode = 0;
	int freqAdsrTrigMode = 0;
	int ampAdsrTrigMode = 0;

	void updateEnvMode()
	{
		if (freqEnvMode == 1)
			cutoffEnvelopeType = freqAdsrTrigMode == 1 ? EnvelopeType::ADSR_ALWAYS : EnvelopeType::ADSR_LEGATO;
		else
			cutoffEnvelopeType = EnvelopeType::SLEW_LIMITER;

		if (ampEnvMode == 1)
			amplitudeEnvelopeType = ampAdsrTrigMode == 1 ? EnvelopeType::ADSR_ALWAYS : EnvelopeType::ADSR_LEGATO;
		else
			amplitudeEnvelopeType = EnvelopeType::SLEW_LIMITER;
	}

	InternalModSrc midiVelocityModSrc{ "midi_vel_mod_src", "MIDI Velocity" };
	InternalModSrc freqAdsrModSrc{ "freq_adsr_mod_src", "Freq ADSR" };
	InternalModSrc ampAdsrModSrc{ "amp_adsr_mod_src", "Amp ADSR" };

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FlowerChildFilterModule)
};
