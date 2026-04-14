#pragma once

#include "Graphics.h"
#include "RhythmAndPitchGeneratorModule.h"

class Resources : public ResourcesBase
{
public:
	Resources()
	{
		fonts.tekoBold = loadFont(BinaryData::TekoBold_ttf, BinaryData::TekoBold_ttfSize);
		fonts.tekoMedium = loadFont(BinaryData::TekoMedium_ttf, BinaryData::TekoMedium_ttfSize);
		fonts.tekoRegular = loadFont(BinaryData::TekoRegular_ttf, BinaryData::TekoRegular_ttfSize);
		fonts.robotoRegular = loadFont(BinaryData::RobotoRegular_ttf, BinaryData::RobotoRegular_ttfSize);
		images.presetSection = loadImage(BinaryData::preset_section_png, BinaryData::preset_section_pngSize);
		images.clockLightOn = loadImage(BinaryData::clock_light_on_png, BinaryData::clock_light_on_pngSize);
		images.clockLightOff = loadImage(BinaryData::clock_light_off_png, BinaryData::clock_light_off_pngSize);
		images.background = loadImage(BinaryData::RPGENERATORBACKGROUND_png, BinaryData::RPGENERATORBACKGROUND_pngSize);
	}

	struct FONTS
	{
		Font tekoBold;
		Font tekoMedium;
		Font tekoRegular;
		Font robotoRegular;
	} fonts;

	struct IMAGES
	{
		Image presetSection;
		Image clockLightOn;
		Image clockLightOff;
		Image background;
	} images;
};


// Used as an alternative string display for help and value display
class RaPG_Helper
{
public:
	RaPG_Helper() = default;
	~RaPG_Helper() = default;

	bool doNotUse = true;

	String getValueDisplayString()
	{
		return helpValueString();
	}

	void setStringFunction(std::function<String()> func)
	{
		helpValueString = func;
		doNotUse = false;
	}

	std::function<String()> helpValueString = []() { return String(); };
};

class RaPG_ClockEditor : public WidgetSet
{
public:
	RaPG_ClockEditor(RaPG_RhythmModule * rhythmToEdit, RaPG_ClockModule * clockToEdit);

	~RaPG_ClockEditor()
	{
		clockToEdit->getParameterByName("pulse1time")->deRegisterParameterObserver(pulseDisplay);
		clockToEdit->getParameterByName("pulse2time")->deRegisterParameterObserver(pulseDisplay);
	}

	void resized() override;

	RaPG_ClockModule * clockToEdit;
	RaPG_RhythmModule * rhythmToEdit;
	PulseVisualizer * pulseDisplay;
	NumberCircle * division_slider;
	SVGKnob * pulse1time_slider;
	SVGKnob * pulse2time_slider;
	elan::ImageButton * enable_button;

	SharedResourcePointer<Resources> RESOURCES;
};

class ModulatableNumberDisplayWithHelper : public ModulatableNumberDisplay, public RaPG_Helper
{
public:
	ModulatableNumberDisplayWithHelper(Font font, Colour fontColor)
		: ModulatableNumberDisplay(font, fontColor)
	{
	}
};

class JUCE_API RhythmAndPitchGeneratorEditor : public BasicEditor, public TextEditor::Listener
{
public:

	BackgroundImage backgroundImage;
  
  SharedResourcePointer<Resources> RESOURCES;

	RhythmAndPitchGeneratorEditor(RhythmAndPitchGeneratorModule * moduleToEdit);
	virtual ~RhythmAndPitchGeneratorEditor()
	{
		moduleToEdit->oscillatorModule->getParameterByName("phase")->deRegisterParameterObserver(waveform_display);
		moduleToEdit->oscillatorModule->getParameterByName("saw")->deRegisterParameterObserver(waveform_display);
		moduleToEdit->oscillatorModule->getParameterByName("spike")->deRegisterParameterObserver(waveform_display);
		moduleToEdit->oscillatorModule->getParameterByName("square")->deRegisterParameterObserver(waveform_display);
		moduleToEdit->oscillatorModule->getParameterByName("sine")->deRegisterParameterObserver(waveform_display);
		moduleToEdit->oscillatorModule->getParameterByName("knee")->deRegisterParameterObserver(waveform_display);

		moduleToEdit->envelopeModule->getParameterByName("attack")->deRegisterParameterObserver(envelope_display);
		moduleToEdit->envelopeModule->getParameterByName("decay")->deRegisterParameterObserver(envelope_display);
		moduleToEdit->envelopeModule->getParameterByName("sustain")->deRegisterParameterObserver(envelope_display);
		moduleToEdit->envelopeModule->getParameterByName("release")->deRegisterParameterObserver(envelope_display);
		moduleToEdit->envelopeModule->getParameterByName("attackCurve")->deRegisterParameterObserver(envelope_display);
		moduleToEdit->envelopeModule->getParameterByName("decayCurve")->deRegisterParameterObserver(envelope_display);
		moduleToEdit->envelopeModule->getParameterByName("releaseCurve")->deRegisterParameterObserver(envelope_display);

		for (int i = 0; i < clockWidgetArray.size(); ++i)
		{
			moduleToEdit->rhythmModule->ClockModules[i]->getParameterByName("enable")->deRegisterParameterObserver(clockWidgetArray[i]->enable_button);
		}

		moduleToEdit->removeChangeListener(this);
	}

