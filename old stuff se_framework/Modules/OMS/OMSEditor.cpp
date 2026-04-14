#include "OMSEditor.h"

#include "OMSBinaryData.cpp"
#include "PresetInterface.cpp"
#include "ErrorWindow.cpp"

OscEditor::OscEditor(OscilloscopeModule * module_, ParameterInfo & parameterInfo_)
	: AudioModuleEditor(module_)
	, parameterInfo(parameterInfo_)
	, module(module_)
	, display(module_)
{
	addAndMakeVisible(display);

	//addAndMakeVisible(color_dropdown);
	//color_dropdown.setName("Color");

	addAndMakeVisible(color_field = new TempBox);
	addAndMakeVisible(oned_button = new TempBox);
	addAndMakeVisible(oned_field = new TempBox);
	addAndMakeVisible(cycles_slider = new FilmStripKnob(RESOURCES->images.display_slider_filmstrip));
	addAndMakeVisible(cycles_field = new TempBox);
	addAndMakeVisible(z_slider = new FilmStripKnob(RESOURCES->images.knob4_filmstrip));
	addAndMakeVisible(brightness_slider = new FilmStripKnob(RESOURCES->images.knob4_filmstrip));
	addAndMakeVisible(afterglow_slider = new FilmStripKnob(RESOURCES->images.knob4_filmstrip));
	addAndMakeVisible(dotlimit_slider = new FilmStripKnob(RESOURCES->images.knob4_filmstrip));

	addAndMakeVisible(color_title = new TempBox);
	addAndMakeVisible(view_title = new TempBox);

	//z_title.setImage(&RESOURCES->images.icon_zdepth.getImage());
	//addAndMakeVisible(z_title);
	//z_title.setName("Z Depth");

	//brightness_title.setImage(&RESOURCES->images.icon_brightnes.getImage());
	//addAndMakeVisible(brightness_title);
	//brightness_title.setName("Brightness");

	//afterglow_title.setImage(&RESOURCES->images.icon_overglow.getImage());
	//addAndMakeVisible(afterglow_title);
	//afterglow_title.setName("Afterglow");

	//dotlimit_title.setImage(&RESOURCES->images.icon_dots.getImage());
	//addAndMakeVisible(dotlimit_title);
	//dotlimit_title.setName("Dot Limit");
}

