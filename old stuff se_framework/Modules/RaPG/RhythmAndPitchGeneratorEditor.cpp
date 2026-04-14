#include <map>
#include <vector>

#include "RhythmAndPitchGeneratorEditor.h"

#include "jura_framework/tools/jura_StringTools.h"

using namespace se;
using namespace juce;

/*
https://regex101.com/r/G9hsLr/1/
https://regex101.com/r/Dg8jrl/1/
*/

RaPG_ClockEditor::RaPG_ClockEditor(RaPG_RhythmModule * rhythmToEdit, RaPG_ClockModule * clockToEdit)
	: clockToEdit(clockToEdit)
	, rhythmToEdit(rhythmToEdit)
{
  addAndMakeVisible(division_slider = new NumberCircle(RESOURCES->fonts.tekoMedium, {125, 123, 119}, { 249, 245, 238 }));
	division_slider->font.setHeight(24);
	division_slider->offsetY = 1.f;
	division_slider->assignParameter(clockToEdit->getParameterByName("divide"));
	division_slider->setName("Clock Division");
	division_slider->setDescription("Number of times clock must be triggered to send a click.");
	division_slider->setStringConversionFunction(&elan::StringFunc3);

	addAndMakeVisible(pulse1time_slider = new SVGKnob());
	pulse1time_slider->setArcUnfilled(2.5, 4.5, { 61, 62, 109 });
	pulse1time_slider->setArcFill(2.5, 4.5, { 237, 234, 229 });
	pulse1time_slider->setBackgroundColor(Colour::fromRGBA(114, 123, 173, 255));
	pulse1time_slider->setArcHighlightColor({ uint8(61), uint8(62), uint8(109), uint8(0.2 * 255) });
	pulse1time_slider->assignParameter(clockToEdit->getParameterByName("pulse1time"));
	pulse1time_slider->setName("Click Delay 1");
	pulse1time_slider->setDescription("Click timing where 0 is instant, 0.5 is a half division of delay, 1 is a whole division late which \
may be percieved if the next triggered clock does not have a pulse at 0.");
	pulse1time_slider->setStringConversionFunction(&elan::StringFunc3);

	addAndMakeVisible(pulse2time_slider = new SVGKnob());
	pulse2time_slider->setArcUnfilled(2.5, 4.5, { 61, 62, 109 });
	pulse2time_slider->setArcFill(2.5, 4.5, { 237, 234, 229 });
	pulse2time_slider->setBackgroundColor(Colour::fromRGBA(114, 123, 173, 255));
	pulse2time_slider->setArcHighlightColor({ uint8(61), uint8(62), uint8(109), uint8(0.2 * 255) });
	pulse2time_slider->assignParameter(clockToEdit->getParameterByName("pulse2time"));
	pulse2time_slider->setName("Click Delay 2");
	pulse2time_slider->setDescription("Click timing where 0 is instant, 0.5 is a half division of delay, 1 is a whole division late which \
may be percieved if the next triggered clock does not have a pulse at 0.");
	pulse2time_slider->setStringConversionFunction(&elan::StringFunc3);

	addAndMakeVisible(enable_button = new elan::ImageButton(
		BinaryData::btn_cdivider_off_png, BinaryData::btn_cdivider_off_pngSize,
		BinaryData::btn_cdivider_on_png, BinaryData::btn_cdivider_on_pngSize));
	enable_button->setClickableArea(21.f, 21.f, 14, 14);
	enable_button->setButtonText("enable");
	enable_button->assignParameter(clockToEdit->getParameterByName("enable"));
	enable_button->setName("Clock");
	enable_button->setDescription("Turns clock divider on or off.");
	enable_button->stringFunc = [this]() { return enable_button->getToggleState() ? "on" : "off"; };

	addAndMakeVisible(pulseDisplay = new PulseVisualizer(clockToEdit));
	pulseDisplay->registerSlider(pulse1time_slider);
	pulseDisplay->registerSlider(pulse2time_slider);
	clockToEdit->getParameterByName("pulse1time")->registerParameterObserver(pulseDisplay);
	clockToEdit->getParameterByName("pulse2time")->registerParameterObserver(pulseDisplay);
	pulseDisplay->setName("Click Display");
	pulseDisplay->setDescription("Displays where in time the triggers occur from Click 1 and Click 2.");
}

void RaPG_ClockEditor::resized()
{
	division_slider->setBounds(0, 27, 27, 27);
	pulse1time_slider->setBounds(27, 0, 27, 27);
	pulse2time_slider->setBounds(54, 27, 27, 27);
	pulseDisplay->setBounds(27, 54, 27, 27);

	enable_button->setCenteredBounds(40.8f, 40.7f, enable_button->getFileWidth(), enable_button->getFileHeight());
}

//==================================================================================//