	void resized() override;
	void paint(Graphics& g) override;

	double bpm_old;
	double bpm_new;
	String presettext_old;
	String presettext_new;
	vector<ElanModulatableSlider*> slidersWithCustomInputBox;
	void changeListenerCallback(ChangeBroadcaster* source) override
	{
		{//update widgets according to state
			if (source == stateWidgetSet)
			{
				updateWidgetsAccordingToState();
				envelope_display->parameterChanged(nullptr);
			}
		}

		{//do preset text
			presettext_new = moduleToEdit->getStateNameWithStarIfDirty();
			if (presettext_new != presetWidget.text)
				presetWidget.setText(presettext_new);
		}

		{//do bpm text
			bpm_new = moduleToEdit->bpm;
			if (bpm_new != bpm_old)
			{
				bpm_textDisplay->setText(String(moduleToEdit->bpm) + " BPM");
				bpm_old = bpm_new;
			}
		}

		// do clock blinkenlights
		clockLightsArray[lastClock]->setIsOn(false);
		currentClock = moduleToEdit->rhythmModule->rhythmGen.getCurrentClockIndex();

		if (currentClock >= clockWidgetArray.size() || currentClock < 0 )
			return;

		if (clockWidgetArray[currentClock]->enable_button->getIsOff())
		{
			clockLightsArray[lastClock]->setIsOn(false);
			return;
		}

		clockLightsArray[currentClock]->setIsOn(true);

		lastClock = currentClock;
	}

	void mouseEnter(const MouseEvent& event) override
	{
		String name, description;
		getDisplayTextForParameterFromMouseEvent(event, &name, &description);
		GlobalParameterText->setText(name);
		GlobalHelpText->setText(description);
	}
	
	void mouseDown(const MouseEvent& event) override
	{
		String name, description;
		getDisplayTextForParameterFromMouseEvent(event, &name, &description);
		GlobalParameterText->setText(name);
		GlobalHelpText->setText(description);

		if (event.eventComponent != &entryFieldForValueInput)
			entryFieldForValueInput.setVisible(false);
	}

	void mouseDrag(const MouseEvent & event) override
	{
		String name, description;
		getDisplayTextForParameterFromMouseEvent(event, &name, &description);
		GlobalParameterText->setText(name);
		GlobalHelpText->setText(description);
	}

	void mouseWheelMove(const MouseEvent& event, const MouseWheelDetails& wheel) override
	{
		String name, description;
		getDisplayTextForParameterFromMouseEvent(event, &name, &description);
		GlobalParameterText->setText(name);
		GlobalHelpText->setText(description);
	}

	void mouseDoubleClick(const MouseEvent& event) override
	{
		currentlyEditingWithInputBox = dynamic_cast<ElanModulatableSlider *>(event.eventComponent);

		if (currentlyEditingWithInputBox == nullptr)
			return;

		juce::Rectangle<int> bounds(90, 20);
		auto area = getLocalArea(currentlyEditingWithInputBox, currentlyEditingWithInputBox->getLocalBounds());

		entryFieldForValueInput.setBounds(area.withSizeKeepingCentre(90, 16));
		entryFieldForValueInput.setText(String(currentlyEditingWithInputBox->getValue()));
		entryFieldForValueInput.selectAll();
		entryFieldForValueInput.setVisible(true);
		entryFieldForValueInput.grabKeyboardFocus();
	}

	void textEditorEscapeKeyPressed(TextEditor&) override
	{
		entryFieldForValueInput.setVisible(false);
	}

	void textEditorReturnKeyPressed(TextEditor&) override