JerobeamRadarEditor::JerobeamRadarEditor(JerobeamRadarModule * module_, OMSModule * omsModule_, ParameterInfo & parameterInfo_)
	: AudioModuleEditor(module_)
	, parameterInfo(parameterInfo_)
	, module(module_)
	, omsModule(omsModule_)
{
	addAndMakeVisible(bypass_button = new TwoStateImageButton(RESOURCES->images.led_button_small));
	bypass_button->assignParameter(module->getParameterByName("Bypass"));
	bypass_button->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	bypass_button->ParameterInfo::InfoSender::name = "Radar Bypass";
	bypass_button->ParameterInfo::InfoSender::description = "Enable / disable radar generator";
	bypass_button->ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		double value = bypass_button->getAssignedParameter()->getValue();
		return String(value > 0.5 ? "RADAR OFF" : "RADAR ON") + "( "+String(value)+" )";
	};

	addAndMakeVisible(reset_dropdown = new MenuLG());
	reset_dropdown->assignParameter(omsModule->getParameterByName("Reset_Mode"));
	reset_dropdown->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	reset_dropdown->ParameterInfo::InfoSender::name = "Reset Mode";
	reset_dropdown->ParameterInfo::InfoSender::description = "Always: Resets phase every note / Legato: when note is not tied  / One-shot: Trigger release after one oscillation.";

	addAndMakeVisible(gain_slider = new FilmStripKnob(RESOURCES->images.knob3_filmstrip));
	gain_slider->assignParameter(module->getParameterByName("RadarGain"));
	gain_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	gain_slider->ParameterInfo::InfoSender::name = "Radar Gain";
	gain_slider->ParameterInfo::InfoSender::description = "Gain of radar oscillator. Set to 0 to turn off. Useful for when you want to hear modulators through DC Offset modulation.";
	gain_slider->ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return String(gain_slider->getAssignedParameter()->getValue(), 3);
	};

	addAndMakeVisible(phase_slider = new FilmStripKnob(RESOURCES->images.knob3_filmstrip));
	phase_slider->assignParameter(module->getParameterByName("Phase"));
	phase_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	phase_slider->ParameterInfo::InfoSender::name = "Phase";
	phase_slider->ParameterInfo::InfoSender::description = "Offset of radar oscillator phase.";
	phase_slider->ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return String(phase_slider->getAssignedParameter()->getValue(), 3);
	};

	addAndMakeVisible(subphase_slider = new FilmStripKnob(RESOURCES->images.knob3_filmstrip));
	subphase_slider->assignParameter(module->getParameterByName("SubPhase"));
	subphase_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	subphase_slider->ParameterInfo::InfoSender::name = "SubPhase";
	subphase_slider->ParameterInfo::InfoSender::description = "Position of sub spiral along the main axis.";
	subphase_slider->ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return String(subphase_slider->getAssignedParameter()->getValue(), 3);
	};

	addAndMakeVisible(rotspeed_slider = new FilmStripKnob(RESOURCES->images.knob3_filmstrip));
	rotspeed_slider->assignParameter(module->getParameterByName("SubPhaseRotation"));
	rotspeed_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	rotspeed_slider->ParameterInfo::InfoSender::name = "SubPhase Rotation";
	rotspeed_slider->ParameterInfo::InfoSender::description = "Sets the speed of the sub phase rotation.";
	rotspeed_slider->ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return hertzToStringWithUnitTotal4_bipolar(rotspeed_slider->getAssignedParameter()->getValue());
	};

	addAndMakeVisible(cylinderFlat_slider = new BoxSlider());
	cylinderFlat_slider->assignParameter(module->getParameterByName("Morph"));
	cylinderFlat_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	cylinderFlat_slider->ParameterInfo::InfoSender::name = "Morph";
	cylinderFlat_slider->ParameterInfo::InfoSender::description = "Morphs between two modes. Help text will refer to 'Flat' and 'Cylinder' to describe the effect of parameters in both modes.";
	cylinderFlat_slider->ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return String(cylinderFlat_slider->getAssignedParameter()->getValue(), 3);
	};

	addAndMakeVisible(density_slider = new FilmStripKnob(RESOURCES->images.knob3_filmstrip));
	density_slider->assignParameter(module->getParameterByName("Density"));
	density_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	density_slider->ParameterInfo::InfoSender::name = "Density";
	density_slider->ParameterInfo::InfoSender::description = "Length of the spiral or number of rotations.";
	density_slider->ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return String(density_slider->getAssignedParameter()->getValue(), 3);
	};

	addAndMakeVisible(sharp_slider = new FilmStripKnob(RESOURCES->images.knob3_filmstrip));
	sharp_slider->assignParameter(module->getParameterByName("Sharp"));
	sharp_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	sharp_slider->ParameterInfo::InfoSender::name = "Sharp";
	sharp_slider->ParameterInfo::InfoSender::description = "Sharpness of internal triangle oscillator. Use to add harmonics. Triangle controls the movement of phase through the spiral.";
	sharp_slider->ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return String(sharp_slider->getAssignedParameter()->getValue(), 3);
	};

	addAndMakeVisible(curve_slider = new FilmStripKnob(RESOURCES->images.knob3_filmstrip));
	curve_slider->assignParameter(module->getParameterByName("Sharp_Curve"));
	curve_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	curve_slider->ParameterInfo::InfoSender::name = "Sharp Curve";
	curve_slider->ParameterInfo::InfoSender::description = "Controls the sharpness curve of triangle oscillator. Note: Curve buttons '/' '\' must be enabled.";
	curve_slider->ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return String(curve_slider->getAssignedParameter()->getValue(), 3);
	};

	addAndMakeVisible(shade_slider = new FilmStripKnob(RESOURCES->images.knob3_filmstrip));
	shade_slider->assignParameter(module->getParameterByName("Shade"));
	shade_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	shade_slider->ParameterInfo::InfoSender::name = "Shade";
	shade_slider->ParameterInfo::InfoSender::description = "Causes the start of the spiral to be accentuated and an exaggerated radar visual. Makes the sound nasaly.";
	shade_slider->ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return String(shade_slider->getAssignedParameter()->getValue(), 3);
	};

	addAndMakeVisible(curveLeading_button = new TwoStateImageButton(RESOURCES->images.led_button_small));
	curveLeading_button->assignParameter(module->getParameterByName("Curve_Start"));
	curveLeading_button->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	curveLeading_button->ParameterInfo::InfoSender::name = "Curve Start";
	curveLeading_button->ParameterInfo::InfoSender::description = "Enables sharpness curve for the first half of triangle phase.";
	curveLeading_button->ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return "";
	};

	addAndMakeVisible(curveTrailing_button = new TwoStateImageButton(RESOURCES->images.led_button_small));
	curveTrailing_button->assignParameter(module->getParameterByName("Curve_End"));
	curveTrailing_button->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	curveTrailing_button->ParameterInfo::InfoSender::name = "Curve End";
	curveTrailing_button->ParameterInfo::InfoSender::description = "Enables sharpness curve for the second half of triangle phase.";
	curveTrailing_button->ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return "";
	};

	addAndMakeVisible(direction_slider = new FilmStripKnob(RESOURCES->images.knob1_filmstrip));
	direction_slider->assignParameter(module->getParameterByName("Direction"));
	direction_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	direction_slider->ParameterInfo::InfoSender::name = "Direction";
	direction_slider->ParameterInfo::InfoSender::description = "Affects how spiral arms connect ranging from smooth, sharp, and discontinuous. Enable Pow2Bend to prevent discontinuity.";
	direction_slider->ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return String(direction_slider->getAssignedParameter()->getValue(), 3);
	};

	addAndMakeVisible(lap_slider = new FilmStripKnob(RESOURCES->images.knob3_filmstrip));
	lap_slider->assignParameter(module->getParameterByName("Lap"));
	lap_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	lap_slider->ParameterInfo::InfoSender::name = "Lap";
	lap_slider->ParameterInfo::InfoSender::description = "Number of spiral rotations. Creates \"scanning visuals\" when used with Rotation. Non-integer causes discontinuty.";
	lap_slider->ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return valueToStringWithPlusMinusSign(lap_slider->getAssignedParameter()->getValue(), 3);
	};

	addAndMakeVisible(bend_button = new TwoStateImageButton(RESOURCES->images.led_button_small));
	bend_button->assignParameter(module->getParameterByName("Bend"));
	bend_button->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	bend_button->ParameterInfo::InfoSender::name = "Bend";
	bend_button->ParameterInfo::InfoSender::description = "Bends spiral arms to give the appearance of a scan line.";
	bend_button->ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return bend_button->getAssignedParameter()->getValue() > 0.5 ? "ON" : "OFF";
	};

	addAndMakeVisible(innerLenght_slider = new BoxSlider());
	innerLenght_slider->assignParameter(module->getParameterByName("Length"));
	innerLenght_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	innerLenght_slider->ParameterInfo::InfoSender::name = "Length";
	innerLenght_slider->ParameterInfo::InfoSender::description = "Lenght of inner spiral";
	innerLenght_slider->ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return String(innerLenght_slider->getAssignedParameter()->getValue(), 3);
	};

	addAndMakeVisible(innerZDepth_slider = new BoxSlider());
	innerZDepth_slider->assignParameter(module->getParameterByName("Inner"));
	innerZDepth_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	innerZDepth_slider->ParameterInfo::InfoSender::name = "Inner";
	innerZDepth_slider->ParameterInfo::InfoSender::description = "Flat: Changes the end point of the inner spiral. Value of 1 will create a circle. Cylinder: Sharpens the spiral protrusion";
	innerZDepth_slider->ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return String(innerZDepth_slider->getAssignedParameter()->getValue(), 3);
	};

	addAndMakeVisible(outerLenght_slider = new BoxSlider());
	outerLenght_slider->assignParameter(module->getParameterByName("FrontRing"));
	outerLenght_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	outerLenght_slider->ParameterInfo::InfoSender::name = "FrontRing";
	outerLenght_slider->ParameterInfo::InfoSender::description = "Lenght of outer spiral";
	outerLenght_slider->ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return String(outerLenght_slider->getAssignedParameter()->getValue(), 3);
	};

	addAndMakeVisible(outerZDepth_slider = new BoxSlider());
	outerZDepth_slider->assignParameter(module->getParameterByName("ZDepth"));
	outerZDepth_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	outerZDepth_slider->ParameterInfo::InfoSender::name = "ZDepth";
	outerZDepth_slider->ParameterInfo::InfoSender::description = "Flat: Squeezes the outer spiral arms toward the center. Cylinder: Creates illusion of depth by skewing size and speed of outer spiral.";
	outerZDepth_slider->ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return String(outerZDepth_slider->getAssignedParameter()->getValue(), 3);
	};

	addAndMakeVisible(protrudeX_slider = new BoxSlider());
	protrudeX_slider->assignParameter(module->getParameterByName("Protrude_X"));
	protrudeX_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	protrudeX_slider->ParameterInfo::InfoSender::name = "Protrude X";
	protrudeX_slider->ParameterInfo::InfoSender::description = "Flat: changes the X angle of protrusion. Cylinder: rotates or folds cylinder onto itself on the X axis like a slinky.";
	protrudeX_slider->ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return String(protrudeX_slider->getAssignedParameter()->getValue(), 3);
	};

	addAndMakeVisible(protrudeY_slider = new BoxSlider());
	protrudeY_slider->assignParameter(module->getParameterByName("Protrude_Y"));
	protrudeY_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	protrudeY_slider->ParameterInfo::InfoSender::name = "Protrude Y";
	protrudeY_slider->ParameterInfo::InfoSender::description = "Flat: changes the Y angle of protrusion. Cylinder: rotates or folds cylinder into itself on the Y axis like a slinky.";
	protrudeY_slider->ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return String(protrudeY_slider->getAssignedParameter()->getValue(), 3);
	};

	addAndMakeVisible(protrudeRatio_slider = new FilmStripKnob(RESOURCES->images.knob1_filmstrip));
	protrudeRatio_slider->assignParameter(module->getParameterByName("Ratio"));
	protrudeRatio_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	protrudeRatio_slider->ParameterInfo::InfoSender::name = "Ratio";
	protrudeRatio_slider->ParameterInfo::InfoSender::description = "Flat: increases the protrusion length of X/Y protrusion. Cylinder: controls cylinder width.";
	protrudeRatio_slider->ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return String(protrudeRatio_slider->getAssignedParameter()->getValue(), 3);
	};

	addAndMakeVisible(ringcut_button = new TwoStateImageButton(RESOURCES->images.led_button_small));
	ringcut_button->assignParameter(module->getParameterByName("RingCut"));
	ringcut_button->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	ringcut_button->ParameterInfo::InfoSender::name = "RingCut";
	ringcut_button->ParameterInfo::InfoSender::description = "Enables discreet circles to be drawn instead of spirals. This causes discontinuity.";
	ringcut_button->ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return ringcut_button->getAssignedParameter()->getValue() > 0.5 ? "ON" : "OFF";
	};

	addAndMakeVisible(phaseInv_button = new TwoStateImageButton(RESOURCES->images.led_button_small));
	phaseInv_button->assignParameter(module->getParameterByName("PhaseInv"));
	phaseInv_button->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	phaseInv_button->ParameterInfo::InfoSender::name = "PhaseInv";
	phaseInv_button->ParameterInfo::InfoSender::description = "Invert overall phase of radar oscillator";
	phaseInv_button->ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return phaseInv_button->getAssignedParameter()->getValue() > 0.5 ? "ON" : "OFF";
	};

	addAndMakeVisible(spiralReturn_button = new TwoStateImageButton(RESOURCES->images.led_button_small));
	spiralReturn_button->assignParameter(module->getParameterByName("SpiralReturn"));
	spiralReturn_button->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	spiralReturn_button->ParameterInfo::InfoSender::name = "SpiralReturn";
	spiralReturn_button->ParameterInfo::InfoSender::description = "Enables radar to jumps to the next spiral arm instead of retracing. This causes discontinuity";
	spiralReturn_button->ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return spiralReturn_button->getAssignedParameter()->getValue() > 0.5 ? "ON" : "OFF";
	};

	addAndMakeVisible(tunnelInv_button = new TwoStateImageButton(RESOURCES->images.led_button_small));
	tunnelInv_button->assignParameter(module->getParameterByName("TunnelInv"));
	tunnelInv_button->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	tunnelInv_button->ParameterInfo::InfoSender::name = "TunnelInv";
	tunnelInv_button->ParameterInfo::InfoSender::description = "Flips the radar oscillator in a \"mirror image\" kind of way.";
	tunnelInv_button->ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return tunnelInv_button->getAssignedParameter()->getValue() > 0.5 ? "ON" : "OFF";
	};
}

