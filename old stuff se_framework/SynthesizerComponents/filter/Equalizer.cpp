#include "Equalizer.h"

void ElanLimiterModule::createParameters()
{
	parBypass.initButton("Bypass", 1);
	parThreshold.initSlider("Threshold", -48, 12, 0);
	parAttack.initSlider("Attack", 0, .01, .001);
	parRelease.initSlider("Release", .01, 1, .1);
	parInLevel.initSlider("In Level", -24, 24, 0);
	parOutLevel.initSlider("Out Level", -24, 24, 0);
	parDryWet.initSlider("Dry/Wet", 0, 1, 1);

	parAttack.shouldBeSmoothed = false;
	parRelease.shouldBeSmoothed = false;

	paramManager.instantiateParameters(this);
}

void ElanLimiterModule::setupCallbacks()
{
	parBypass.setCallback([this](double v) { isBypassed = v > 0.5; });
	parThreshold.setCallback([this](double v) { limiter.setLimit(v); });
	parAttack.setCallback([this](double v) { limiter.setAttackTime(v*1000); });
	parRelease.setCallback([this](double v) { limiter.setReleaseTime(v*1000); });
	parInLevel.setCallback([this](double v) { limiter.setInputGain(v); });
	parOutLevel.setCallback([this](double v) { limiter.setOutputGain(v); });
	parDryWet.setCallback([this](double v) { limiter.setDryWetRatio(v); });
}

void ElanLimiterModule::setHelpTextAndLabels()
{
	parBypass.helpText = "Bypass the limiter effect";
	parThreshold.helpText = "Limit above which the signal will be attenuated";
	parAttack.helpText = "Attack time for the limiter's envelope detector";
	parRelease.helpText = "Release time for the limiter's envelope detector";
	parInLevel.helpText = "Pre-compression gain";
	parOutLevel.helpText = "Post-compression gain";
	parDryWet.helpText = "Mix ratio between original and compressed signal";
}

void ElanLimiterModule::processSampleFrame(double * Left, double * Right)
{
	if (!isBypassed)
		limiter.getSampleFrameStereo(Left, Right);
}

void BandpassModule::createParameters()
{
	parBypass.initButton("Bypass", 0);
	parHPF.initSlider("Highpass", .01, 24000, .01);
	parLPF.initSlider("Lowpass", .01, 24000, 24000);

	paramManager.instantiateParameters(this);
}

void BandpassModule::setupCallbacks()
{
	parBypass.setCallback([this](double v) { isBypassed = v > 0.5; });
	parHPF.setCallback([this](double v) { filter.setHighpassCutoff(v); });
	parLPF.setCallback([this](double v) { filter.setLowpassCutoff(v); });
}

void BandpassModule::setHelpTextAndLabels()
{
	parBypass.helpText = "Bypass both highpass and lowpass filters.";
	parHPF.helpText = "Sets frequency of highpass filter.";
	parLPF.helpText = "Sets frequency of lowpass filter.";
}

void BandpassModule::processSampleFrame(double * Left, double * Right)
{
	if (!isBypassed)
		filter.getSample(Left, Right);
}

void DelayModule::createParameters()
{
	parBypass.initButton("Bypass", 0);
	parSendAmt.initSlider("Send Amount", 0, 1, 1);
	parDelayTime.initSlider("Delay Time", .0125, 4, .25);
	parFeedback.initSlider("Feedback Amt", -1, 1, 0);
	parPan.initSlider("Pan", -1, 1, 0);
	parHighDamp.initSlider("High Damp", 20, 20000, 20);
	parLowDamp.initSlider("Low Damp", 20, 20000, 4000);
	parDryWet.initSlider("Dry/Wet", 0, 1, .5);
	parPingPongEnable.initButton("Ping Pong", 1);
	parTempoSyncEnable.initButton("Tempo Sync", 1);
	parStereoEnable.initButton("Stereo", 1);

	parDelayTime.interval = .0125;

	paramManager.instantiateParameters(this);
}