RhythmAndPitchGeneratorEditor::RhythmAndPitchGeneratorEditor(RhythmAndPitchGeneratorModule * newModuleToEdit)
	: BasicEditor(newModuleToEdit)
	, moduleToEdit(newModuleToEdit)
{
	ScopedLock scopedLock(*lock);

	backgroundImage.setImage(&RESOURCES->images.background);
	addAndMakeVisible(backgroundImage);
	backgroundImage.toBack();

	/*DOUBLE CLICK ENTRY FIELD*/

	addChildComponent(entryFieldForValueInput);
	entryFieldForValueInput.setInputRestrictions(20, "1234567890.-+");
	entryFieldForValueInput.addListener(this);
	entryFieldForValueInput.setAlwaysOnTop(true);
	entryFieldForValueInput.setVisible(false);

	/* PRESET */

	presetWidget.setImage(&RESOURCES->images.presetSection);
	backgroundImage.addMouseListener(&presetWidget, false);
	addAndMakeVisible(presetWidget);	
	presetWidget.toBehind(stateWidgetSet);
	stateWidgetSet->addMouseListener(&presetWidget, true);
	presetWidget.leftButton = dynamic_cast<Component*>(stateWidgetSet->stateMinusButton);
	presetWidget.rightButton = dynamic_cast<Component*>(stateWidgetSet->statePlusButton);
	presetWidget.save = dynamic_cast<Component*>(stateWidgetSet->stateSaveButton);
	presetWidget.load = dynamic_cast<Component*>(stateWidgetSet->stateLoadButton);

	/*HELP*/

	addAndMakeVisible(GlobalParameterText = new elan::StaticText(RESOURCES->fonts.tekoMedium.withPointHeight(25.f)));
	GlobalParameterText->setFontColor({ 228, 224, 228 });
	GlobalParameterText->setFontJustification(juce::Justification::left);
	GlobalParameterText->setText("Parameter : Value");

	addAndMakeVisible(GlobalHelpText = new elan::StaticText(RESOURCES->fonts.robotoRegular.withPointHeight(14.f)));
	GlobalHelpText->setFontColor({ 228, 224, 228 });
	GlobalHelpText->setFontJustification(juce::Justification::left);
	GlobalHelpText->setDrawMultilineText(true);
	GlobalHelpText->setText("Help me!!! :O I'm stuck in cyberspace and I can't get out!");

	/*DISPLAYS*/

	addAndMakeVisible(bpm_textDisplay = new elan::StaticText(RESOURCES->fonts.tekoMedium.withPointHeight(28.f)));
	bpm_textDisplay->setFontColor({ 228, 224, 228 });
	bpm_textDisplay->setTextYOffset(+1.5f);
	
	addAndMakeVisible(waveform_display = new RaPG_WaveformVisualizer());
	moduleToEdit->oscillatorModule->getParameterByName("phase")->registerParameterObserver(waveform_display);
	moduleToEdit->oscillatorModule->getParameterByName("saw")->registerParameterObserver(waveform_display);
	moduleToEdit->oscillatorModule->getParameterByName("spike")->registerParameterObserver(waveform_display);
	moduleToEdit->oscillatorModule->getParameterByName("square")->registerParameterObserver(waveform_display);
	moduleToEdit->oscillatorModule->getParameterByName("sine")->registerParameterObserver(waveform_display);
	moduleToEdit->oscillatorModule->getParameterByName("knee")->registerParameterObserver(waveform_display);

	envelope_display = new RaPG_EnvelopeVisualizer();
	envelope_display->setModulatorToEdit(&moduleToEdit->envelopeModule->envelope.env);
	addAndMakeVisible(envelope_display);

	/*MASTER*/

	addAndMakeVisible(division_slider = new ModulatableNumberDisplayWithHelper(RESOURCES->fonts.tekoMedium, { 228, 224, 228 }));
	division_slider->font.setHeight(22.f);
	division_slider->offsetY = +1.5f;	
	division_slider->assignParameter(moduleToEdit->getParameterByName("division"));
	division_slider->setName("Master Tempo Division");
	division_slider->setDescription("Tempo division of master clock trigger. Triggers are sent to Melody, Rhythm, Envelope, and optionally Oscillator modules.");
	division_slider->setDescriptionField(infoField);
	division_slider->addMouseListener(this, false);
	division_slider->stringFunc = [this]() { return elan::indexToTimeSigName(division_slider->getValue()); };
	division_slider->setStringFunction([this]() { return elan::indexToTimeSigName(division_slider->getValue()) + " (" + String(division_slider->getValue()) + ")"; });

	addAndMakeVisible(division_up = new StaticClickButton(BinaryData::increment_arrow_white_svg));
	division_up->setName(division_slider->getName());
	division_up->setDescription(division_slider->getDescription());
	division_up->assignParameter(moduleToEdit->getParameterByName("division"));
	division_up->setIncrementAmount(+1);
	division_up->addMouseListener(this, false);
	division_up->stringFunc = [this]() { return division_slider->RaPG_Helper::getValueDisplayString(); };

	addAndMakeVisible(division_dn = new StaticClickButton(BinaryData::increment_arrow_white_svg));
	division_dn->setName(division_slider->getName());
	division_dn->setDescription(division_slider->getDescription());
	division_dn->setRotation(180.f);
	division_dn->setIncrementAmount(-1);
	division_dn->assignParameter(moduleToEdit->getParameterByName("division"));
	division_dn->addMouseListener(this, false);
	division_dn->stringFunc = [this]() { return division_slider->RaPG_Helper::getValueDisplayString(); };

	addAndMakeVisible(globalResetBars_slider = new ModulatableNumberDisplayWithHelper(RESOURCES->fonts.tekoMedium, { 228, 224, 228 }));
	globalResetBars_slider->font.setHeight(22.f);
	globalResetBars_slider->offsetY = +1.5f;
	globalResetBars_slider->stringFunc = [this]() { return elan::StringFunc2(globalResetBars_slider->getValue()); };
	globalResetBars_slider->setStringFunction([this]() { return elan::StringFunc5(globalResetBars_slider->getValue()); });
	globalResetBars_slider->assignParameter(moduleToEdit->getParameterByName("globalResetBars"));
	globalResetBars_slider->setName("Global Reset Bars");
	globalResetBars_slider->setDescription("Number of bars before sending a reset to Melody and Rhythm and optionally Envelope and Oscillator. \
0 means never reset.");
	globalResetBars_slider->setDescriptionField(infoField);
	globalResetBars_slider->addMouseListener(this, false);

	addAndMakeVisible(resetbars_up = new StaticClickButton(BinaryData::increment_arrow_white_svg));
	resetbars_up->setName(globalResetBars_slider->getName());
	resetbars_up->setDescription(globalResetBars_slider->getDescription());
	resetbars_up->setIncrementAmount(+1);
	resetbars_up->assignParameter(moduleToEdit->getParameterByName("globalResetBars"));
	resetbars_up->addMouseListener(this, false);
	resetbars_up->stringFunc = [this]() { return globalResetBars_slider->RaPG_Helper::getValueDisplayString(); };

	addAndMakeVisible(resetbars_dn = new StaticClickButton(BinaryData::increment_arrow_white_svg));
	resetbars_dn->setName(globalResetBars_slider->getName());
	resetbars_dn->setDescription(globalResetBars_slider->getDescription());
	resetbars_dn->setRotation(180.f);
	resetbars_dn->setIncrementAmount(-1);
	resetbars_dn->assignParameter(moduleToEdit->getParameterByName("globalResetBars"));
	resetbars_dn->addMouseListener(this, false);
	resetbars_dn->stringFunc = [this]() { return globalResetBars_slider->RaPG_Helper::getValueDisplayString(); };

	addAndMakeVisible(masterRate_slider = new SVGKnob(BinaryData::KNOBdefault_svg));
	masterRate_slider->setArcFillThickness(0);
	masterRate_slider->setArcUnfilledThickness(0);
	masterRate_slider->assignParameter(moduleToEdit->getParameterByName("masterRate"));
	masterRate_slider->setName("Master Rate");
	masterRate_slider->setDescription("Controls the rate of Melody and Rhythm clocks and optionally the pitch of Oscillator. \
WARNING! Do not turn above x128. It is unstable past x128. Don't do it. Stop it. Get some help.");
	masterRate_slider->setDescriptionField(infoField);
	masterRate_slider->addMouseListener(this, false);
	masterRate_slider->setStringConversionFunction(&elan::StringFunc3WithX);

	addAndMakeVisible(pitchMod_button = new elan::ImageButton(
		BinaryData::btn_light_off_png, BinaryData::btn_light_off_pngSize,
		BinaryData::btn_light_on_png, BinaryData::btn_light_on_pngSize));
	pitchMod_button->setClickableArea(13.5f, 13.5f, 15, 17);
	pitchMod_button->setButtonText("pitchMod");
	pitchMod_button->assignParameter(moduleToEdit->getParameterByName("pitchMod"));
	pitchMod_button->setName("Rate-pitch Mod");
	pitchMod_button->setDescription("Allows Master Rate to affect Oscillator's pitch.");
	pitchMod_button->setDescriptionField(infoField);
	pitchMod_button->addMouseListener(this, false);
	pitchMod_button->stringFunc = [this]() { return pitchMod_button->getToggleState() ? "on" : "off"; };

	addAndMakeVisible(masterVolume_slider = new SVGKnob());
	masterVolume_slider->setArcFill(4.5, 6, { 237, 234, 229 });
	masterVolume_slider->setArcUnfilled(4.5, 6, { 122, 124, 217 });
	masterVolume_slider->assignParameter(moduleToEdit->getParameterByName("masterVolume"));
	masterVolume_slider->setName("Master Volume");
	masterVolume_slider->setDescription("Final output amplitude.");
	masterVolume_slider->setDescriptionField(infoField);
	masterVolume_slider->addMouseListener(this, false);
	masterVolume_slider->setStringConversionFunction(&elan::StringFunc3);

	addAndMakeVisible(paramSmooth_slider = new SVGKnob());
	paramSmooth_slider->setArcFillColor({ 237, 234, 229 });
	paramSmooth_slider->setArcUnfilledColor({ 122, 124, 217 });
	paramSmooth_slider->setArcFillThickness(6.8f);
	paramSmooth_slider->setArcUnfilledThickness(6.8f);
	paramSmooth_slider->setArcHighlightColor({ uint8(61), uint8(62), uint8(109), uint8(0.3 * 255) });
	paramSmooth_slider->assignParameter(moduleToEdit->getParameterByName("paramSmooth"));
	paramSmooth_slider->setName("Parameter Smoothing");
	paramSmooth_slider->setDescription("Speed of parameter smoothing, affects user interaction and host automation.");
	paramSmooth_slider->setDescriptionField(infoField);
	paramSmooth_slider->addMouseListener(this, false);
	paramSmooth_slider->setStringConversionFunction(&elan::StringFunc3);

	addAndMakeVisible(oversample_slider = new SimpleNumberDisplay(RESOURCES->fonts.tekoMedium, { 228, 224, 228 }));
	oversample_slider->font.setHeight(32.f);
	oversample_slider->offsetY = +1.5f;	
	oversample_slider->assignParameter(moduleToEdit->getParameterByName("oversample"));
	oversample_slider->setName("Oversampling");
	oversample_slider->setDescription("Sets the CPU usage dedicated to removing digital aliasing and increases accuracy of all processes.");
	oversample_slider->setDescriptionField(infoField);
	oversample_slider->addMouseListener(this, false);
	oversample_slider->stringFunc = [this]() { return "x" + String(oversample_slider->getValue(), 0); };

	addAndMakeVisible(oversample_up = new StaticClickButton(BinaryData::increment_arrow_white_svg));
	oversample_up->setName(oversample_slider->getName());
	oversample_up->setDescription(oversample_slider->getDescription());
	oversample_up->assignParameter(moduleToEdit->getParameterByName("oversample"));
	oversample_up->setIncrementAmount(+1);
	oversample_up->addMouseListener(this, false);
	oversample_up->stringFunc = [this]() { return oversample_slider->getValueDisplayString(); };

	addAndMakeVisible(oversample_dn = new StaticClickButton(BinaryData::increment_arrow_white_svg));
	oversample_dn->setName(oversample_slider->getName());
	oversample_dn->setDescription(oversample_slider->getDescription());
	oversample_dn->setRotation(180.f);
	oversample_dn->setIncrementAmount(-1);
	oversample_dn->assignParameter(moduleToEdit->getParameterByName("oversample"));
	oversample_dn->addMouseListener(this, false);
	oversample_dn->stringFunc = [this]() { return oversample_slider->getValueDisplayString(); };

	/*OSCILLATOR*/

	addAndMakeVisible(phase_slider = new SVGKnob());
	phase_slider->setArcUnfilled(2.8f, 4.5f, { 122, 124, 217 });
	phase_slider->setArcFill(2.8f, 4.5f, { 237, 234, 229 });
	phase_slider->assignParameter(moduleToEdit->oscillatorModule->getParameterByName("phase"));
	phase_slider->setName("Oscillator Phase");
	phase_slider->setDescription(
		"Sets the starting phase of the oscillator as well as modulates the current phase. \
Use this to reduce or enhance clickyness of oscillator restarts.");
	phase_slider->setDescriptionField(infoField);
	phase_slider->addMouseListener(this, false);
	phase_slider->setStringConversionFunction(&elan::StringFunc3);

	addAndMakeVisible(frequency_slider = new SVGKnob());
	frequency_slider->setArcUnfilled(2.8f, 4.5f, { 122, 124, 217 });
	frequency_slider->setArcFill(2.8f, 4.5f, { 237, 234, 229 });
	frequency_slider->setArcHighlightColor({ uint8(61), uint8(62), uint8(109), uint8(0.2 * 255) });
	frequency_slider->assignParameter(moduleToEdit->oscillatorModule->getParameterByName("frequency"));
	frequency_slider->setName("Oscillator Frequency");
	frequency_slider->setDescription("Sets the linear frequency offset of the oscillator in hertz. OUCH!");
	frequency_slider->setDescriptionField(infoField);
	frequency_slider->addMouseListener(this, false);
	frequency_slider->setStringConversionFunction(&elan::StringFunc3);

	addAndMakeVisible(pitch_slider = new SVGKnob());
	pitch_slider->setArcUnfilled(2.8f, 4.5f, { 122, 124, 217 });
	pitch_slider->setArcFill(2.8f, 4.5f, { 237, 234, 229 });
	pitch_slider->setArcHighlightColor({ uint8(61), uint8(62), uint8(109), uint8(0.2 * 255) });
	pitch_slider->assignParameter(moduleToEdit->oscillatorModule->getParameterByName("pitch"));
	pitch_slider->setName("Oscillator Pitch");
	pitch_slider->setDescription("Sets the musical pitch offset of the oscillator in semitones.");
	pitch_slider->setDescriptionField(infoField);
	pitch_slider->addMouseListener(this, false);
	pitch_slider->setStringConversionFunction(&elan::StringFunc3);

	addAndMakeVisible(amplitude_slider = new SVGKnob());
	amplitude_slider->setArcUnfilled(2.8f, 4.5f, { 122, 124, 217 });
	amplitude_slider->setArcFill(2.8f, 4.5f, { 237, 234, 229 });
	amplitude_slider->setArcHighlightColor({ uint8(61), uint8(62), uint8(109), uint8(0.2 * 255) });
	amplitude_slider->assignParameter(moduleToEdit->oscillatorModule->getParameterByName("amplitude"));
	amplitude_slider->setName("Oscillator Amplitude");
	amplitude_slider->setDescription("Sets the final output amplitude of the oscillator.");
	amplitude_slider->setDescriptionField(infoField);
	amplitude_slider->addMouseListener(this, false);
	amplitude_slider->setStringConversionFunction(&elan::StringFunc3);

	addAndMakeVisible(saw_slider = new SVGKnob());
	saw_slider->setArcUnfilled(0, 4, { 122, 124, 217 });
	saw_slider->setArcFill(0, 4, { 237, 234, 229 });
	saw_slider->setArcHighlightColor({ uint8(61), uint8(62), uint8(109), uint8(0.2 * 255) });
	saw_slider->assignParameter(moduleToEdit->oscillatorModule->getParameterByName("saw"));
	saw_slider->setName("Saw Shaper");
	saw_slider->setDescription("Sets the left or right skew of the oscillator allowing for Saw and Tri shapes and \
allows for pulse width modulation when the shape is square.");
	saw_slider->setDescriptionField(infoField);
	saw_slider->addMouseListener(this, false);
	saw_slider->setStringConversionFunction(&elan::StringFunc3);

	addAndMakeVisible(spike_slider = new SVGKnob());
	spike_slider->setArcUnfilled(0, 4, { 122, 124, 217 });
	spike_slider->setArcFill(0, 4, { 237, 234, 229 });
	spike_slider->setArcHighlightColor({ uint8(61), uint8(62), uint8(109), uint8(0.2 * 255) });
	spike_slider->assignParameter(moduleToEdit->oscillatorModule->getParameterByName("spike"));
	spike_slider->setName("Spike Shaper");
	spike_slider->setDescription("Sets the leading and trailing curvature of oscillations allowing for a spike shape.");
	spike_slider->setDescriptionField(infoField);
	spike_slider->addMouseListener(this, false);
	spike_slider->setStringConversionFunction(&elan::StringFunc3);

	addAndMakeVisible(square_slider = new SVGKnob());
	square_slider->setArcUnfilled(0, 4, { 122, 124, 217 });
	square_slider->setArcFill(0, 4, { 237, 234, 229 });
	square_slider->setArcHighlightColor({ uint8(61), uint8(62), uint8(109), uint8(0.2 * 255) });
	square_slider->assignParameter(moduleToEdit->oscillatorModule->getParameterByName("square"));
	square_slider->setName("Square Shaper");
	square_slider->setDescription("Sets the amount of squaring.");
	square_slider->setDescriptionField(infoField);
	square_slider->addMouseListener(this, false);
	square_slider->setStringConversionFunction(&elan::StringFunc3);

	addAndMakeVisible(sine_slider = new SVGKnob());
	sine_slider->setArcUnfilled(0, 4, { 122, 124, 217 });
	sine_slider->setArcFill(0, 4, { 237, 234, 229 });
	sine_slider->setArcHighlightColor({ uint8(61), uint8(62), uint8(109), uint8(0.2 * 255) });
	sine_slider->assignParameter(moduleToEdit->oscillatorModule->getParameterByName("sine"));
	sine_slider->setName("Sine Shaper");
	sine_slider->setDescription("Sets the leading and trailing curvature of oscillations to be sine at 1, linear (triangle) at 0, \
and anti-sine at -0.5, extreme anti-sine at -1.");
	sine_slider->setDescriptionField(infoField);
	sine_slider->addMouseListener(this, false);
	sine_slider->setStringConversionFunction(&elan::StringFunc3);

	addAndMakeVisible(knee_slider = new SVGKnob(BinaryData::KNOBdefault_svg));
	knee_slider->setArcUnfilledThickness(0);
	knee_slider->setArcFillThickness(0);
	knee_slider->assignParameter(moduleToEdit->oscillatorModule->getParameterByName("knee"));
	knee_slider->setName("Knee Shaper");
	knee_slider->setDescription("Positive values creates a knee curve on the trailing side while negative values creates a knew curve on the leading side.");
	knee_slider->setDescriptionField(infoField);
	knee_slider->addMouseListener(this, false);
	knee_slider->setStringConversionFunction(&elan::StringFunc3);

	addAndMakeVisible(lpCut_slider = new SVGKnob(BinaryData::KNOBdefault_svg));
	lpCut_slider->setArcUnfilledThickness(0);
	lpCut_slider->setArcFillThickness(0);
	lpCut_slider->assignParameter(moduleToEdit->oscillatorModule->getParameterByName("lpCut"));
	lpCut_slider->setName("Lowpass Cutoff Max");
	lpCut_slider->setDescription("Sets the maximum opening of the lowpass filter as modulated by Envelope.");
	lpCut_slider->setDescriptionField(infoField);
	lpCut_slider->addMouseListener(this, false);
	lpCut_slider->setStringConversionFunction(&elan::StringFunc3);

	addAndMakeVisible(ampFilterEnvMod_slider = new SVGKnob());
	ampFilterEnvMod_slider->setArcFill(4.5, 6, { 237, 234, 229 });
	ampFilterEnvMod_slider->setArcUnfilled(4.5, 6, { 122, 124, 217 });
	ampFilterEnvMod_slider->setArcHighlightColor({ uint8(61), uint8(62), uint8(109), uint8(0.3 * 255) });
	ampFilterEnvMod_slider->assignParameter(moduleToEdit->oscillatorModule->getParameterByName("ampFilterEnvMod"));
	ampFilterEnvMod_slider->setName("Filter-Amp Mod Crossfade");
	ampFilterEnvMod_slider->setDescription("Inspired by Buchla Lowpass gate. Filter(-) or Amplitude(+) dominant sound. \
Center value for classic lowpass gate sound which modulates filtering and amplitude evenly.");
	ampFilterEnvMod_slider->setDescriptionField(infoField);
	ampFilterEnvMod_slider->addMouseListener(this, false);
	ampFilterEnvMod_slider->setStringConversionFunction(&elan::StringFunc3);

	addAndMakeVisible(glideSpeed_slider = new SVGKnob(BinaryData::KNOBdefault_svg));
	glideSpeed_slider->setArcUnfilledThickness(0);
	glideSpeed_slider->setArcFillThickness(0);
	glideSpeed_slider->assignParameter(moduleToEdit->oscillatorModule->getParameterByName("glideSpeed"));
	glideSpeed_slider->setName("Glide Speed");
	glideSpeed_slider->setDescription("Pitch transition speed of Oscillator when note lengths are tied. \
Glide speed is also affected by host tempo and master rate. \
Here we use a special gaussian filter on pitch to smoothly glide pitch. BABY SMOOTH!");
	glideSpeed_slider->setDescriptionField(infoField);
	glideSpeed_slider->addMouseListener(this, false);
	glideSpeed_slider->setStringConversionFunction(&elan::StringFunc3);

	addAndMakeVisible(osc_resetOnNote_button = new elan::ImageButton(
		BinaryData::btn_light_off_png, BinaryData::btn_light_off_pngSize,
		BinaryData::btn_light_on_png, BinaryData::btn_light_on_pngSize));
	osc_resetOnNote_button->setClickableArea(13.5f, 13.5f, 15, 17);
	osc_resetOnNote_button->setButtonText("resetOnNote");
	osc_resetOnNote_button->assignParameter(moduleToEdit->oscillatorModule->getParameterByName("resetOnNote"));
	osc_resetOnNote_button->setName("Osc Reset via Melody");
	osc_resetOnNote_button->setDescription("Resets the oscillator when triggered by Melody.");
	osc_resetOnNote_button->setDescriptionField(infoField);
	osc_resetOnNote_button->addMouseListener(this, false);
	osc_resetOnNote_button->stringFunc = [this]() { return osc_resetOnNote_button->getToggleState() ? "on" : "off"; };

	addAndMakeVisible(osc_resetOnClick_button = new elan::ImageButton(
		BinaryData::btn_light_off_png, BinaryData::btn_light_off_pngSize,
		BinaryData::btn_light_on_png, BinaryData::btn_light_on_pngSize));
	osc_resetOnClick_button->setClickableArea(13.5f, 13.5f, 15, 17);
	osc_resetOnClick_button->setButtonText("resetOnClick");
	osc_resetOnClick_button->assignParameter(moduleToEdit->oscillatorModule->getParameterByName("resetOnClick"));
	osc_resetOnClick_button->setName("Osc Reset via Rhythm");
	osc_resetOnClick_button->setDescription("Resets the oscillator when triggered by Rhythm.");
	osc_resetOnClick_button->setDescriptionField(infoField);
	osc_resetOnClick_button->addMouseListener(this, false);
	osc_resetOnClick_button->stringFunc = [this]() { return osc_resetOnClick_button->getToggleState() ? "on" : "off"; };

	addAndMakeVisible(osc_resetOnMasterReset_button = new elan::ImageButton(
		BinaryData::btn_light_off_png, BinaryData::btn_light_off_pngSize,
		BinaryData::btn_light_on_png, BinaryData::btn_light_on_pngSize));
	osc_resetOnMasterReset_button->setClickableArea(13.5f, 13.5f, 15, 17);
	osc_resetOnMasterReset_button->setButtonText("resetOnMasterReset");
	osc_resetOnMasterReset_button->assignParameter(moduleToEdit->oscillatorModule->getParameterByName("resetOnMasterReset"));
	osc_resetOnMasterReset_button->setName("Osc Reset via Master");
	osc_resetOnMasterReset_button->setDescription("Resets the oscillator based on master reset.");
	osc_resetOnMasterReset_button->setDescriptionField(infoField);
	osc_resetOnMasterReset_button->addMouseListener(this, false);
	osc_resetOnMasterReset_button->stringFunc = [this]() { return osc_resetOnMasterReset_button->getToggleState() ? "on" : "off"; };

	/*ENVELOPE*/

	addAndMakeVisible(attack_slider = new SVGKnob());
	attack_slider->setArcFill(2.5f, 4.8f, { 237, 234, 229 });
	attack_slider->setArcUnfilled(2.5f, 4.8f, { 122, 124, 217 });
	attack_slider->setArcHighlightColor({ uint8(61), uint8(62), uint8(109), uint8(0.2 * 255) });
	attack_slider->assignParameter(moduleToEdit->envelopeModule->getParameterByName("attack"));
	attack_slider->setName("Envelope Attack Time");
	attack_slider->setDescription("Sets the time in seconds for the envelope attack.");
	attack_slider->setDescriptionField(infoField);
	attack_slider->addMouseListener(this, false);
	attack_slider->setStringConversionFunction(&elan::secondsToStringWithUnitTotal4);

	addAndMakeVisible(decay_slider = new SVGKnob());
	decay_slider->setArcFill(2.5f, 4.8f, { 237, 234, 229 });
	decay_slider->setArcUnfilled(2.5f, 4.8f, { 122, 124, 217 });
	decay_slider->setArcHighlightColor({ uint8(61), uint8(62), uint8(109), uint8(0.2 * 255) });
	decay_slider->assignParameter(moduleToEdit->envelopeModule->getParameterByName("decay"));
	decay_slider->setName("Envelope Decay Time");
	decay_slider->setDescription("Sets the time in seconds for the envelope decay.");
	decay_slider->setDescriptionField(infoField);
	decay_slider->addMouseListener(this, false);
	decay_slider->setStringConversionFunction(&elan::secondsToStringWithUnitTotal4);

	addAndMakeVisible(sustain_slider = new SVGKnob());
	sustain_slider->setArcFill(2.5f, 4.8f, { 237, 234, 229 });
	sustain_slider->setArcUnfilled(2.5f, 4.8f, { 122, 124, 217 });
	sustain_slider->setArcHighlightColor({ uint8(61), uint8(62), uint8(109), uint8(0.2 * 255) });
	sustain_slider->assignParameter(moduleToEdit->envelopeModule->getParameterByName("sustain"));
	sustain_slider->setName("Envelope Sustain Level");
	sustain_slider->setDescription("Sets the sustaining amplitude that is held after decay.");
	sustain_slider->setDescriptionField(infoField);
	sustain_slider->addMouseListener(this, false);
	sustain_slider->setStringConversionFunction(&elan::StringFunc3);

	addAndMakeVisible(release_slider = new SVGKnob());
	release_slider->setArcFill(2.5f, 4.8f, { 237, 234, 229 });
	release_slider->setArcUnfilled(2.5f, 4.8f, { 122, 124, 217 });
	release_slider->setArcHighlightColor({ uint8(61), uint8(62), uint8(109), uint8(0.2 * 255) });
	release_slider->assignParameter(moduleToEdit->envelopeModule->getParameterByName("release"));
	release_slider->setName("Envelope Release Time");
	release_slider->setDescription("Sets the time in seconds for the envelope release.");
	release_slider->setDescriptionField(infoField);
	release_slider->addMouseListener(this, false);
	release_slider->setStringConversionFunction(&elan::secondsToStringWithUnitTotal4);

	addAndMakeVisible(attackCurve_slider = new SVGKnob(BinaryData::KNOBdefault_svg));
	attackCurve_slider->setArcUnfilledThickness(0);
	attackCurve_slider->setArcFillThickness(0);
	attackCurve_slider->assignParameter(moduleToEdit->envelopeModule->getParameterByName("attackCurve"));
	attackCurve_slider->setName("Envelope Attack Curve");
	attackCurve_slider->setDescription("Sets the curvature of the envelope attack. Exponential / Linear / Logarithmic for negative, centered, and positive values respectively. \
Use positive values for a classic envelope shape.");
	attackCurve_slider->setDescriptionField(infoField);
	attackCurve_slider->addMouseListener(this, false);
	attackCurve_slider->setStringConversionFunction(&elan::StringFunc3);

	addAndMakeVisible(decayCurve_slider = new SVGKnob(BinaryData::KNOBdefault_svg));
	decayCurve_slider->setArcUnfilledThickness(0);
	decayCurve_slider->setArcFillThickness(0);
	decayCurve_slider->assignParameter(moduleToEdit->envelopeModule->getParameterByName("decayCurve"));
	decayCurve_slider->setName("Envelope Decay Curve");
	decayCurve_slider->setDescription("Sets the curvature of the envelope decay. Logarithmic / Linear / Exponential for negative, centered, and positive values respectively. \
Use positive values for a classic envelope shape.");
	decayCurve_slider->setDescriptionField(infoField);
	decayCurve_slider->addMouseListener(this, false);
	decayCurve_slider->setStringConversionFunction(&elan::StringFunc3);

	addAndMakeVisible(releaseCurve_slider = new SVGKnob(BinaryData::KNOBdefault_svg, BinaryData::KNOBdefault_svg));
	releaseCurve_slider->setArcUnfilledThickness(0);
	releaseCurve_slider->setArcFillThickness(0);
	releaseCurve_slider->assignParameter(moduleToEdit->envelopeModule->getParameterByName("releaseCurve"));
	releaseCurve_slider->setName("Envelope Release Curve");
	releaseCurve_slider->setDescription("Sets the curvature of the envelope release. Logarithmic / Linear / Exponential for negative, centered, and positive values respectively. \
Use positive values for a classic envelope shape.");
	releaseCurve_slider->setDescriptionField(infoField);
	releaseCurve_slider->addMouseListener(this, false);
	releaseCurve_slider->setStringConversionFunction(&elan::StringFunc3);

	addAndMakeVisible(velocityMod_slider = new SVGKnob(BinaryData::KNOBdefault_svg));
	velocityMod_slider->setArcFillThickness(0);
	velocityMod_slider->setArcUnfilledThickness(0);
	velocityMod_slider->assignParameter(moduleToEdit->envelopeModule->getParameterByName("velocityMod"));
	velocityMod_slider->setName("Ghost Click Amplitude");
	velocityMod_slider->setDescription("Sets the scale of envelope for dynamic variation with the help of ghost clicks. 1 effectively \
creates mutes and 0 means no dynamic variation.");
	velocityMod_slider->setDescriptionField(infoField);
	velocityMod_slider->addMouseListener(this, false);
	velocityMod_slider->setStringConversionFunction(&elan::StringFunc3);

	addAndMakeVisible(env_triggerOnNote_button = new elan::ImageButton(
		BinaryData::btn_light_off_png, BinaryData::btn_light_off_pngSize,
		BinaryData::btn_light_on_png, BinaryData::btn_light_on_pngSize));
	env_triggerOnNote_button->setClickableArea(13.5f, 13.5f, 15, 17);
	env_triggerOnNote_button->setButtonText("triggerOnNote");
	env_triggerOnNote_button->assignParameter(moduleToEdit->envelopeModule->getParameterByName("triggerOnNote"));
	env_triggerOnNote_button->setName("Env Trigger via Melody");
	env_triggerOnNote_button->setDescription("Allows Melody to trigger the envelope.");
	env_triggerOnNote_button->setDescriptionField(infoField);
	env_triggerOnNote_button->addMouseListener(this, false);
	env_triggerOnNote_button->stringFunc = [this]() { return env_triggerOnNote_button->getToggleState() ? "on" : "off"; };

	addAndMakeVisible(env_resetOnNote_button = new elan::ImageButton(
		BinaryData::btn_light_off_png, BinaryData::btn_light_off_pngSize,
		BinaryData::btn_light_on_png, BinaryData::btn_light_on_pngSize));
	env_resetOnNote_button->setClickableArea(13.5f, 13.5f, 15, 17);
	env_resetOnNote_button->setButtonText("resetOnNote");
	env_resetOnNote_button->assignParameter(moduleToEdit->envelopeModule->getParameterByName("resetOnNote"));
	env_resetOnNote_button->setName("Env Reset via Melody");
	env_resetOnNote_button->setDescription("Resets the envelope when triggered by Melody.");
	env_resetOnNote_button->setDescriptionField(infoField);
	env_resetOnNote_button->addMouseListener(this, false);
	env_resetOnNote_button->stringFunc = [this]() { return env_resetOnNote_button->getToggleState() ? "on" : "off"; };

	addAndMakeVisible(env_triggerOnClick_button = new elan::ImageButton(
		BinaryData::btn_light_off_png, BinaryData::btn_light_off_pngSize,
		BinaryData::btn_light_on_png, BinaryData::btn_light_on_pngSize));
	env_triggerOnClick_button->setClickableArea(13.5f, 13.5f, 15, 17);
	env_triggerOnClick_button->setButtonText("triggerOnClick");
	env_triggerOnClick_button->assignParameter(moduleToEdit->envelopeModule->getParameterByName("triggerOnClick"));
	env_triggerOnClick_button->setName("Env Trigger via Rhythm");
	env_triggerOnClick_button->setDescription("Allows Rhythm to trigger the envelope.");
	env_triggerOnClick_button->setDescriptionField(infoField);
	env_triggerOnClick_button->addMouseListener(this, false);
	env_triggerOnClick_button->stringFunc = [this]() { return env_triggerOnClick_button->getToggleState() ? "on" : "off"; };

	addAndMakeVisible(env_resetOnClick_button = new elan::ImageButton(
		BinaryData::btn_light_off_png, BinaryData::btn_light_off_pngSize,
		BinaryData::btn_light_on_png, BinaryData::btn_light_on_pngSize));
	env_resetOnClick_button->setClickableArea(13.5f, 13.5f, 15, 17);
	env_resetOnClick_button->setButtonText("resetOnClick");
	env_resetOnClick_button->assignParameter(moduleToEdit->envelopeModule->getParameterByName("resetOnClick"));
	env_resetOnClick_button->setName("Env Reset via Rhythm");
	env_resetOnClick_button->setDescription("Resets the envelope when triggered by Rhythm.");
	env_resetOnClick_button->setDescriptionField(infoField);
	env_resetOnClick_button->addMouseListener(this, false);
	env_resetOnClick_button->stringFunc = [this]() { return env_resetOnClick_button->getToggleState() ? "on" : "off"; };

	addAndMakeVisible(env_triggerOnMasterReset_button = new elan::ImageButton(
		BinaryData::btn_light_off_png, BinaryData::btn_light_off_pngSize,
		BinaryData::btn_light_on_png, BinaryData::btn_light_on_pngSize));
	env_triggerOnMasterReset_button->setClickableArea(13.5f, 13.5f, 15, 17);
	env_triggerOnMasterReset_button->setButtonText("triggerOnMasterReset");
	env_triggerOnMasterReset_button->assignParameter(moduleToEdit->envelopeModule->getParameterByName("triggerOnMasterReset"));
	env_triggerOnMasterReset_button->setName("Env Trigger via Master");
	env_triggerOnMasterReset_button->setDescription("Allows a Master reset to trigger the envelope.");
	env_triggerOnMasterReset_button->setDescriptionField(infoField);
	env_triggerOnMasterReset_button->addMouseListener(this, false);
	env_triggerOnMasterReset_button->stringFunc = [this]() { return env_triggerOnMasterReset_button->getToggleState() ? "on" : "off"; };

	addAndMakeVisible(env_resetOnMasterReset_button = new elan::ImageButton(
		BinaryData::btn_light_off_png, BinaryData::btn_light_off_pngSize,
		BinaryData::btn_light_on_png, BinaryData::btn_light_on_pngSize));
	env_resetOnMasterReset_button->setClickableArea(13.5f, 13.5f, 15, 17);
	env_resetOnMasterReset_button->setButtonText("resetOnMasterReset");
	env_resetOnMasterReset_button->assignParameter(moduleToEdit->envelopeModule->getParameterByName("resetOnMasterReset"));
	env_resetOnMasterReset_button->setName("Env Reset via Master");
	env_resetOnMasterReset_button->setDescription("Resets the envelope when triggered by Melody.");
	env_resetOnMasterReset_button->setDescriptionField(infoField);
	env_resetOnMasterReset_button->addMouseListener(this, false);
	env_resetOnMasterReset_button->stringFunc = [this]() { return env_resetOnMasterReset_button->getToggleState() ? "on" : "off"; };

	/*PITCH*/

	addAndMakeVisible(pitch_enable_button = new elan::ImageButton(
		BinaryData::btn_cdivider_off_png, BinaryData::btn_cdivider_off_pngSize,
		BinaryData::btn_cdivider_on_png, BinaryData::btn_cdivider_on_pngSize));
	pitch_enable_button->setClickableArea(21.f, 21.f, 14, 14);
	pitch_enable_button->setButtonText("enable");
	pitch_enable_button->assignParameter(moduleToEdit->pitchModule->getParameterByName("enable"));
	pitch_enable_button->setName("Melody");
	pitch_enable_button->setDescription("Sets the Melody's note selector on or off, similar to setting Octave Amplitude to 0.");
	pitch_enable_button->addMouseListener(this, false);
	pitch_enable_button->stringFunc = [this]() { return pitch_enable_button->getToggleState() ? "on" : "off"; };

	addAndMakeVisible(pitch_seed_slider = new NumberCircle(RESOURCES->fonts.tekoMedium, { 125, 123, 119 }, { 193, 193, 201 }));
	pitch_seed_slider->font.setHeight(24);
	pitch_seed_slider->offsetY = 1.f;
	pitch_seed_slider->assignParameter(moduleToEdit->pitchModule->getParameterByName("seed"));
	pitch_seed_slider->setName("Melody Seed");
	pitch_seed_slider->setDescription("Selects the random seed for Melody's note selection. \
New seed does not take affect until Melody is reset.");
	pitch_seed_slider->setDescriptionField(infoField);
	pitch_seed_slider->addMouseListener(this, false);
	pitch_seed_slider->setStringConversionFunction(&elan::StringFunc0);

	addAndMakeVisible(pitch_steps_slider = new NumberCircle(RESOURCES->fonts.tekoMedium, { 125, 123, 119 }, { 193, 193, 201 }));
	pitch_steps_slider->font.setHeight(24);
	pitch_steps_slider->offsetY = 1.f;
	pitch_steps_slider->assignParameter(moduleToEdit->pitchModule->getParameterByName("steps"));
	pitch_steps_slider->setName("Melody Steps");
	pitch_steps_slider->setDescription("Sets the number of steps before Melody resets its pattern. 0 means never reset.");
	pitch_steps_slider->setDescriptionField(infoField);
	pitch_steps_slider->addMouseListener(this, false);
	pitch_steps_slider->setStringConversionFunction(&elan::StringFunc0);

	addAndMakeVisible(pitch_division_slider = new NumberCircle(RESOURCES->fonts.tekoMedium, { 125, 123, 119 }, { 193, 193, 201 }));
	pitch_division_slider->font.setHeight(24);
	pitch_division_slider->offsetY = 1.f;
	pitch_division_slider->assignParameter(moduleToEdit->pitchModule->getParameterByName("divide"));
	pitch_division_slider->setName("Melody Clock Division");
	pitch_division_slider->setDescription("Number of times Melody must be triggered to select new note.");
	pitch_division_slider->setDescriptionField(infoField);
	pitch_division_slider->addMouseListener(this, false);
	pitch_division_slider->setStringConversionFunction(&elan::StringFunc0);

	addAndMakeVisible(pitch_manual_slider = new SVGKnob());
	pitch_manual_slider->setArcFill(2.5, 4.5, { 237, 234, 229 });
	pitch_manual_slider->setArcUnfilled(2.5, 4.5, { 61, 62, 109 });
	pitch_manual_slider->setBackgroundColor({ 139, 139, 167 });
	pitch_manual_slider->setArcHighlightColor({ uint8(61), uint8(62), uint8(109), uint8(0.2 * 255) });
	pitch_manual_slider->assignParameter(moduleToEdit->pitchModule->getParameterByName("octaveOffset"));
	pitch_manual_slider->setName("Melody Manual Offset");
	pitch_manual_slider->setDescription("Increases or decreases the octave offset which is like playing \
higher or lower notes, also known as a musical inversion.");
	pitch_manual_slider->setDescriptionField(infoField);
	pitch_manual_slider->addMouseListener(this, false);
	pitch_manual_slider->setStringConversionFunction(&elan::StringFunc3);

	addAndMakeVisible(noteLengthMod_slider = new SVGKnob(BinaryData::KNOBdefault_svg));
	noteLengthMod_slider->setArcUnfilledThickness(0);
	noteLengthMod_slider->setArcFillThickness(0);
	noteLengthMod_slider->assignParameter(moduleToEdit->pitchModule->getParameterByName("noteLengthMod"));
	noteLengthMod_slider->setName("Note Length Modifier");
	noteLengthMod_slider->setDescription("Sets the length of notes where 1 is the full length of the beat, \
and generally creates glide notes (legato) and 2 always creates glide notes. \
Lower values cause notes to stop short of a full beat and glide will have no effect.");
	noteLengthMod_slider->setDescriptionField(infoField);
	noteLengthMod_slider->addMouseListener(this, false);
	noteLengthMod_slider->setStringConversionFunction(&elan::StringFunc3);

	addAndMakeVisible(octaveAmp_slider = new SVGKnob(BinaryData::KNOBdefault_svg));
	octaveAmp_slider->setArcUnfilledThickness(0);
	octaveAmp_slider->setArcFillThickness(0);
	octaveAmp_slider->assignParameter(moduleToEdit->pitchModule->getParameterByName("octaveAmp"));
	octaveAmp_slider->setName("Octave Amplitude");
	octaveAmp_slider->setDescription("Increases the number of selectable octaves by Melody.");
	octaveAmp_slider->setDescriptionField(infoField);
	octaveAmp_slider->addMouseListener(this, false);
	octaveAmp_slider->setStringConversionFunction(&elan::StringFunc3);

	addAndMakeVisible(bipolar_button = new elan::ImageButton(
		BinaryData::btn_light_off_png, BinaryData::btn_light_off_pngSize,
		BinaryData::btn_light_on_png, BinaryData::btn_light_on_pngSize));
	bipolar_button->setClickableArea(13.5f, 13.5f, 15, 17);
	bipolar_button->setButtonText("bipolar");
	bipolar_button->assignParameter(moduleToEdit->pitchModule->getParameterByName("bipolar"));
	bipolar_button->setName("Bipolar Octaves");
	bipolar_button->setDescription("Allows Melody to select notes below the base note. \
Octave Offset will also affect whether lower notes are selected.");
	bipolar_button->setDescriptionField(infoField);
	bipolar_button->addMouseListener(this, false);
	bipolar_button->stringFunc = [this]() { return bipolar_button->getToggleState() ? "on" : "off"; };

	/*RHYTHM*/

	addAndMakeVisible(rhythm_enable_button = new elan::ImageButton(
		BinaryData::btn_cdivider_off_png, BinaryData::btn_cdivider_off_pngSize,
		BinaryData::btn_cdivider_on_png, BinaryData::btn_cdivider_on_pngSize));
	rhythm_enable_button->setClickableArea(21.f, 21.f, 14, 14);
	rhythm_enable_button->assignParameter(moduleToEdit->rhythmModule->getParameterByName("enable"));
	rhythm_enable_button->setName("Rhythm");
	rhythm_enable_button->setDescription("Sets the Rhythm module's clock selector on or off. Use the Manual knob to select clocks by hand. \
");
	rhythm_enable_button->addMouseListener(this, false);
	rhythm_enable_button->stringFunc = [this]() { return rhythm_enable_button->getToggleState() ? "on" : "off"; };

	addAndMakeVisible(rhythm_seed_slider = new NumberCircle(RESOURCES->fonts.tekoMedium, { 125, 123, 119 }, { 193, 193, 201 }));
	rhythm_seed_slider->font.setHeight(24);
	rhythm_seed_slider->offsetY = 1.f;
	rhythm_seed_slider->assignParameter(moduleToEdit->rhythmModule->getParameterByName("seed"));
	rhythm_seed_slider->setName("Rhythm Seed");
	rhythm_seed_slider->setDescription("Selects the random seed for clock selection. \
New seed does not take affect until Rhythm is reset."); 
	rhythm_seed_slider->setDescriptionField(infoField);
	rhythm_seed_slider->addMouseListener(this, false);
	rhythm_seed_slider->setStringConversionFunction(&elan::StringFunc0);

	addAndMakeVisible(rhythm_steps_slider = new NumberCircle(RESOURCES->fonts.tekoMedium, { 125, 123, 119 }, { 193, 193, 201 }));
	rhythm_steps_slider->font.setHeight(24);
	rhythm_steps_slider->offsetY = 1.f;
	rhythm_steps_slider->assignParameter(moduleToEdit->rhythmModule->getParameterByName("steps"));
	rhythm_steps_slider->setName("Rhythm Steps");
	rhythm_steps_slider->setDescription("Sets the number of steps before Rhythm resets its pattern. 0 means never reset.");
	rhythm_steps_slider->setDescriptionField(infoField);
	rhythm_steps_slider->addMouseListener(this, false);
	rhythm_steps_slider->setStringConversionFunction(&elan::StringFunc0);

	addAndMakeVisible(rhythm_division_slider = new NumberCircle(RESOURCES->fonts.tekoMedium, { 125, 123, 119 }, { 193, 193, 201 }));
	rhythm_division_slider->font.setHeight(24);
	rhythm_division_slider->offsetY = 1.f;
	rhythm_division_slider->assignParameter(moduleToEdit->rhythmModule->getParameterByName("divide"));
	rhythm_division_slider->setName("Rhythm Clock Division");
	rhythm_division_slider->setDescription("Number of times Rhythm must be triggered to trigger a new clock.");
	rhythm_division_slider->setDescriptionField(infoField);
	rhythm_division_slider->addMouseListener(this, false);
	rhythm_division_slider->setStringConversionFunction(&elan::StringFunc0);

	addAndMakeVisible(rhythm_manual_slider = new SVGKnob());
	rhythm_manual_slider->setArcFill(2.5, 4.5, { 237, 234, 229 });
	rhythm_manual_slider->setArcUnfilled(2.5, 4.5, { 61, 62, 109 });
	rhythm_manual_slider->setBackgroundColor({ 139, 139, 167 });
	rhythm_manual_slider->setArcHighlightColor({ uint8(61), uint8(62), uint8(109), uint8(0.2 * 255) });
	rhythm_manual_slider->assignParameter(moduleToEdit->rhythmModule->getParameterByName("manual"));
	rhythm_manual_slider->setName("Rhythm Manual Clock");
	rhythm_manual_slider->setDescription("Manually offset the selected clock. Only updates on when Rhythm is triggered!");
	rhythm_manual_slider->setDescriptionField(infoField);
	rhythm_manual_slider->addMouseListener(this, false);
	rhythm_manual_slider->setStringConversionFunction(&elan::StringFunc3);

	addAndMakeVisible(enableGhostClicks_button = new elan::ImageButton(
		BinaryData::btn_light_off_png, BinaryData::btn_light_off_pngSize,
		BinaryData::btn_light_on_png, BinaryData::btn_light_on_pngSize));
	enableGhostClicks_button->setClickableArea(13.5f, 13.5f, 15, 17);
	enableGhostClicks_button->setButtonText("enableGhostClicks");
	enableGhostClicks_button->assignParameter(moduleToEdit->rhythmModule->getParameterByName("enableGhostClicks"));
	enableGhostClicks_button->setName("Ghost Clicks");
	enableGhostClicks_button->setDescription("Allows ghost clicks, which are extra triggers sent by Rhythm by on selecting a second simultaneous clock. \
Adjust Envelope Vel Mod to hear the effect.");
	enableGhostClicks_button->setDescriptionField(infoField);
	enableGhostClicks_button->addMouseListener(this, false);
	enableGhostClicks_button->stringFunc = [this]() { return enableGhostClicks_button->getToggleState() ? "on" : "off"; };

	/*CLOCKS*/

	for (int i = 0; i < clockWidgetArray.size(); ++i)
	{
		addAndMakeVisible(clockWidgetArray[i] = new RaPG_ClockEditor(moduleToEdit->rhythmModule, moduleToEdit->rhythmModule->ClockModules[i]));
		slidersWithCustomInputBox.push_back(clockWidgetArray[i]->division_slider);
		slidersWithCustomInputBox.push_back(clockWidgetArray[i]->pulse1time_slider);
		slidersWithCustomInputBox.push_back(clockWidgetArray[i]->pulse2time_slider);
		clockWidgetArray[i]->addMouseListener(this, true);
		moduleToEdit->rhythmModule->ClockModules[i]->getParameterByName("enable")->registerParameterObserver(clockWidgetArray[i]->enable_button);

		addAndMakeVisible(clockLightsArray[i] = new elan::StaticSwitchImageWithText());
		clockLightsArray[i]->addMouseListener(this, false);
		clockLightsArray[i]->imageOff.setImage(RESOURCES->images.clockLightOff);
		clockLightsArray[i]->imageOn.setImage(RESOURCES->images.clockLightOn);
		clockLightsArray[i]->font = RESOURCES->fonts.tekoBold;
		clockLightsArray[i]->font.setHeight(15.f);
		clockLightsArray[i]->fontOffsetX = .5f;
		clockLightsArray[i]->fontOffsetY = 1.5f;
		clockLightsArray[i]->fontColor = Colour(0xac9eac).withAlpha(1.f);
		clockLightsArray[i]->setName("Clock Light " + String(i + 1));
		clockLightsArray[i]->setDescription("Turns bright indicating the clock is selected by Rhythm to send clicks.");
	}

	clockLightsArray[0]->text = "I";
	clockLightsArray[1]->text = "II";
	clockLightsArray[2]->text = "III";
	clockLightsArray[3]->text = "IV";
	clockLightsArray[4]->text = "V";
	clockLightsArray[5]->text = "VI";
	clockLightsArray[6]->text = "VII";

	/*ADDITIONAL SETTINGS*/

	setAsTopLevelEditor(false);
	moduleToEdit->addChangeListener(this);

	moduleToEdit->envelopeModule->getParameterByName("attack")->registerParameterObserver(envelope_display);
	moduleToEdit->envelopeModule->getParameterByName("decay")->registerParameterObserver(envelope_display);
	moduleToEdit->envelopeModule->getParameterByName("sustain")->registerParameterObserver(envelope_display);
	moduleToEdit->envelopeModule->getParameterByName("release")->registerParameterObserver(envelope_display);
	moduleToEdit->envelopeModule->getParameterByName("attackCurve")->registerParameterObserver(envelope_display);
	moduleToEdit->envelopeModule->getParameterByName("decayCurve")->registerParameterObserver(envelope_display);
	moduleToEdit->envelopeModule->getParameterByName("releaseCurve")->registerParameterObserver(envelope_display);

	slidersWithCustomInputBox.push_back(masterRate_slider);
	slidersWithCustomInputBox.push_back(paramSmooth_slider);
	slidersWithCustomInputBox.push_back(phase_slider);
	slidersWithCustomInputBox.push_back(frequency_slider);
	slidersWithCustomInputBox.push_back(pitch_slider);
	slidersWithCustomInputBox.push_back(amplitude_slider);
	slidersWithCustomInputBox.push_back(saw_slider);
	slidersWithCustomInputBox.push_back(spike_slider);
	slidersWithCustomInputBox.push_back(square_slider);
	slidersWithCustomInputBox.push_back(sine_slider);
	slidersWithCustomInputBox.push_back(knee_slider);
	slidersWithCustomInputBox.push_back(lpCut_slider);
	slidersWithCustomInputBox.push_back(ampFilterEnvMod_slider);
	slidersWithCustomInputBox.push_back(glideSpeed_slider);
	slidersWithCustomInputBox.push_back(attack_slider);
	slidersWithCustomInputBox.push_back(decay_slider);
	slidersWithCustomInputBox.push_back(sustain_slider);
	slidersWithCustomInputBox.push_back(release_slider);
	slidersWithCustomInputBox.push_back(attackCurve_slider);
	slidersWithCustomInputBox.push_back(decayCurve_slider);
	slidersWithCustomInputBox.push_back(releaseCurve_slider);
	slidersWithCustomInputBox.push_back(velocityMod_slider);
	slidersWithCustomInputBox.push_back(pitch_manual_slider);
	slidersWithCustomInputBox.push_back(noteLengthMod_slider);
	slidersWithCustomInputBox.push_back(octaveAmp_slider);

	for (const auto & slider : slidersWithCustomInputBox)
		slider->addMouseListener(this, false);

	/*INITIALIZE UI ELEMENTS*/
	
	bpm_textDisplay->setText(String(moduleToEdit->bpm, 0) + " BPM");
	envelope_display->parameterChanged(nullptr);
	setSize(backgroundImage.getImageBounds().getWidth(), backgroundImage.getImageBounds().getHeight());
}