OMSEditor::OMSEditor(OMSModule * module_)
	: BasicEditorWithLicensing(module_, this, "Radar Generator", "RadarGenerator", "https://www.soundemote.com/plugins/radar-generator", "http://www.elanhickler.com/Soundemote/Soundemote_-_Radar_Generator.zip")
	, module(module_)
{
	ScopedLock scopedLock(*lock);

	parameterInfo.addListener(this);
	module->addChangeListener(this);

	keyFileButton->setVisible(false);
	setHeadlineStyle(headlineStyles::NO_HEADLINE);

	//EDITORS
	addAndMakeVisible(radarGenerator_editor = new JerobeamRadarEditor(module->radarModule, module, parameterInfo));

	addAndMakeVisible(oscilloscopeEditor = new OscilloscopeEditor(module->oscilloscopeModule));
	addChildEditor(oscilloscopeEditor);
	oscilloscopeEditor->changeListenerCallback(nullptr);
	oscilloscopeEditor->addMouseListener(this, true);
	addMouseListener(&oscilloscopeArea, false);
	oscilloscopeArea.setInterceptsMouseClicks(false, true);

	addAndMakeVisible(psedit = new PrettyScopeEditor(module->psmod));

	//SECTION TABS
	/*fx*/
	addAndMakeVisible(fx_tab);
	addAndMakeVisible(fx_page);
	sectionTabHandler.addTab(&fx_tab);
	fx_tab.addAssociatedComponentsForShowHide(&fx_page);

	//VALUE INPUT
	valueEntry.setSize(100, 26);
	valueEntry.setAlpha(0.9f);
	parameterValueInput.setTextEditorBoundsFunction = [this]()
	{
		auto bounds = parameterValueInput.state.componentBounds;
		valueEntry.drawWithLeftArrow();

		valueEntry.setBounds(ElanRect::place(valueEntry.getBounds(), parameterValueInput.state.componentBounds, Anchor::MidLeft, Anchor::MidRight));

		if (valueEntry.getRight() > getWidth())
		{
			valueEntry.drawWithRightArrow();

			valueEntry.setBounds(ElanRect::place(valueEntry.getBounds(), parameterValueInput.state.componentBounds, Anchor::MidRight, Anchor::MidLeft));
		}
	};	

	//HELP
	addAndMakeVisible(ParameterDescriptionText);
	ParameterDescriptionText.setFont(RESOURCES->fonts.RobotoLight);
	ParameterDescriptionText.setFontColor({ 216, 216, 216 });
	ParameterDescriptionText.setFontPointHeight(8.f);
	ParameterDescriptionText.setFontJustification(juce::Justification::topLeft);
	ParameterDescriptionText.setDrawMultilineText(true);
	ParameterDescriptionText.setText("Description");

	addAndMakeVisible(ParameterValueText);
	ParameterValueText.setFont(RESOURCES->fonts.TekoSemiBold);
	ParameterValueText.setFontColor({ 191, 191, 191 });
	ParameterValueText.setFontPointHeight(18.f);
	ParameterValueText.setDrawMultilineText(false);
	ParameterValueText.setDoDrawFittedText(true);
	ParameterValueText.setText("1.0000");

	addAndMakeVisible(ParameterNameText);
	ParameterNameText.setFont(RESOURCES->fonts.TekoMedium);
	ParameterNameText.setFontColor({ 143, 143, 143 });
	ParameterNameText.setFontPointHeight(16.f);
	ParameterNameText.setDrawMultilineText(false);
	ParameterNameText.setDoDrawFittedText(true);
	ParameterNameText.setText("Name");

	//MAIN
	addAndMakeVisible(channelmode_dropdown = new MenuLG());
	channelmode_dropdown->assignParameter(module->getParameterByName("Channel_Mode"));
	channelmode_dropdown->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	channelmode_dropdown->ParameterInfo::InfoSender::name = "Channel Mode";
	channelmode_dropdown->ParameterInfo::InfoSender::description = "Options for summing output to mono.";

	addAndMakeVisible(clipping_soft_button = new TwoStateImageButton(RESOURCES->images.led_button_big));
	clipping_soft_button->assignParameter(module->getParameterByName("Clip_Mode"));
	clipping_soft_button->setParameterInfoReceiver(&parameterInfo);
	clipping_soft_button->ParameterInfo::InfoSender::name = "Clip Mode";
	clipping_soft_button->ParameterInfo::InfoSender::description = "Soft clipping will affect signal before limit is reached. Set Clip Level to 1.5 for more headroom.";
	clipping_soft_button->ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return clipping_soft_button->getAssignedParameter()->getValue() > 0.5 ? "Soft" : "Hard";
	};

	addAndMakeVisible(clipping_hard_button = new TwoStateImageButton(RESOURCES->images.led_button_big));
	clipping_hard_button->setStateForOn(0);
	clipping_hard_button->assignParameter(clipping_soft_button->getAssignedParameter());
	clipping_hard_button->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	clipping_hard_button->ParameterInfo::InfoSender::sender = clipping_soft_button;

	addAndMakeVisible(clipping_gain_slider = new FilmStripKnob(RESOURCES->images.knob1_filmstrip));
	clipping_gain_slider->assignParameter(module->getParameterByName("Gain"));
	clipping_gain_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	clipping_gain_slider->ParameterInfo::InfoSender::name = "Gain";
	clipping_gain_slider->ParameterInfo::InfoSender::description = "Gain for the entire synth. Signal will be clipped at Clip Level amplitude.";
	clipping_gain_slider->ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return String(clipping_gain_slider->getAssignedParameter()->getValue(), 3);
	};

	addAndMakeVisible(clipping_limit_slider = new FilmStripKnob(RESOURCES->images.knob2_filmstrip));
	clipping_limit_slider->assignParameter(module->getParameterByName("Clip_Level"));
	clipping_limit_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	clipping_limit_slider->ParameterInfo::InfoSender::name = "Clip Level";
	clipping_limit_slider->ParameterInfo::InfoSender::description = "Sets the point of clipping. Soft clipping will affect signal before limit is reached.";
	clipping_limit_slider->ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return String(clipping_limit_slider->getAssignedParameter()->getValue(), 3);
	};

	addAndMakeVisible(parameterSmoothing_slider = new FilmStripKnob(RESOURCES->images.knob2_filmstrip));
	parameterSmoothing_slider->assignParameter(module->getParameterByName("Parameter_Smoothing"));
	parameterSmoothing_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	parameterSmoothing_slider->ParameterInfo::InfoSender::name = "Parameter Smoothing";
	parameterSmoothing_slider->ParameterInfo::InfoSender::description = "Amount of smoothing applied to sliders / controls, as well as host automation.";
	parameterSmoothing_slider->ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return elan::secondsToStringWithUnitTotal4(parameterSmoothing_slider->getAssignedParameter()->getValue());
	};

	addAndMakeVisible(oversample_slider = new NumberDisplay());
	oversample_slider->setMouseScaling(.005);
	oversample_slider->assignParameter(module->getParameterByName("Oversampling"));
	oversample_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	oversample_slider->ParameterInfo::InfoSender::name = "Oversampling";
	oversample_slider->ParameterInfo::InfoSender::description = "Applies antialiasing, increases CPU usage, and distorts image if viewed on oscilloscope. Set to x2 if not recording video.";
	oversample_slider->stringFunc = [this]() { return "x" + String(oversample_slider->getValue(), 0); };
	oversample_slider->ParameterInfo::InfoSender::valueStringFunction = oversample_slider->stringFunc;	

	addAndMakeVisible(oversample_up = new ClickableArea());
	oversample_up->setIncrementAmount(+1);
	oversample_up->assignParameter(oversample_slider->getAssignedParameter());
	//oversample_up->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	//oversample_up->ParameterInfo::InfoSender::sender = oversample_slider;

	addAndMakeVisible(oversample_dn = new ClickableArea());
	oversample_dn->setIncrementAmount(-1);
	oversample_dn->assignParameter(oversample_slider->getAssignedParameter());
	//oversample_dn->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	//oversample_dn->ParameterInfo::InfoSender::sender = oversample_slider;

	addAndMakeVisible(rotate_slider = new FilmStripKnob(RESOURCES->images.knob1_filmstrip));
	rotate_slider->assignParameter(module->getParameterByName("Stereo_Rotate"));
	rotate_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	rotate_slider->ParameterInfo::InfoSender::name = "Stereo Rotate";
	rotate_slider->ParameterInfo::InfoSender::description = "Rotates stereo image similar to panning. Will cause mono to become stereo.";
	rotate_slider->ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return valueToStringWithPlusMinusSign(rotate_slider->getAssignedParameter()->getValue(), 3);
	};

	addAndMakeVisible(dc_offset_slider = new FilmStripKnob(RESOURCES->images.knob1_filmstrip));
	dc_offset_slider->assignParameter(module->getParameterByName("DC_Offset"));
	dc_offset_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	dc_offset_slider->ParameterInfo::InfoSender::name = "DC Offset";
	dc_offset_slider->ParameterInfo::InfoSender::description = "Monophonic DC offset (affecting left/right channels equally) and is affected by rotation and clipping.";
	dc_offset_slider->ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return String(dc_offset_slider->getAssignedParameter()->getValue(), 3);
	};

	addAndMakeVisible(dc_X_slider = new FilmStripKnob(RESOURCES->images.knob2_filmstrip));
	dc_X_slider->assignParameter(module->getParameterByName("X_Offset"));
	dc_X_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	dc_X_slider->ParameterInfo::InfoSender::name = "X Offset";
	dc_X_slider->ParameterInfo::InfoSender::description = "DC offset of left channel and is affected by rotation and clipping.";
	dc_X_slider->ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return valueToStringWithPlusMinusSign(dc_X_slider->getAssignedParameter()->getValue(), 3);
	};

	addAndMakeVisible(dc_Y_slider = new FilmStripKnob(RESOURCES->images.knob2_filmstrip));
	dc_Y_slider->assignParameter(module->getParameterByName("Y_Offset"));
	dc_Y_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	dc_Y_slider->ParameterInfo::InfoSender::name = "Y Offset";
	dc_Y_slider->ParameterInfo::InfoSender::description = "DC offset of right channel and is affected by rotation and clipping.";
	dc_Y_slider->ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return valueToStringWithPlusMinusSign(dc_Y_slider->getAssignedParameter()->getValue(), 3);
	};

	addAndMakeVisible(chaos_source_dropdown = new MenuLG());
	chaos_source_dropdown->assignParameter(module->getParameterByName("Feedback_Source"));
	chaos_source_dropdown->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	chaos_source_dropdown->ParameterInfo::InfoSender::name = "Feedback Source";
	chaos_source_dropdown->ParameterInfo::InfoSender::description = "Ramp: upwards sawtooth derived from oscillator timer / Triangle: Same as ramp except affected by Sharp control.";

	addAndMakeVisible(chaos_LP_slider = new FilmStripKnob(RESOURCES->images.knob2_filmstrip));
	chaos_LP_slider->assignParameter(module->getParameterByName("LP_Feedback"));
	chaos_LP_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	chaos_LP_slider->ParameterInfo::InfoSender::name = "LP Feedback";
	chaos_LP_slider->ParameterInfo::InfoSender::description = "Lowpass filter applied to feedback source. Use to reduce harmonic content.";
	chaos_LP_slider->ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return elan::hertzToStringLowpassFilter(chaos_LP_slider->getAssignedParameter()->getValue());
	};

	addAndMakeVisible(chaos_HP_slider = new FilmStripKnob(RESOURCES->images.knob2_filmstrip));
	chaos_HP_slider->assignParameter(module->getParameterByName("HP_Feedback"));
	chaos_HP_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	chaos_HP_slider->ParameterInfo::InfoSender::name = "HP Feedback";
	chaos_HP_slider->ParameterInfo::InfoSender::description = "Highpass filter applied to feedback source. Important for when modulating frequency to remain pitch centered.";
	chaos_HP_slider->ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return elan::hertzToStringHighpassFilter(chaos_HP_slider->getAssignedParameter()->getValue());
	};

	addAndMakeVisible(chaos_amount_slider = new FilmStripKnob(RESOURCES->images.knob1_filmstrip));
	chaos_amount_slider->assignParameter(module->getParameterByName("Feedback_Amount"));
	chaos_amount_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	chaos_amount_slider->ParameterInfo::InfoSender::name = "Feedback Amount";
	chaos_amount_slider->ParameterInfo::InfoSender::description = "Feedback gain. To use feedback: right-click on slider -> modulation setup -> add Feedback Source.";
	chaos_amount_slider->ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return String(chaos_amount_slider->getAssignedParameter()->getValue(), 3);
	};

	presetInterface.presetPage.load_preset_dropdown.setImage(&RESOURCES->images.icon_load.getImage());
	addAndMakeVisible(presetInterface.presetPage.load_preset_dropdown);
	presetInterface.presetPage.load_preset_dropdown.setName("name");
	presetInterface.presetPage.load_preset_dropdown.clickedFunc = [this]()
	{
		errorWindow.showErrorMessage("ERROR", "This is an error message yo yo.");
		//if (fileToSaveTo.existsAsFile())
		//{
		//	File tmpFile = fileToSaveTo.getNonexistentSibling();
		//	tmpFile.create();
		//	tmpFile.appendText(myXmlDoc);
		//	fileToSaveTo.deleteFile();
		//	tmpFile.moveFileTo(fileToSaveTo);
		//}
		//else
		//{
		//	fileToSaveTo.create();
		//	fileToSaveTo.appendText(myXmlDoc);
		//}

		//updateFileList();
		//delete xmlState;
		//setStateName(fileToSaveTo.getFileNameWithoutExtension(), true);
		////markStateAsClean();
		//return true;

		//bool result = saveStateToXmlFile(fileToSaveTo);
		//if (result == true)
		//	markFileAsClean(true);
		//notifyListeners();
		//return result;


		////FileChooser chooser(dialogTitle, getActiveDirectory(), wildcardPatterns, true);
		////if (chooser.browseForFileToSave(true))
		////{
		//	File fileToSaveTo = chooser.getResult();
		//	if (!fileToSaveTo.hasFileExtension(defaultExtension) && defaultExtension != String::empty)
		//		fileToSaveTo = fileToSaveTo.withFileExtension(defaultExtension);
		//	bool result = saveToFile(fileToSaveTo);
		//	if (result == true)
		//	{
		//		setActiveFile(fileToSaveTo);
		//		return true;
		//	}
		//	//else
		//		//return false;
		////}
		////else
		//	//return false;
	};
	//load_preset_dropdown.setDescription("description");

	presetInterface.presetPage.save_preset_dropdown.setImage(&RESOURCES->images.icon_save.getImage());
	addAndMakeVisible(presetInterface.presetPage.save_preset_dropdown);
	presetInterface.presetPage.save_preset_dropdown.setName("name");
	presetInterface.presetPage.save_preset_dropdown.clickedFunc = [this]()
	{
		presetInterface.setVisible(true);
	};
	//save_preset_dropdown.setDescription("description");

	presetInterface.presetPage.manage_preset_dropdown.setImage(&RESOURCES->images.icon_presetb.getImage());
	addAndMakeVisible(presetInterface.presetPage.manage_preset_dropdown);
	presetInterface.presetPage.manage_preset_dropdown.setName("name");
	presetInterface.presetPage.manage_preset_dropdown.clickedFunc = [this]()
	{
		MenuHelper::clear(mainMenu);

		if (isInDemoMode)
		{
			MenuHelper::addCallbackItem(mainMenu, "BUY NOW to remove demo limitations!", [this]()
			{
				if (!URL(pluginFileManager.info.moduleWebsite).launchInDefaultBrowser())
				{
					showAlertBox("Error launching website!", "Could not launch website: \n\n" + pluginFileManager.info.moduleWebsite);
				}
			});
		}

		MenuHelper::addCallbackItem(mainMenu, "Open Plugin Registration dialog", [this]()
		{
			BasicEditorWithLicensing::showRegistrationDialog();
		});		

		MenuHelper::addSeperator(mainMenu);

		MenuHelper::addCallbackItem(mainMenu, "Open User Presets folder", [this]()
		{
			try { FileHelper::openFolder(getUserPresetFolder(), true); }
			catch (...)
			{
				showAlertBox("Error opening folder!", FileHelper::getErrorString());
			}
		});

		MenuHelper::addCallbackItem(mainMenu, "Open Factory Presets folder", [this]()
		{
			try { FileHelper::openFolder(getFactoryPresetFolder(), true); }
			catch (...)
			{
				showAlertBox("Error opening folder!", FileHelper::getErrorString() + "\n\n" + getFactoryPresetFolder() + "\n\nThis error may occur due to permissions. Make sure the folder exists. If not, reinstall the plugin's factory presets.");
			}
		});

		MenuHelper::addCallbackItem(mainMenu, "Open Key Files folder", [this]()
		{
			try { FileHelper::openFolder(pluginFileManager.userFolder.keys, true); }
			catch (...)
			{
				showAlertBox("Error opening folder!", FileHelper::getErrorString());
			}
		});

		MenuHelper::addSeperator(mainMenu);

		MenuHelper::addCallbackItem(mainMenu, "Re-download Plugin (if you need to update)", [this]()
		{
			if (!URL(pluginFileManager.info.moduleDownloadLink).launchInDefaultBrowser())
			{
				showAlertBox("Error downloading file!", "Could not launch download link: " + pluginFileManager.info.moduleWebsite);
			}
		});

		MenuHelper::addSeperator(mainMenu);

		MenuHelper::addGreyedOutItem(mainMenu, calculateHeadlineText());

		MenuHelper::show(&mainMenu, &presetInterface.presetPage.manage_preset_dropdown);
	};

	//manage_preset_dropdown.setDescription("description");	

	presetInterface.presetPage.save_button.clickedFunc = [this]()
	{
		presetInterface.presetPage.updateState();

		presetInterface.setVisible(false);

		XmlElement* xmlState = module->getStateAsXml(PresetInformation::SenderReceiver::getState().name, true);
		String myXmlDoc = xmlState->createDocument(String());

		File fileToSaveTo = File(
			getUserPresetFolder() + "/" +
			PresetInformation::SenderReceiver::getState().category + "/" +
			PresetInformation::SenderReceiver::getState().name + ".xml"
		);

		File tmpFile = fileToSaveTo.getNonexistentSibling();
		tmpFile.create();
		tmpFile.appendText(myXmlDoc);
		tmpFile.moveFileTo(fileToSaveTo);
	};

	presetInterface.presetPage.rename_button.clickedFunc = [this]()
	{
		presetInterface.setVisible(false);
	};

	presetInterface.presetPage.delete_button.clickedFunc = [this]()
	{
		presetInterface.setVisible(false);
	};

	addAndMakeVisible(keytracking_button = new TwoStateImageButton(RESOURCES->images.led_button_big));
	keytracking_button->setStateForOn(0);
	keytracking_button->assignParameter(module->getParameterByName("Frequency_Mode"));
	keytracking_button->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	keytracking_button->ParameterInfo::InfoSender::name = "Keytracking";
	keytracking_button->ParameterInfo::InfoSender::description = "Allows pitch to correspond to midi note. If off, frequency is 0 unless offset by other frequency parameters.";
	keytracking_button->ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return keytracking_button->getAssignedParameter()->getValue() > 0.5 ? "ON" : "OFF";
	};

	addAndMakeVisible(changePitch_always_button = new TwoStateImageButton(RESOURCES->images.led_button_big));
	changePitch_always_button->assignParameter(module->getParameterByName("Note_Change_Mode"));
	changePitch_always_button->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	changePitch_always_button->ParameterInfo::InfoSender::name = "Note Change Mode";
	changePitch_always_button->ParameterInfo::InfoSender::description = "Always: Note changed as expected for legato playing. On Note: You must re-press a key to cause note to change, useful for percussion.";
	changePitch_always_button->ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return changePitch_always_button->getAssignedParameter()->getValue() > 0.5 ? "ON NOTE" : "LEGATO";
	};

	addAndMakeVisible(changePitch_legato_button = new TwoStateImageButton(RESOURCES->images.led_button_big));
	changePitch_legato_button->setStateForOn(0);
	changePitch_legato_button->assignParameter(changePitch_always_button->getAssignedParameter());
	changePitch_legato_button->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	changePitch_legato_button->ParameterInfo::InfoSender::sender = changePitch_always_button;

	addAndMakeVisible(tempo_reset_button = new TwoStateImageButton(RESOURCES->images.led_button_big));
	tempo_reset_button->assignParameter(module->getParameterByName("Reset_on_tempo_change"));
	tempo_reset_button->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	tempo_reset_button->ParameterInfo::InfoSender::name = "Reset on tempo change";
	tempo_reset_button->ParameterInfo::InfoSender::description = "Help keep things on tempo by resetting phase when tempo settings change.";
	tempo_reset_button->ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return tempo_reset_button->getAssignedParameter()->getValue() > 0.5 ? "ON" : "OFF";
	};

	addAndMakeVisible(tempo_freq_slider = new FilmStripKnob(RESOURCES->images.knob1_filmstrip));
	tempo_freq_slider->assignParameter(module->getParameterByName("Frequency"));
	tempo_freq_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	tempo_freq_slider->ParameterInfo::InfoSender::name = "Frequency";
	tempo_freq_slider->ParameterInfo::InfoSender::description = "Offsets pitch linearly. Allows for off-pitch beating. Modulate with LFO for classic \"in-tune\" FM, or with feedback for growls.";
	tempo_freq_slider->ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return elan::hertzToStringWithUnitTotal4_bipolar(tempo_freq_slider->getAssignedParameter()->getValue());
	};

	addAndMakeVisible(tempo_tempo_slider = new FilmStripKnob(RESOURCES->images.knob1_filmstrip));
	tempo_tempo_slider->assignParameter(module->getParameterByName("Tempo"));
	tempo_tempo_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	tempo_tempo_slider->ParameterInfo::InfoSender::name = "Tempo";
	tempo_tempo_slider->ParameterInfo::InfoSender::description = "Choose a beat multiplier. Will offset pitch unless set to 0/0. x1 = whole / x4 = quarter / x5.333 = quater dotted.";
	tempo_tempo_slider->ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return elan::tempoMulToString(tempo_tempo_slider->getAssignedParameter()->getValue());
	};

	addAndMakeVisible(tempo_multiply_slider = new FilmStripKnob(RESOURCES->images.knob1_filmstrip));
	tempo_multiply_slider->assignParameter(module->getParameterByName("Tempo_Multiply"));
	tempo_multiply_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	tempo_multiply_slider->ParameterInfo::InfoSender::name = "Tempo Multiply";
	tempo_multiply_slider->ParameterInfo::InfoSender::description = "Doubles or halves the tempo setting. This does not affect LFOs/ADSRs/Breakpoints.";
	tempo_multiply_slider->ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return beatMulToString(tempo_multiply_slider->getAssignedParameter()->getValue());
	};

	addAndMakeVisible(masterRate_slider = new FilmStripKnob(RESOURCES->images.knob1_filmstrip));
	masterRate_slider->assignParameter(module->getParameterByName("Frequency_Multiply"));
	masterRate_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	masterRate_slider->ParameterInfo::InfoSender::name = "Master Rate";
	masterRate_slider->ParameterInfo::InfoSender::description = "Multiplies the frequency of LFOs and synth. Modulating this with an LFO will cause frequency feedback. Try it!";
	masterRate_slider->ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		double v = masterRate_slider->getAssignedParameter()->getValue();
		if (v > 1)
			return "x" + String(v, 3) + " ff";
		if (v == 1)
			return (String)"x1.000 play";
		if (v == 0)
			return (String)"frozen (let it go!)";
		if (v < 0)
			return "x" + String(std::abs(v), 3) + " rw";

		return "x" + String(v, 3) + " slo";
	};

	addAndMakeVisible(glide_always_button = new TwoStateImageButton(RESOURCES->images.led_button_big));
	glide_always_button->assignParameter(module->getParameterByName("Glide_Mode"));
	glide_always_button->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	glide_always_button->ParameterInfo::InfoSender::name = "Glide Mode";
	glide_always_button->ParameterInfo::InfoSender::description = "Choose when to apply pitch glide.";
	glide_always_button->ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return glide_always_button->getAssignedParameter()->getValue() < 0.5 ? "LEGATO" : "ALWAYS";
	};

	addAndMakeVisible(glide_legato_button = new TwoStateImageButton(RESOURCES->images.led_button_big));
	glide_legato_button->setStateForOn(0);
	glide_legato_button->assignParameter(module->getParameterByName("Glide_Mode"));
	glide_legato_button->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	glide_legato_button->ParameterInfo::InfoSender::sender = glide_always_button;

	addAndMakeVisible(pitch_glide_slider = new FilmStripKnob(RESOURCES->images.knob1_filmstrip));
	pitch_glide_slider->assignParameter(module->getParameterByName("Glide_Amount"));
	pitch_glide_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	pitch_glide_slider->ParameterInfo::InfoSender::name = "Glide Amount";
	pitch_glide_slider->ParameterInfo::InfoSender::description = "Speed of transition between pitches.";
	pitch_glide_slider->ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return elan::secondsToStringWithUnitTotal4(pitch_glide_slider->getAssignedParameter()->getValue());
	};

	addAndMakeVisible(pitch_tune_slider = new FilmStripKnob(RESOURCES->images.knob1_filmstrip));
	pitch_tune_slider->assignParameter(module->getParameterByName("Tune"));
	pitch_tune_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	pitch_tune_slider->ParameterInfo::InfoSender::name = "Tune";
	pitch_tune_slider->ParameterInfo::InfoSender::description = "Pitch offset in semitones. Modulate this for exponential FM. This will not stay in tune, but will sounds great for fx.";
	pitch_tune_slider->ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return String(pitch_tune_slider->getAssignedParameter()->getValue(), 3) + " st";
	};

	addAndMakeVisible(octave_slider = new ModulatableNumberDisplay());
	octave_slider->setMouseScaling(.002);
	octave_slider->assignParameter(module->getParameterByName("Octave"));
	octave_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	octave_slider->ParameterInfo::InfoSender::name = "Octave";
	octave_slider->ParameterInfo::InfoSender::description = "Pitch offset in octaves (quantized)";
	octave_slider->ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return valueToStringWithPlusMinusSign(octave_slider->getAssignedParameter()->getValue()) + " oct";
	};
	octave_slider->stringFunc = [this]()
	{
		return valueToStringWithPlusMinusSign(octave_slider->getAssignedParameter()->getValue());
	};

	addAndMakeVisible(octave_up = new ClickableArea());
	octave_up->setIncrementAmount(1);
	octave_up->assignParameter(module->getParameterByName("Octave"));
	octave_up->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	octave_up->ParameterInfo::InfoSender::sender = octave_slider;

	addAndMakeVisible(octave_dn = new ClickableArea());
	octave_dn->setIncrementAmount(-1);
	octave_dn->assignParameter(module->getParameterByName("Octave"));
	octave_dn->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	octave_dn->ParameterInfo::InfoSender::sender = octave_slider;

	addAndMakeVisible(pitch_harmonic_slider = new FilmStripKnob(RESOURCES->images.knob1_filmstrip));
	pitch_harmonic_slider->assignParameter(module->getParameterByName("Harmonic_Multiply"));
	pitch_harmonic_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	pitch_harmonic_slider->ParameterInfo::InfoSender::name = "Harmonic Multiply";
	pitch_harmonic_slider->ParameterInfo::InfoSender::description = "Multiplies frequency by number of harmonics. Modulate with LFO to create fun little melodies. Add feedback for even more fun.";
	pitch_harmonic_slider->ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return String(pitch_harmonic_slider->getAssignedParameter()->getValue(), 0);
	};

	addAndMakeVisible(pitch_harmOctGlide_slider = new FilmStripKnob(RESOURCES->images.knob1_filmstrip));
	pitch_harmOctGlide_slider->assignParameter(module->getParameterByName("HarmOct_Glide_Amount"));
	pitch_harmOctGlide_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	pitch_harmOctGlide_slider->ParameterInfo::InfoSender::name = "HarmOct Glide Amount";
	pitch_harmOctGlide_slider->ParameterInfo::InfoSender::description = "Linear speed of transition when changing octave or harmonic parameters.";
	pitch_harmOctGlide_slider->ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return elan::secondsToStringWithUnitTotal4(pitch_harmOctGlide_slider->getAssignedParameter()->getValue());
	};

	addAndMakeVisible(ampEnv_reset_dropdown = new MenuSM());
	ampEnv_reset_dropdown->assignParameter(module->getParameterByName("EnvResetMode"));
	ampEnv_reset_dropdown->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	ampEnv_reset_dropdown->ParameterInfo::InfoSender::name = "EnvResetMode";
	ampEnv_reset_dropdown->ParameterInfo::InfoSender::description = "Always: Reset on note on/off except for final off. / On-note: only for note on. / Legato: when not tied. / Never: Do not reset.";

	addAndMakeVisible(ampEnv_speed_slider = new FilmStripKnob(RESOURCES->images.knob1_filmstrip));
	ampEnv_speed_slider->assignParameter(module->getParameterByName("Speed"));
	ampEnv_speed_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	ampEnv_speed_slider->ParameterInfo::InfoSender::name = "Speed";
	ampEnv_speed_slider->ParameterInfo::InfoSender::description = "Higher values mean faster. Affects the timing of Attack, Decay, and Release stages.";;
	ampEnv_speed_slider->ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return String(ampEnv_speed_slider->getAssignedParameter()->getValue(), 3);
	};

	addAndMakeVisible(ampEnv_velScale_slider = new FilmStripKnob(RESOURCES->images.knob1_filmstrip));
	ampEnv_velScale_slider->assignParameter(module->getParameterByName("Vel_Scale"));
	ampEnv_velScale_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	ampEnv_velScale_slider->ParameterInfo::InfoSender::name = "Vel Scale";
	ampEnv_velScale_slider->ParameterInfo::InfoSender::description = "Sets how much influence midi velocity has on envelop. 0 means no infleunce.";
	ampEnv_velScale_slider->ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return String(ampEnv_velScale_slider->getAssignedParameter()->getValue(), 3);
	};

	addAndMakeVisible(ampEnv_attack_slider = new VerticalHandleSlider());
	ampEnv_attack_slider->assignParameter(module->getParameterByName("Attack"));
	ampEnv_attack_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	ampEnv_attack_slider->ParameterInfo::InfoSender::name = "Attack";
	ampEnv_attack_slider->ParameterInfo::InfoSender::description = "Attack time (not in seconds) of envelope before decay.";
	ampEnv_attack_slider->ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return String(ampEnv_attack_slider->getAssignedParameter()->getValue(), 3);
	};

	addAndMakeVisible(ampEnv_decay_slider = new VerticalHandleSlider());
	ampEnv_decay_slider->assignParameter(module->getParameterByName("Decay"));
	ampEnv_decay_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	ampEnv_decay_slider->ParameterInfo::InfoSender::name = "Decay";
	ampEnv_decay_slider->ParameterInfo::InfoSender::description = "Decay time (not in seconds) of envelope before sustaining.";
	ampEnv_decay_slider->ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return String(ampEnv_decay_slider->getAssignedParameter()->getValue(), 3);
	};

	addAndMakeVisible(ampEnv_sustain_slider = new VerticalHandleSlider());
	ampEnv_sustain_slider->assignParameter(module->getParameterByName("Sustain"));
	ampEnv_sustain_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	ampEnv_sustain_slider->ParameterInfo::InfoSender::name = "Sustain";
	ampEnv_sustain_slider->ParameterInfo::InfoSender::description = "Sustain level, or the level that the decay stage decays to, or in loop mode, the point of looping, amplitude-wise";
	ampEnv_sustain_slider->ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return String(ampEnv_sustain_slider->getAssignedParameter()->getValue(), 3);
	};

	addAndMakeVisible(ampEnv_release_slider = new VerticalHandleSlider());
	ampEnv_release_slider->assignParameter(module->getParameterByName("Release"));
	ampEnv_release_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	ampEnv_release_slider->ParameterInfo::InfoSender::name = "Release";
	ampEnv_release_slider->ParameterInfo::InfoSender::description = "Affects the shape of Attack, Decay, and Release envelope stages";
	ampEnv_release_slider->ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return String(ampEnv_release_slider->getAssignedParameter()->getValue(), 3);
	};

	addAndMakeVisible(ampEnv_lc_slider = new VerticalHandleSlider());
	ampEnv_lc_slider->assignParameter(module->getParameterByName("Linear_Time"));
	ampEnv_lc_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	ampEnv_lc_slider->ParameterInfo::InfoSender::name = "<- Linear / Curved ->";
	ampEnv_lc_slider->ParameterInfo::InfoSender::description = "Affects the shape of Attack, Decay, and Release envelope stages";
	ampEnv_lc_slider->ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return String(ampEnv_lc_slider->getAssignedParameter()->getValue(), 3);
	};

	addAndMakeVisible(ampEnv_attackCurve_dropdown = new MenuSM());
	ampEnv_attackCurve_dropdown->assignParameter(module->getParameterByName("Atk_Shape"));
	ampEnv_attackCurve_dropdown->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	ampEnv_attackCurve_dropdown->ParameterInfo::InfoSender::name = "Atk Shape";
	ampEnv_attackCurve_dropdown->ParameterInfo::InfoSender::description = "Choose shape of envelope stage. Generally LOG for attack. Try S-CURVE for blown-instrument-style envelope and to tame clicks.";

	addAndMakeVisible(ampEnv_decayCurve_dropdown = new MenuSM());
	ampEnv_decayCurve_dropdown->assignParameter(module->getParameterByName("Dec_Shape"));
	ampEnv_decayCurve_dropdown->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	ampEnv_decayCurve_dropdown->ParameterInfo::InfoSender::name = "Dec Shape";
	ampEnv_decayCurve_dropdown->ParameterInfo::InfoSender::description = "Choose shape of envelope stage. Generally EXP for attack. Try EXP2 for extreme pluck sound and vactrol emulation.";
	ampEnv_decayCurve_dropdown->ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return String(ampEnv_decayCurve_dropdown->getAssignedParameter()->getValue(), 0);
	};

	addAndMakeVisible(ampEnv_releaseCurve_dropdown = new MenuSM());
	ampEnv_releaseCurve_dropdown->assignParameter(module->getParameterByName("Rel_Shape"));
	ampEnv_releaseCurve_dropdown->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	ampEnv_releaseCurve_dropdown->ParameterInfo::InfoSender::name = "Rel Shape";
	ampEnv_releaseCurve_dropdown->ParameterInfo::InfoSender::description = "Choose shape of envelope stage. Generally EXP for release. Try S-CURVE for blown-instrument-style envelope and to tame clicks.";

	module->presetInformation.addListener(&presetInterface.presetPage);
	module->presetInformation.addListener(this);
	module->presetInformation.addListener(module);

	addAndMakeVisible(presetInterface);

	addChildComponent(errorWindow);
	errorWindow.setErrorWindowSize(216, 92);
	errorWindow.addButton("OK", []() { return; });
	errorWindow.addButton("CANCEL", []() { return; });
	errorWindow.addButton("YOU SUCK", []() { return; });
}