void DelayModule::setupCallbacks()
{
	parBypass.setCallback([this](double v) { delay.setBypass(v >= 0.5); });
	parSendAmt.setCallback([this](double v) { delay.setDelayInputGain(v); });
	parDelayTime.setCallback([this](double v) { delay.setDelayTime(v); });
	parTempoSyncEnable.setCallback([this](double v) { delay.setSyncMode(v >= 0.5); });
	parFeedback.setCallback([this](double v) { delay.setFeedbackFactor(v); });
	parPan.setCallback([this](double v) { delay.setPan(v); });
	parHighDamp.setCallback([this](double v) { delay.setLowDamp(v); });
	parLowDamp.setCallback([this](double v) { delay.setHighDamp(v); });
	parDryWet.setCallback([this](double v) { delay.setDryWetRatio(v); });
	parPingPongEnable.setCallback([this](double v) { delay.setPingPongMode(v >= 0.5); });
	parStereoEnable.setCallback([this](double v) { delay.setTrueStereoMode(v >= 0.5); });

	parBypass.setValue(1);
}

void DelayModule::setHelpTextAndLabels()
{
	parBypass.helpText = "Bypass the delay effect.";
	parSendAmt.helpText = "Amount of signal sent to the delay effect.";
	parDelayTime.helpText = "Delay time in beats";
	parFeedback.helpText = "Amount of feedback in percent";
	parPan.helpText = "Panorama position of the first echo";
	parHighDamp.helpText = "Cutoff frequency for the high damping (lowpass) filter";
	parLowDamp.helpText = "Cutoff frequency for the low damping (highpass) filter";
	parDryWet.helpText = "Ratio between dry and wet signal";
	parPingPongEnable.helpText = "Toggle ping-pong mode (alternating pan-positions) on/off";
	parTempoSyncEnable.helpText = "Toggle tempo synchronization on/off";
	parStereoEnable.helpText = "Toggle true-stereo mode on/off";
}

void DelayModule::processSampleFrame(double * Left, double * Right)
{
	 delay.getSampleFrameStereo(Left, Right);
}

void EqualizerEditor::paint(Graphics & g)
{
	AudioModuleEditor::paint(g);

	jura::fillRectWithBilinearGradient(g, rightSectionRectangle, editorColourScheme.topLeft, editorColourScheme.topRight,
		editorColourScheme.bottomLeft, editorColourScheme.bottomRight);

	g.setColour(editorColourScheme.outline);
	g.drawRect(rightSectionRectangle, 2);

	globalGainSlider->setSliderName("Gain");

	filterModeComboBox->setNameLabelPosition(jura::RNamedComboBox::ABOVE_BOX);
	frequencySlider->setLayout(jura::RSlider::NAME_ABOVE);
	gainSlider->setLayout(jura::RSlider::NAME_ABOVE);
	bandwidthSlider->setLayout(jura::RSlider::NAME_ABOVE);

	stereoModeComboBox->setVisible(false);
}

void EqualizerEditor::resized()
{
	ScopedLock scopedLock(*lock);

	const int margin = 4;
	const int sliderHeight = 16;
	const int doubleSliderHeight = 32;

	stateWidgetSet->setBounds(0, 0, 0, 0);
	rightSectionRectangle.setBounds(0, 0, 0, 0);

	bypassButton->setBounds(getHeadlineRight()+margin*2, margin, 60, sliderHeight);

	{
		const int width = 100;
		gainRangeComboBox->setBounds(getWidth()-width-margin, margin, width, sliderHeight);
	}

	{
		int x = bypassButton->getRight()+margin*2;
		globalGainSlider->setBounds(x, margin, getWidth() - x - gainRangeComboBox->getWidth() - margin*2, sliderHeight);
	}

	{
		int y = gainRangeComboBox->getBottom()+margin;
		plotEditor->setBounds(0, y, getWidth(), getHeight()-y-margin-doubleSliderHeight*2);
	}

	{ // first slider row under plot editor
		const int sliderWidth = (getWidth()-4-4)/2;
		const int y = plotEditor->getBottom();
		int x = margin;

		filterModeComboBox->setBounds(x, y, sliderWidth-margin, doubleSliderHeight);

		x = filterModeComboBox->getRight()+margin;

		frequencySlider->setBounds(x, y, sliderWidth-margin, doubleSliderHeight);

		x = frequencySlider->getRight()+margin;
	}

	{ // second slider row under plot editor
		const int sliderWidth = (getWidth()-margin*2)/2;
		const int y = frequencySlider->getBottom();
		int x = margin;

		gainSlider->setBounds(x, y, sliderWidth-margin, doubleSliderHeight);

		x = gainSlider->getRight()+margin;

		bandwidthSlider->setBounds(x, y, sliderWidth-margin, doubleSliderHeight);
	}

	updateWidgetVisibility();
	updateWidgetAppearance();
}