	{
		entryFieldForValueInput.setVisible(false);
		currentlyEditingWithInputBox->setValue(entryFieldForValueInput.getText().getDoubleValue(), false, false);
	}
	
	ElanModulatableSlider * currentlyEditingWithInputBox;

	void getDisplayTextForParameterFromMouseEvent(const MouseEvent & event, String * name, String * description )
	{
		auto helper = dynamic_cast<RaPG_Helper *>(event.eventComponent);
		auto widget = dynamic_cast<jura::RWidget *>(event.eventComponent);
		if (helper != nullptr && !helper->doNotUse && widget != nullptr)
		{
			*description = widget->getDescription();
			*name = widget->getName() + " : " + helper->getValueDisplayString();
			return;
		}
		else if (widget != nullptr)
		{
			*description = widget->getDescription();
			*name = widget->getName() + " : " + widget->getValueDisplayString();
			return;
		}
	}

	RhythmAndPitchGeneratorModule * moduleToEdit;

protected:
	TextEditor entryFieldForValueInput;

	RaPG_PresetWidget presetWidget;

	// Help
	ScopedPointer<elan::StaticText> GlobalParameterText;
	ScopedPointer<elan::StaticText> GlobalHelpText;

	// Main
	ScopedPointer<elan::StaticText> bpm_textDisplay;

	// Global Division
	ModulatableNumberDisplayWithHelper * division_slider;
	StaticClickButton * division_up;
	StaticClickButton * division_dn;

	// Global Reset
	ModulatableNumberDisplayWithHelper * globalResetBars_slider;
	StaticClickButton * resetbars_up;
	StaticClickButton * resetbars_dn;

	// Oversampling
	SimpleNumberDisplay * oversample_slider;
	StaticClickButton * oversample_up;
	StaticClickButton * oversample_dn;

	RaPG_EnvelopeVisualizer * envelope_display;

	SVGKnob * attack_slider;
	SVGKnob * decay_slider;
	SVGKnob * sustain_slider;
	SVGKnob * release_slider;
	SVGKnob * attackCurve_slider;
	SVGKnob * decayCurve_slider;
	SVGKnob * releaseCurve_slider;
	SVGKnob * velocityMod_slider;
	elan::ImageButton * env_triggerOnNote_button;
	elan::ImageButton * env_resetOnNote_button;
	elan::ImageButton * env_triggerOnClick_button;
	elan::ImageButton * env_resetOnClick_button;
	elan::ImageButton * env_triggerOnMasterReset_button;
	elan::ImageButton * env_resetOnMasterReset_button;

	elan::ImageButton * pitch_enable_button;
	NumberCircle * pitch_seed_slider;
	NumberCircle * pitch_steps_slider;
	NumberCircle * pitch_division_slider;
	SVGKnob * pitch_manual_slider;
	SVGKnob * noteLengthMod_slider;
	SVGKnob * octaveAmp_slider;
	elan::ImageButton * bipolar_button;

	elan::ImageButton * rhythm_enable_button;
	NumberCircle * rhythm_seed_slider;
	NumberCircle * rhythm_steps_slider;
	NumberCircle * rhythm_division_slider;
	SVGKnob * rhythm_manual_slider;
	elan::ImageButton * enableGhostClicks_button;

	vector<RaPG_ClockEditor*> clockWidgetArray{ 7 };
	vector<elan::StaticSwitchImageWithText*> clockLightsArray{ 7 };

	RaPG_WaveformVisualizer * waveform_display;
	SVGKnob * phase_slider;
	SVGKnob * frequency_slider;
	SVGKnob * pitch_slider;
	SVGKnob * amplitude_slider;
	SVGKnob *	lpCut_slider;
	SVGKnob * saw_slider;
	SVGKnob * spike_slider;
	SVGKnob * square_slider;
	SVGKnob * sine_slider;
	SVGKnob * knee_slider;
	SVGKnob * ampFilterEnvMod_slider;
	SVGKnob * glideSpeed_slider;
	elan::ImageButton * osc_resetOnNote_button;
	elan::ImageButton * osc_resetOnClick_button;
	elan::ImageButton * osc_resetOnMasterReset_button;

	SVGKnob * masterRate_slider;
	elan::ImageButton * pitchMod_button;
	SVGKnob * masterVolume_slider;
	SVGKnob * paramSmooth_slider;

	int currentClock = 0;
	int lastClock = 0;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RhythmAndPitchGeneratorEditor);
};