AudioModuleEditor * RhythmAndPitchGeneratorModule::createEditor(int type)
{
	auto editor = new RhythmAndPitchGeneratorEditor(this);

	return editor;
}

void RhythmAndPitchGeneratorEditor::resized()
{
	backgroundImage.setBounds(getLocalBounds());

	{// PRESET WIDGET
		int h = presetWidget.image->getHeight();
		presetWidget.setBounds(543, 10, presetWidget.image->getWidth(), presetWidget.image->getHeight());
		presetWidget.setAlpha(0.9f);

		stateWidgetSet->setVisible(true);
		stateWidgetSet->stateFileNameLabel->setVisible(false);

		stateWidgetSet->setBounds(543, 10, presetWidget.image->getWidth(), h);		

		stateWidgetSet->stateMinusButton->setBounds(0, 0, 19, h);
		stateWidgetSet->stateMinusButton->setAlpha(.01f);

		stateWidgetSet->statePlusButton->setBounds(19, 0, 19, h);
		stateWidgetSet->statePlusButton->setAlpha(.01f);

		stateWidgetSet->stateSaveButton->setBounds(186, 0, 33, h);
		stateWidgetSet->stateSaveButton->setAlpha(.01f);

		stateWidgetSet->stateLoadButton->setBounds(218, 0, 33, h);
		stateWidgetSet->stateLoadButton->setAlpha(.01f);
	}

	GlobalParameterText->setBounds(568, 460, 204, 18);
	GlobalHelpText->setBounds(568, 479, 204, 91);

	bpm_textDisplay->setBounds(270, 48, 65, 32);

	division_slider->setBounds(353, 48, 44, 32);
	division_up->setBounds(398, 48, 10, 14);
	division_dn->setBounds(398, 66, 10, 14);

	globalResetBars_slider->setBounds(475, 48, 32, 32);
	resetbars_up->setBounds(508, 48, 10, 14);
	resetbars_dn->setBounds(508, 66, 10, 14);

	envelope_display->setAlpha(0.6f);
	envelope_display->setBounds(596+3, 112+3, 147-6, 60-6);	
	//waveform_display->setBounds(x, y, w, h);

	attack_slider->setBounds(600, 200, 29, 29);
	decay_slider->setBounds(637, 200, 29, 29);
	sustain_slider->setBounds(674, 200, 29, 29);
	release_slider->setBounds(711, 200, 29, 29);

	attackCurve_slider->setBounds(604, 241, 20, 20);
	decayCurve_slider->setBounds(641, 241, 20, 20);
	velocityMod_slider->setBounds(678, 241, 20, 20);
	releaseCurve_slider->setBounds(715, 241, 20, 20);

	pitch_enable_button->setCenteredBounds(399.5f, 535.5f, pitch_enable_button->getFileWidth(), pitch_enable_button->getFileHeight());
	pitch_seed_slider->setBounds(359, 521, 27, 27);
	pitch_steps_slider->setBounds(386, 494, 27, 27);
	pitch_division_slider->setBounds(413, 521, 27, 27);
	pitch_manual_slider->setBounds(386, 548, 27, 27);
	noteLengthMod_slider->setBounds(309, 545, 20, 20);
	octaveAmp_slider->setBounds(471, 545, 20, 20);

	glideSpeed_slider->setBounds(272, 545, 20, 20);

	phase_slider->setBounds(46, 133, 29, 29);
	frequency_slider->setBounds(83, 133, 29, 29);
	pitch_slider->setBounds(120, 133, 29, 29);
	amplitude_slider->setBounds(157, 133, 29, 29);
	lpCut_slider->setBounds(199, 138, 20, 20);
	saw_slider->setBounds(44, 181, 33, 33);
	spike_slider->setBounds(81, 181, 33, 33);
	square_slider->setBounds(118, 181, 33, 33);
	sine_slider->setBounds(155, 181, 33, 33);
	knee_slider->setBounds(198, 186, 20, 20);

	{ // all standard buttons
		int w = env_triggerOnNote_button->getFileWidth();
		int h = env_triggerOnNote_button->getFileHeight();

		env_triggerOnNote_button->setCenteredBounds(613.5f, 298.5f, w, h);
		env_resetOnNote_button->setCenteredBounds(613.5f, 316.5f, w, h);
		env_triggerOnClick_button->setCenteredBounds(613.5f, 344.5f, w, h);
		env_resetOnClick_button->setCenteredBounds(613.5f, 363.5f, w, h);
		env_triggerOnMasterReset_button->setCenteredBounds(613.5f, 392.5f, w, h);
		env_resetOnMasterReset_button->setCenteredBounds(613.5f, 411.5f, w, h);

		enableGhostClicks_button->setCenteredBounds(474.5f, 145.5f, w, h);
		pitchMod_button->setCenteredBounds(427.5f, 30.5f, w, h);
		bipolar_button->setCenteredBounds(519.5f, 554.5f, w, h);

		osc_resetOnNote_button->setCenteredBounds(65.5f, 282.5f, w, h);
		osc_resetOnClick_button->setCenteredBounds(130.5f, 282.5f, w, h);
		osc_resetOnMasterReset_button->setCenteredBounds(195.5f, 282.5f, w, h);
	}

	masterRate_slider->setBounds(418, 48, 20, 20);

	ampFilterEnvMod_slider->setBounds(110, 317, 41, 41);
	masterVolume_slider->setBounds(110, 393, 41, 41);

	oversample_slider->setBounds(152, 467, 32, 32);
	oversample_up->setBounds(185, 467, 10, 14);
	oversample_dn->setBounds(185, 485, 10, 14);

	paramSmooth_slider->setBounds(60, 467, 34, 34);

	rhythm_enable_button->setCenteredBounds(400, 179, rhythm_enable_button->getFileWidth(), rhythm_enable_button->getFileHeight());
	rhythm_seed_slider->setBounds(360, 165, 27, 27);
	rhythm_steps_slider->setBounds(387, 137, 27, 27);
	rhythm_division_slider->setBounds(414, 165, 27, 27);
	rhythm_manual_slider->setBounds(386, 192, 27, 27);

	{
		clockWidgetArray[0]->setBounds(439, 171, 82, 82);
		clockWidgetArray[1]->setBounds(473, 251, 82, 82);
		clockWidgetArray[2]->setBounds(439, 331, 82, 82);
		clockWidgetArray[3]->setBounds(359, 364, 82, 82);
		clockWidgetArray[4]->setBounds(279, 332, 82, 82);
		clockWidgetArray[5]->setBounds(246, 252, 82, 82);
		clockWidgetArray[6]->setBounds(279, 172, 82, 82);

		int w = clockLightsArray[0]->getFileWidth();
		int h = clockLightsArray[0]->getFileHeight();

		clockLightsArray[0]->setBounds(410, 238, w, h);
		clockLightsArray[1]->setBounds(423, 271, w, h);
		clockLightsArray[2]->setBounds(410, 303, w, h);
		clockLightsArray[3]->setBounds(378, 317, w, h);
		clockLightsArray[4]->setBounds(345, 303, w, h);
		clockLightsArray[5]->setBounds(332, 271, w, h);
		clockLightsArray[6]->setBounds(345, 238, w, h);
	}
}

void RhythmAndPitchGeneratorEditor::paint(Graphics & g)
{
}