OMSEditor::~OMSEditor()
{
	module->removeChangeListener(this);
	module->presetInformation.removeListener(&presetInterface.presetPage);
	module->presetInformation.removeListener(this);
	module->presetInformation.removeListener(module);
	//parameterValueInput.removeListener(this);
}

void OMSEditor::rButtonClicked(jura::RButton* button)
{
	BasicEditorWithLicensing::rButtonClicked(button);
}

void OMSEditor::updateWidgetsAccordingToState()
{
	AudioModuleEditor::updateWidgetsAccordingToState();
}

jura::AudioModuleEditor * OMSModule::createEditor(int)
{
	auto ptr = new OMSEditor(this);

	ptr->setSize(886, 690);

	ptr->updateWidgetsAccordingToState();

	ptr->BasicEditorWithLicensing::keyValidator.setProductIndex(se::KeyGenerator::productIndices::RADARGENERATOR);
	ptr->BasicEditorWithLicensing::initializePlugIn();

	return ptr;
}

//https://regexr.com/41j7o

void OMSEditor::resized()
{
	ScopedLock scopedLock(*lock);

	BasicEditorWithLicensing::resized();

	presetInterface.setBounds(2, 0, 882, 362);


	// HELP
	ParameterDescriptionText.setBounds(ElanRect::pad({ 385, 306, 199, 51 }, 13, 7, 3, 3));
	ParameterValueText.setBounds(ElanRect::pad({ 300, 306, 86, 26 }, 3, 3, 7, 3));
	ParameterNameText.setBounds(ElanRect::pad({300, 331, 86, 26}, 3, 3, 7, 3));

	radarGenerator_editor->setBounds(174, 0, 415, 364);

	oscilloscopeEditor->setBounds(298, 0, 291, 210);
	oscilloscopeArea.setBounds(oscilloscopeEditor->getBounds());

	psedit->setBounds(298, 0, 291, 210);

	channelmode_dropdown->setBounds(9, 41, 70, 19);
	clipping_soft_button->setBounds(25, 105, 16, 18, MidCenter);
	clipping_hard_button->setBounds(64, 105, 16, 18, MidCenter);
	clipping_gain_slider->setBounds(45, 169, 32, 32, MidCenter);
	clipping_limit_slider->setBounds(45, 209, 30, 30, MidCenter);
	parameterSmoothing_slider->setBounds(45, 267, 30, 30, MidCenter);

	rotate_slider->setBounds(128, 31, 32, 32, MidCenter);
	dc_offset_slider->setBounds(110, 126, 32, 32, MidCenter);
	dc_X_slider->setBounds(148, 106, 30, 30, MidCenter);
	dc_Y_slider->setBounds(148, 146, 30, 30, MidCenter);
	chaos_source_dropdown->setBounds(94, 219, 70, 19);
	chaos_LP_slider->setBounds(111, 285, 30, 30, MidCenter);
	chaos_HP_slider->setBounds(111, 325, 30, 30, MidCenter);
	chaos_amount_slider->setBounds(149, 305, 32, 32, MidCenter);

	presetInterface.presetPage.load_preset_dropdown.setBounds(803, 2, 22, 22);
	presetInterface.presetPage.save_preset_dropdown.setBounds(830, 2, 22, 22);
	presetInterface.presetPage.manage_preset_dropdown.setBounds(857, 2, 22, 22);

	keytracking_button->setBounds(606, 55, 16, 18, MidCenter);
	changePitch_always_button->setBounds(770, 55, 16, 18, MidCenter);
	changePitch_legato_button->setBounds(831, 55, 16, 18, MidCenter);
	tempo_reset_button->setBounds(606, 113, 16, 18, MidCenter);
	tempo_freq_slider->setBounds(684, 103, 32, 32, MidCenter);
	tempo_tempo_slider->setBounds(734, 103, 32, 32, MidCenter);
	tempo_multiply_slider->setBounds(787, 103, 32, 32, MidCenter);
	masterRate_slider->setBounds(848, 103, 32, 32, MidCenter);
	glide_always_button->setBounds(606, 183, 16, 18, MidCenter);
	glide_legato_button->setBounds(606, 209, 16, 18, MidCenter);
	pitch_glide_slider->setBounds(684, 194, 32, 32, MidCenter);
	pitch_tune_slider->setBounds(623, 258, 32, 32, MidCenter);

	oversample_slider->setBounds(33, 306, 24, 34);
	oversample_up->setBounds(33, 306, 24, 6);
	oversample_dn->setBounds(33, 333, 24, 6);

	octave_slider->setBounds(672, 243, 24, 34);
	octave_up->setBounds(672, 243, 24, 6);
	octave_dn->setBounds(672, 270, 24, 6);

	pitch_harmonic_slider->setBounds(623, 318, 32, 32, MidCenter);
	pitch_harmOctGlide_slider->setBounds(684, 318, 32, 32, MidCenter);
	ampEnv_reset_dropdown->setBounds(722, 184, 48, 19);
	ampEnv_speed_slider->setBounds(802, 194, 32, 32, MidCenter);
	ampEnv_velScale_slider->setBounds(848, 194, 32, 32, MidCenter);

	ampEnv_attack_slider->setBounds(ampEnv_attack_slider->getBoundsForSliderArea(734, 241, 6, 63));
	ampEnv_decay_slider->setBounds(ampEnv_decay_slider->getBoundsForSliderArea(759, 241, 6, 63));
	ampEnv_sustain_slider->setBounds(ampEnv_sustain_slider->getBoundsForSliderArea(784, 241, 6, 63));
	ampEnv_release_slider->setBounds(ampEnv_release_slider->getBoundsForSliderArea(809, 241, 6, 63));
	ampEnv_lc_slider->setBounds(ampEnv_lc_slider->getBoundsForSliderArea(859, 241, 6, 63));

	ampEnv_attackCurve_dropdown->setBounds(722, 337, 48, 19);
	ampEnv_decayCurve_dropdown->setBounds(776, 337, 48, 19);
	ampEnv_releaseCurve_dropdown->setBounds(830, 337, 48, 19);

	{
		int x = 424;
		x += 126;
		x += 126;
		x += 126;
		fx_tab.setBounds(x, 367, 80, 24);
	}

	fx_page.setBounds(0, 392, 886, 238);
}

