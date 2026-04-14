#pragma once

using std::vector;

class BasicModule : public jura::AudioModuleWithMidiIn, public juce::ChangeBroadcaster
{
public:
	BasicModule(juce::CriticalSection * lockToUse,
		jura::MetaParameterManager * metaManagerToUse = nullptr, 
		jura::ModulationManager * modManagerToUse = nullptr);

	~BasicModule() = default;

	// demo
	bool getIsInDemoMode() { return isInDemoMode; }
	void setIsInDemoMode(bool v)
	{
		isInDemoMode = v;
		if (!isInDemoMode)
			demoTimedOut = false;
	}
	bool isDemoTimedOut() { return demoTimedOut; }
	void setDemoTimedOut(bool v) { demoTimedOut = v; }

	void setupParametersAndModules()
	{
		setupModulesAndModulation();
		createParameters();
		setupCallbacks();
		setInitPatchValues();
		setHelpTextAndLabels();
	}

	virtual void setupModulesAndModulation() {}
	virtual void createParameters() {}
	virtual void setupCallbacks() {}
	virtual void setInitPatchValues() {}
	virtual void setHelpTextAndLabels() {}

	void setGlobalSmoothingSpeed(double v)
	{
		paramManager.setGlobalSmoothingAmount(v);
		if (smoothingManager != nullptr)
			for (auto & obj : smoothingManager->usedSmoothers)
				obj->setTimeConstantAndSampleRate(v, sampleRate);
	}

	//virtual juce::String getDefaultPresetLocation() override { return programFolder.initPresetPath; }

	virtual void processBlock(double** /*inOutBuffer*/, int /*numChannels*/, int /*numSamples*/) override {}
	virtual void processBlockDemo(double** inOutBuffer, int /*numChannels*/, int numSamples)
	{
		for (int n = 0; n < numSamples; n++)
		{
			double x = inOutBuffer[0] ? inOutBuffer[0][n] : 0.0;
			double y = inOutBuffer[1] ? inOutBuffer[1][n] : 0.0;

			if (inOutBuffer[0]) { inOutBuffer[0][n] = x; }
			if (inOutBuffer[1]) { inOutBuffer[1][n] = y; }
		}
	}
	virtual void processSampleFrame(double* /*left*/, double* /*right*/) {}
	virtual void setPlayheadInfo(AudioPlayHead::CurrentPositionInfo /*info*/) {}

	//// returns 0 to 1 based on where host playhead is in a musical bar. Call handlePlayhead() first.
	//double getBarPhase();
	// returns 0 to 1 like above except based on bars per cycle. Call handlePlayhead() first.
	//double getPhaseBasedOnBarsPerCycle(double barsPerCycle);

	static const vector<String> NoteNames;
	static const vector<String> MidiControllerNames;
	static String midiNoteToString(double v);
	static String midiNoteToStringWithNum(double v);

	/* Core Objects */
	ParamManager paramManager;

protected:
	double
		currentFrequency = 1,
		currentBeatFreq = 0,
		currentPitchToFreq = 1,
		currentFreqMult = 1;

	juce::AudioPlayHead::CurrentPositionInfo positioninfo;

	const double ppb = 960.0; // pulses per bar
	const double ppb_inv = 1/960.0;

	bool isInDemoMode = true;
	bool demoTimedOut = false;
	double sampleRate = 44100;
};
