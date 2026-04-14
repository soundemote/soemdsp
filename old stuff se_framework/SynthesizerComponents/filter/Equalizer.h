#pragma once

class ElanLimiterModule : public BasicModule
{
public:
	ElanLimiterModule(juce::CriticalSection * lockToUse,
		jura::MetaParameterManager * metaManagerToUse = nullptr,
		jura::ModulationManager * modManagerToUse = nullptr)
		: BasicModule(lockToUse, metaManagerToUse, modManagerToUse)
	{
		BasicModule::setupParametersAndModules();
	}

	virtual ~ElanLimiterModule() = default;

	void createParameters() override;
	void setupCallbacks() override;
	void setHelpTextAndLabels() override;

	myparams parBypass{ &paramManager };
	ParameterProfile::Linear parThreshold{ &paramManager };
	ParameterProfile::LinearTime parAttack{ &paramManager };
	ParameterProfile::ExponentialTime parRelease{ &paramManager };
	ParameterProfile::Decibel parInLevel{ &paramManager };
	ParameterProfile::Decibel parOutLevel{ &paramManager };
	ParameterProfile::Percent parDryWet{ &paramManager };

	void processSampleFrame(double * Left, double * Right) override;

protected:

	bool isBypassed = false;
	rosic::Limiter limiter;
};

class BandpassModule : public BasicModule
{
public:
	BandpassModule(juce::CriticalSection * lockToUse,
		jura::MetaParameterManager * metaManagerToUse = nullptr,
		jura::ModulationManager * modManagerToUse = nullptr)
		: BasicModule(lockToUse, metaManagerToUse, modManagerToUse)
	{
		BasicModule::setupParametersAndModules();
	}

	virtual ~BandpassModule() = default;

	void createParameters() override;
	void setupCallbacks() override;
	void setHelpTextAndLabels() override;

	myparams parBypass{ &paramManager };
	ParameterProfile::HighpassFilterFrequency parHPF{ &paramManager };
	ParameterProfile::LowpassFilterFrequency parLPF{ &paramManager };

	void processSampleFrame(double * Left, double * Right) override;

protected:
	bool isBypassed = false;
	StereoTwoPoleBandpass filter;
};

class EqualizerEditor : public jura::EqualizerModuleEditor
{
public:
	EqualizerEditor(CriticalSection *newPlugInLock, jura::EqualizerAudioModule * newEqualizerAudioModule)
		: jura::EqualizerModuleEditor(newPlugInLock, newEqualizerAudioModule)
	{
		setPresetSectionPosition(positions::INVISIBLE);
		bypassButton->setPainter(&buttonEnableColorRed);
	}

	~EqualizerEditor() = default;

	void paint(Graphics & g) override;

	void resized() override;

};

class LimiterEditor : public BasicEditor
{
public:

	LimiterEditor(ElanLimiterModule * modulePtr);
	~LimiterEditor() = default;

	void resized() override;

	ElanLimiterModule * modulePtr;
};

class BandpassEditor : public BasicEditor
{
public:
	BandpassEditor(BandpassModule * modulePtr);
	~BandpassEditor() = default;

	void resized() override;

	BandpassModule * modulePtr;
};

class DelayModule : public BasicModule
{
public:

	DelayModule(juce::CriticalSection * lockToUse,
		jura::MetaParameterManager * metaManagerToUse = nullptr,
		jura::ModulationManager * modManagerToUse = nullptr)
		: BasicModule(lockToUse, metaManagerToUse, modManagerToUse)
	{
		BasicModule::setupParametersAndModules();
	}

	virtual ~DelayModule() = default;

	void createParameters() override;
	void setupCallbacks() override;
	void setHelpTextAndLabels() override;

	myparams parBypass{ &paramManager };
	ParameterProfile::Linear parSendAmt{ &paramManager };
	ParameterProfile::LinearTime parDelayTime{ &paramManager };
	ParameterProfile::Percent parFeedback{ &paramManager };
	ParameterProfile::Linear parPan{ &paramManager };
	ParameterProfile::HighpassFilterFrequency parHighDamp{ &paramManager };
	ParameterProfile::LowpassFilterFrequency parLowDamp{ &paramManager };
	ParameterProfile::Percent parDryWet{ &paramManager };
	myparams parPingPongEnable{ &paramManager };
	myparams parTempoSyncEnable{ &paramManager };
	myparams parStereoEnable{ &paramManager };

	void processSampleFrame(double * Left, double * Right) override;

	rosic::PingPongEcho delay;
};

class DelayEditor : public BasicEditor
{
public:

	DelayEditor(DelayModule * modulePtr);
	~DelayEditor() = default;

	void resized() override
	{
		setHeadlinePosition(headlinePositions::TOP_CENTER);

		const int margin = 4;
		const int sliderHeight = 16;
		const int sliderHeightLarge = 24;
		const int sliderWidth = getWidth() - margin * 2;

		modulePtr->parBypass.setBounds(getHeadlineRight()+margin, margin, 60, sliderHeight);

		{ // main controls
			const int border = 2;

			int x = margin;
			int y = getHeadlineBottom() + margin;

			modulePtr->parDelayTime.setBounds(x, y, sliderWidth, sliderHeightLarge);

			y += sliderHeightLarge + margin;

			modulePtr->parTempoSyncEnable.setBounds(x, y, sliderWidth, sliderHeight);

			y += sliderHeight + margin;

			modulePtr->parSendAmt.setBounds(x, y, sliderWidth, sliderHeight);

			y += sliderHeight + margin;

			modulePtr->parFeedback.setBounds(x, y, sliderWidth, sliderHeightLarge);

			y += sliderHeightLarge + margin;

			modulePtr->parPan.setBounds(x, y, sliderWidth, sliderHeight);

			y += sliderHeight + margin;

			modulePtr->parHighDamp.setBounds(x, y, sliderWidth, sliderHeight);
			y += sliderHeight - border;
			modulePtr->parLowDamp.setBounds(x, y, sliderWidth, sliderHeight);

			y += sliderHeight + margin;

			{ // buttons
				int buttonWidth = (sliderWidth - margin*1) / 2;
				modulePtr->parPingPongEnable.setBounds(x, y, buttonWidth, sliderHeight);

				x += buttonWidth + margin;

				modulePtr->parStereoEnable.setBounds(x, y, buttonWidth, sliderHeight);
			}

			y += sliderHeight + margin;

			modulePtr->parDryWet.setBounds(margin, y, sliderWidth, sliderHeight);
		}
	}

	DelayModule * modulePtr;
};