LimiterEditor::LimiterEditor(ElanLimiterModule * modulePtr)
	: BasicEditor(modulePtr)
	, modulePtr(modulePtr)
{
	ScopedLock scopedLock(*lock);

	modulePtr->addChangeListener(this);

	BasicEditor::createWidgets();

	static_cast<jura::RButton*>(modulePtr->parBypass.widget)->setPainter(&buttonEnableColorRed);
}

BandpassEditor::BandpassEditor(BandpassModule * modulePtr)
	: BasicEditor(modulePtr)
	, modulePtr(modulePtr)
{
	ScopedLock scopedLock(*lock);

	modulePtr->addChangeListener(this);

	BasicEditor::createWidgets();

	static_cast<jura::RButton*>(modulePtr->parBypass.widget)->setPainter(&buttonEnableColorRed);
}

DelayEditor::DelayEditor(DelayModule * modulePtr)
	: BasicEditor(modulePtr)
	, modulePtr(modulePtr)
{
	ScopedLock scopedLock(*lock);

	modulePtr->addChangeListener(this);

	BasicEditor::createWidgets();
	
	static_cast<jura::RButton*>(modulePtr->parBypass.widget)->setPainter(&buttonEnableColorRed);
	static_cast<jura::RButton*>(modulePtr->parTempoSyncEnable.widget)->setPainter(&buttonEnableColorGreen);
	static_cast<jura::RButton*>(modulePtr->parPingPongEnable.widget)->setPainter(&buttonEnableColorGreen);
	static_cast<jura::RButton*>(modulePtr->parStereoEnable.widget)->setPainter(&buttonEnableColorGreen);
}

void LimiterEditor::resized()
{
	setHeadlinePosition(headlinePositions::TOP_CENTER);

	const int margin = 4;
	const int sliderHeight = 16;
	const int sliderHeightLarge = 24;

	modulePtr->parBypass.setBounds(getHeadlineRight()+margin, margin, 60, sliderHeight);

	{
		const int w = getWidth() - margin * 2;		
		const int x = margin;
		const int border = 2;

		int y = getHeadlineBottom() + margin;

		modulePtr->parAttack.setBounds(x, y, w, sliderHeight);

		y = modulePtr->parAttack.widget->getBottom() - border;

		modulePtr->parRelease.setBounds(x, y, w, sliderHeight);

		y = modulePtr->parRelease.widget->getBottom() + margin*2;

		modulePtr->parInLevel.setBounds(x, y, w, sliderHeight);

		y = modulePtr->parInLevel.widget->getBottom() + margin;

		modulePtr->parThreshold.setBounds(x, y, w, sliderHeightLarge);

		y = modulePtr->parThreshold.widget->getBottom() + margin;

		modulePtr->parOutLevel.setBounds(x, y, w, sliderHeight);

		y = modulePtr->parOutLevel.widget->getBottom() + margin*2;

		modulePtr->parDryWet.setBounds(x, y, w, sliderHeight);
	}
}

void BandpassEditor::resized()
{
	setHeadlinePosition(headlinePositions::TOP_CENTER);

	const int margin = 4;

	modulePtr->parBypass.setBounds(getHeadlineRight()+margin, margin, 60, 16);

	{
		const int w = getWidth() - margin * 2;
		const int x = margin;

		int y = getHeadlineBottom() + margin;

		modulePtr->parHPF.setBounds(x, y, w, 24);

		y = modulePtr->parHPF.widget->getBottom() + margin * 2;

		modulePtr->parLPF.setBounds(x, y, w, 24);
	}
}