void JerobeamRadarEditor::resized()
{
	bypass_button->setBounds(109,19,12,12, MidCenter);
	reset_dropdown->setBounds(8, 56, 70, 19);
	gain_slider->setBounds(91, 56, 22, 22);
	phase_slider->setBounds(13, 111, 22, 22);
	subphase_slider->setBounds(52, 111, 22, 22);
	rotspeed_slider->setBounds(91, 111, 22, 22);
	cylinderFlat_slider->setBounds(12, 187, 65, 8);
	density_slider->setBounds(91, 179, 22, 22);
	sharp_slider->setBounds(13, 237, 22, 22);
	curve_slider->setBounds(52, 237, 22, 22);
	curveLeading_button->setBounds(91, 245, 12, 12, MidCenter);
	curveTrailing_button->setBounds(110, 245, 12, 12, MidCenter);
	shade_slider->setBounds(13, 291, 22, 22);
	direction_slider->setBounds(63, 303, 32, 32, MidCenter);
	lap_slider->setBounds(91, 291, 22, 22);
	bend_button->setBounds(52, 351, 12, 12, MidCenter);
	innerLenght_slider->setBounds(130, 251, 49, 8);
	innerZDepth_slider->setBounds(130, 281, 49, 8);
	outerLenght_slider->setBounds(193, 251, 49, 8);
	outerZDepth_slider->setBounds(193, 281, 49, 8);
	protrudeX_slider->setBounds(256, 251, 49, 8);
	protrudeY_slider->setBounds(256, 281, 49, 8);
	protrudeRatio_slider->setBounds(329, 259, 32, 32, MidCenter);
	ringcut_button->setBounds(364, 237, 12, 12, MidCenter);
	phaseInv_button->setBounds(364, 254, 12, 12, MidCenter);
	spiralReturn_button->setBounds(364, 275, 12, 12, MidCenter);
	tunnelInv_button->setBounds(364, 292, 12, 12, MidCenter);
}

void OscEditor::resized()
{
	color_title->setBounds(31, 10, 29, 8);
	//color_dropdown.setBounds(50, 25, 10, 9);
	color_field->setBounds(2, 25, 48, 9);
	oned_button->setBounds(85, 25, 10, 9);
	view_title->setBounds(109, 10, 24, 8);
	cycles_slider->setBounds(101, 23, 23, 13);
	z_slider->setBounds(150, 28, 18, 18);
	//z_title.setBounds(157, 10, 5, 8);
	brightness_slider->setBounds(176, 28, 18, 18);
	//brightness_title.setBounds(179, 8, 13, 13);
	afterglow_slider->setBounds(202, 28, 18, 18);
	//afterglow_title.setBounds(205, 8, 13, 13);
	dotlimit_slider->setBounds(228, 28, 18, 18);
	//dotlimit_title.setBounds(231, 8, 13, 13);
	oned_field->setBounds(74, 25, 7, 8);
	cycles_field->setBounds(125, 25, 10, 9);
	//bypass_title->setBounds(261, 12, 22, 5);
	//bypass_button->setBounds(267, 25, 10, 9);

	auto bounds = getLocalBounds();

	display.setBounds(getLocalBounds());
}
