#include "FlowerChildFilterEditor.h"

String valueToStringTotalUnit5(double v)
{
	if (v >= 0)
		return String(v, 5).paddedRight('0', 7).paddedRight('+', 8);
	else
		return String(v, 5).paddedRight('0', 8);
}

String timeToStringTotalUnit5(double v)
{
	if (v >= 1.0)
		return String(v, 5).paddedRight('0', 7).substring(0, 7) + " s";

	return String(v * 1000, 5).paddedRight('0', 7).substring(0, 7) +" ms";
}

FlowerChildFilterEditor::FlowerChildFilterEditor(FlowerChildFilterModule * module)
	: BasicEditor(module)
	, moduleToEdit(module)	
	, pluginFileManager(module, this, "Flower Child Filter", "FlowerChildFilter", "https://www.soundemote.com/fmd", "http://www.elanhickler.com/Soundemote/Soundemote_-_Flower_Child_Filter.zip")
	, presetBar(module, this, &presetDropdownSkin, stateWidgetSet)
{
	ScopedLock scopedLock(*lock);

	addAndMakeVisible(presetBar);

	addMouseListener(&presetBar.textEditor, true);

	presetBar.save_button.addMouseListener(this, false);

	presetBar.getPresetIconFunction = [&](const File& preset)
	{
		if (preset.getParentDirectory() == File(getFactoryPresetFolder()))
			return RESOURCES->vector.factoryicon.createCopyWithColor(Colours::red, { 176, 128, 191 });

		return RESOURCES->vector.usericon.createCopyWithColor(Colours::red, { 116, 156, 105 });
	};

	presetBar.presetWasChangedFunction = [&]()
	{
		setComponentVisibilityAndText();
	};

	presetBar.left_button.paintFunction = [this](Graphics& g)
	{
		auto& b = presetBar.left_button;
		auto area = presetBar.left_button.getLocalBounds().toFloat();

		if (b.isOver() && !b.isDown())
		{
			g.setColour(RESOURCES->colors.hoverFill);
			g.fillRect(area);
		}

		g.setColour(RESOURCES->colors.border);
		g.drawRect(area, scale);

		if (b.isDown())
			g.setColour(RESOURCES->colors.clicked);
		else if (b.isOver())
			g.setColour(RESOURCES->colors.hover);
		else
			g.setColour(RESOURCES->colors.normal);
		area.reduce(area.getWidth() * 0.35f, area.getWidth() * 0.35f);
		g.fillPath(ElanPath::triangle(area, Edge::Left));
	};

	presetBar.right_button.paintFunction = [this](Graphics& g)
	{
		auto& b = presetBar.right_button;
		auto area = b.getLocalBounds().toFloat();		

		if (b.isOver() && !b.isDown())
		{
			g.setColour(RESOURCES->colors.hoverFill);
			g.fillRect(area);
		}

		g.setColour(RESOURCES->colors.border);
		g.fillPath(ElanPath::threeSides(area, scale, Edge::Left));

		if (b.isDown())
			g.setColour(RESOURCES->colors.clicked);
		else if (b.isOver())
			g.setColour(RESOURCES->colors.hover);
		else
			g.setColour(RESOURCES->colors.normal);
		area.reduce(area.getWidth() * 0.35f, area.getWidth() * 0.35f);
		g.fillPath(ElanPath::triangle(area, Edge::Right));
	};

	presetBar.presetName_button.paintFunction = [this](Graphics& g)
	{
		auto& b = presetBar.presetName_button;
		auto area = b.getLocalBounds().toFloat();

		if (b.isOver() && !b.isDown())
		{
			g.setColour(RESOURCES->colors.hoverFill);
			g.fillRect(area);
		}

		g.setColour(RESOURCES->colors.border);
		g.drawRect(area, scale);

		Colour color;
		if (b.isDown())
			color = RESOURCES->colors.clicked;
		else if (b.isOver())
			color = { 255, 255, 255 };
		else
			color = RESOURCES->colors.font;
		g.setColour(color);
		g.setFont(RESOURCES->font.RobotoBold.withPointHeight(10 * scale));
		g.drawText(presetBar.getPresetNameText(), ElanRect::pad(area.toNearestInt(), int(10 * scale), 0, 0, (int)round(scale)), Justification::centredLeft);
	};

	presetBar.save_button.paintFunction = [this](Graphics& g)
	{
		auto& b = presetBar.save_button;
		auto area = b.getLocalBounds().toFloat();

		if (b.isOver() && !b.isDown())
		{
			g.setColour(RESOURCES->colors.hoverFill);
			g.fillRect(area);
		}

		g.setColour(RESOURCES->colors.border);
		g.drawRect(area, scale);

		Colour color;
		if (b.isDown())
			color = RESOURCES->colors.clicked;
		else if (b.isOver())
			color = RESOURCES->colors.hover;
		else
			color = RESOURCES->colors.normal;

		area.reduce(4.f * scale, 4.f * scale);
		RESOURCES->vector.saveicon.drawWithColor(g, area.toNearestInt().toFloat(), Colours::red, color);
	};

	addAndMakeVisible(burgerMenu_button);

	burgerMenu_button.paintFunction = [this](Graphics& g)
	{
		auto& b = burgerMenu_button;
		auto area = burgerMenu_button.getLocalBounds().toFloat();

		if (b.isOver() && !b.isDown())
		{
			g.setColour(RESOURCES->colors.hoverFill);
			g.fillRect(area);
		}

		g.setColour(RESOURCES->colors.border);
		g.fillPath(ElanPath::threeSides(area, 1 * scale, Edge::Left));

		Colour color;
		if (b.isDown())
			color = RESOURCES->colors.clicked;
		else if (b.isOver())
			color = RESOURCES->colors.hover;
		else
			color = RESOURCES->colors.normal;

		g.setColour(color);
		area.reduce(4.f * scale, 4.f * scale);
		g.fillPath(ElanPath::burgerMenu(area.toNearestInt().toFloat(), 2.5f * scale));
	};

	burgerMenu_button.clickFunction = [this]()
	{
		burgerMenu_button.resetState();
		PopupMenu menu;

		MenuHelper::addCallbackItem(menu, "Scale 50%", [this]()
			{
				setSize(int(width_x1_scale * 0.5f), int(width_x1_scale * 0.5f));
			});

		MenuHelper::addCallbackItem(menu, "Scale 75%", [this]()
			{
				setSize(int(width_x1_scale * 0.75f), int(width_x1_scale * 0.75f));
			});

		MenuHelper::addCallbackItem(menu, "Scale 100%", [this]()
			{
				setSize(width_x1_scale, height_x1_scale);
			});

		MenuHelper::addCallbackItem(menu, "Scale 150%", [this]()
			{
				setSize(int(width_x1_scale * 1.5f), int(height_x1_scale * 1.5f));
			});

		MenuHelper::addCallbackItem(menu, "Scale 200%", [this]()
			{
				setSize(width_x1_scale * 2, height_x1_scale * 2);
			});

		MenuHelper::addSeperator(menu);

		MenuHelper::addGreyedOutItem(menu, pluginFileManager.getBuildString() + "beta");

		MenuHelper::addCallbackItem(menu, "Re-download plugin (click here if you need update)", [this]()
			{
				URL(pluginFileManager.info.moduleDownloadLink).launchInDefaultBrowser();
			});

		MenuHelper::show(&menu, &burgerMenu_button);
	};

	addAndMakeVisible(oscilloscopeEditor = new OscilloscopeEditor(module->oscilloscopeModule));
	addChildEditor(oscilloscopeEditor);
	oscilloscopeEditor->changeListenerCallback(nullptr);
	oscilloscopeEditor->addMouseListener(this, true);
	addMouseListener(&oscilloscopeArea, false);
	oscilloscopeArea.setInterceptsMouseClicks(false, true);
	oscilloscopeEditor->setHideAllControls(true);

	addAndMakeVisible(algorithm_combobox);
	algorithm_combobox.setSkin(&algorithmDropdownSkin);
	algorithm_combobox.assignParameter(module->getParameterByName("Algorithm"));
	algorithm_combobox.ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	algorithm_combobox.ParameterInfo::InfoSender::name = "Algorithm";
	algorithm_combobox.ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		Component::SafePointer<Dropdown> ptr = { &algorithm_combobox };
		MessageManager::callAsync([=]()
			{
				if (ptr)
					setComponentVisibilityAndText();
			});
		
		return String(algorithm_combobox.getAssignedParameter()->getValue());
	};

	addAndMakeVisible(cutoff_knob);
	cutoff_knob.label = "CUTOFF";
	cutoff_knob.assignParameter(module->getParameterByName("Cutoff"));
	cutoff_knob.ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	cutoff_knob.ParameterInfo::InfoSender::name = "Cutoff";
	cutoff_knob.ParameterInfo::InfoSender::description = "Sets the cutoff frequency of the filter.";
	cutoff_knob.ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return valueToStringTotalUnit5(cutoff_knob.getAssignedParameter()->getValue());
	};

	addAndMakeVisible(resonance_knob);
	resonance_knob.label = "RESO";
	resonance_knob.assignParameter(module->getParameterByName("Resonance"));
	resonance_knob.ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	resonance_knob.ParameterInfo::InfoSender::name = "Resonance";
	resonance_knob.ParameterInfo::InfoSender::description = "Increases interal feedback. High settings result in self-oscillation. Low settings result in reduced high frequency.";
	resonance_knob.ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return valueToStringTotalUnit5(resonance_knob.getAssignedParameter()->getValue());
	};

	addAndMakeVisible(chaos_knob);
	chaos_knob.label = "CHAOS";
	chaos_knob.assignParameter(module->getParameterByName("Chaos"));
	chaos_knob.ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	chaos_knob.ParameterInfo::InfoSender::name = "Chaos";
	chaos_knob.ParameterInfo::InfoSender::description = "Adds internal instability to the filter circuit.";
	chaos_knob.ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return valueToStringTotalUnit5(chaos_knob.getAssignedParameter()->getValue());
	};

	addAndMakeVisible(chaos2_knob);
	chaos2_knob.label = "CHAOS";
	chaos2_knob.assignParameter(module->getParameterByName("Chaos2"));
	chaos2_knob.ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	chaos2_knob.ParameterInfo::InfoSender::name = "Chaos";
	chaos2_knob.ParameterInfo::InfoSender::description = "Adds internal instability to the filter circuit.";
	chaos2_knob.ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return valueToStringTotalUnit5(chaos2_knob.getAssignedParameter()->getValue());
	};

	addAndMakeVisible(input_knob);
	input_knob.label = "INPUT";
	input_knob.assignParameter(module->getParameterByName("Input"));
	input_knob.ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	input_knob.ParameterInfo::InfoSender::name = "Input";
	input_knob.ParameterInfo::InfoSender::description = "Boosting input signal will add overdrive. Rudcing will give extra headroom to the feedback / resonance for a cleaner sound.";
	input_knob.ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return valueToStringTotalUnit5(input_knob.getAssignedParameter()->getValue());
	};

	addAndMakeVisible(output_knob);
	output_knob.label = "OUTPUT";
	output_knob.assignParameter(module->getParameterByName("Output"));
	output_knob.ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	output_knob.ParameterInfo::InfoSender::name = "Output";
	output_knob.ParameterInfo::InfoSender::description = "Final output of the filter. Set to 0 and use output envelope follower to control volume solely by the input signal.";
	output_knob.ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return valueToStringTotalUnit5(output_knob.getAssignedParameter()->getValue());
	};

	addAndMakeVisible(drywet_knob);
	drywet_knob.label = "DRY/WET";
	drywet_knob.assignParameter(module->getParameterByName("DryWet"));
	drywet_knob.ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	drywet_knob.ParameterInfo::InfoSender::name = "Dry vs Wet";
	drywet_knob.ParameterInfo::InfoSender::description = "0 passes original signal through. 1 passes only the filtered signal. Note: This is not a true dry/wet. The output envelope still affects the dry signal.";
	drywet_knob.ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return valueToStringTotalUnit5(drywet_knob.getAssignedParameter()->getValue());
	};

	addAndMakeVisible(freqAttack_knob);
	freqAttack_knob.label = "ATTACK";
	freqAttack_knob.assignParameter(module->getParameterByName("CutoffEnvAttack"));
	freqAttack_knob.ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	freqAttack_knob.ParameterInfo::InfoSender::name = "Cutoff Env Attack";
	freqAttack_knob.ParameterInfo::InfoSender::description = "Rising speed of cutoff envelope follower.";
	freqAttack_knob.ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return timeToStringTotalUnit5(freqAttack_knob.getAssignedParameter()->getValue());
	};

	addAndMakeVisible(freqHold_knob);
	freqHold_knob.label = "HOLD";
	freqHold_knob.assignParameter(module->getParameterByName("CutoffEnvHold"));
	freqHold_knob.ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	freqHold_knob.ParameterInfo::InfoSender::name = "Cutoff Env Hold";
	freqHold_knob.ParameterInfo::InfoSender::description = "Lenght of time before the envelope follower begins to decay. Use this to prevent audio-rate modulation.";
	freqHold_knob.ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return timeToStringTotalUnit5(freqHold_knob.getAssignedParameter()->getValue());
	};

	addAndMakeVisible(freqDecay_knob);
	freqDecay_knob.assignParameter(module->getParameterByName("CutoffEnvDecay"));
	freqDecay_knob.ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	freqDecay_knob.ParameterInfo::InfoSender::name = "Cutoff Env Decay";
	freqDecay_knob.ParameterInfo::InfoSender::description = "Falling speed of cutoff envelope follower.";
	freqDecay_knob.ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return timeToStringTotalUnit5(freqDecay_knob.getAssignedParameter()->getValue());
	};

	addAndMakeVisible(freqSend_knob);
	freqSend_knob.assignParameter(module->getParameterByName("CutoffEnvSend"));
	freqSend_knob.ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	freqSend_knob.ParameterInfo::InfoSender::name = "Cutoff Env Send";
	freqSend_knob.ParameterInfo::InfoSender::description = "How much to affect cutoff with envelope follower. Negative values invert the modulation.";
	freqSend_knob.ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return valueToStringTotalUnit5(freqSend_knob.getAssignedParameter()->getValue());
	};

	addAndMakeVisible(ampAttack_knob);
	ampAttack_knob.label = "ATTACK";
	ampAttack_knob.assignParameter(module->getParameterByName("AmpEnvAttack"));
	ampAttack_knob.ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	ampAttack_knob.ParameterInfo::InfoSender::name = "Amp Env Attack";
	ampAttack_knob.ParameterInfo::InfoSender::description = "Rising speed of amplitude envelope follower.";
	ampAttack_knob.ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return timeToStringTotalUnit5(ampAttack_knob.getAssignedParameter()->getValue());
	};

	addAndMakeVisible(ampHold_knob);
	ampHold_knob.label = "HOLD";
	ampHold_knob.assignParameter(module->getParameterByName("AmpEnvHold"));
	ampHold_knob.ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	ampHold_knob.ParameterInfo::InfoSender::name = "Amp Env Hold";
	ampHold_knob.ParameterInfo::InfoSender::description = "Lenght of time before the envelope follower begins to decay. Use this to prevent audio-rate modulation.";
	ampHold_knob.ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return timeToStringTotalUnit5(ampHold_knob.getAssignedParameter()->getValue());
	};

	addAndMakeVisible(ampDecay_knob);
	ampDecay_knob.label = "DECAY";
	ampDecay_knob.assignParameter(module->getParameterByName("AmpEnvDecay"));
	ampDecay_knob.ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	ampDecay_knob.ParameterInfo::InfoSender::name = "Amp Env Decay";
	ampDecay_knob.ParameterInfo::InfoSender::description = "Falling speed of amplitude envelope follower.";
	ampDecay_knob.ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return timeToStringTotalUnit5(ampDecay_knob.getAssignedParameter()->getValue());
	};

	addAndMakeVisible(ampSend_knob);
	ampSend_knob.assignParameter(module->getParameterByName("AmpEnvSend"));
	ampSend_knob.ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	ampSend_knob.ParameterInfo::InfoSender::name = "Amp Env Send";
	ampSend_knob.ParameterInfo::InfoSender::description = "How much to affect amplitude with envelope follower. Negative values invert the modulation.";
	ampSend_knob.ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return valueToStringTotalUnit5(ampSend_knob.getAssignedParameter()->getValue());
	};

	addAndMakeVisible(adsrFreqAttack_knob);
	adsrFreqAttack_knob.label = "A";
	adsrFreqAttack_knob.assignParameter(module->getParameterByName("CutoffADSRAttack"));
	adsrFreqAttack_knob.ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	adsrFreqAttack_knob.ParameterInfo::InfoSender::name = "ADSR Cutoff Attack";
	adsrFreqAttack_knob.ParameterInfo::InfoSender::description = "Rising speed of cutoff ADSR.";
	adsrFreqAttack_knob.ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return timeToStringTotalUnit5(adsrFreqAttack_knob.getAssignedParameter()->getValue());
	};

	addAndMakeVisible(adsrFreqDecay_knob);
	adsrFreqDecay_knob.label = "D";
	adsrFreqDecay_knob.assignParameter(module->getParameterByName("CutoffADSRDecay"));
	adsrFreqDecay_knob.ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	adsrFreqDecay_knob.ParameterInfo::InfoSender::name = "ADSR Cutoff Decay";
	adsrFreqDecay_knob.ParameterInfo::InfoSender::description = "Falling speed of cutoff ADSR while midi note is held.";
	adsrFreqDecay_knob.ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return timeToStringTotalUnit5(adsrFreqDecay_knob.getAssignedParameter()->getValue());
	};

	addAndMakeVisible(adsrFreqSustain_knob);
	adsrFreqSustain_knob.label = "S";
	adsrFreqSustain_knob.assignParameter(module->getParameterByName("CutoffADSRSustain"));
	adsrFreqSustain_knob.ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	adsrFreqSustain_knob.ParameterInfo::InfoSender::name = "ADSR Cutoff Sustain";
	adsrFreqSustain_knob.ParameterInfo::InfoSender::description = "Sustain level of cutoff ADSR.";
	adsrFreqSustain_knob.ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return timeToStringTotalUnit5(adsrFreqSustain_knob.getAssignedParameter()->getValue());
	};

	addAndMakeVisible(adsrFreqRelease_knob);
	adsrFreqRelease_knob.label = "R";
	adsrFreqRelease_knob.assignParameter(module->getParameterByName("CutoffADSRRelease"));
	adsrFreqRelease_knob.ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	adsrFreqRelease_knob.ParameterInfo::InfoSender::name = "ADSR Cutoff Release";
	adsrFreqRelease_knob.ParameterInfo::InfoSender::description = "Falling speed of cutoff ADSR while midi note is not held.";
	adsrFreqRelease_knob.ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return timeToStringTotalUnit5(adsrFreqRelease_knob.getAssignedParameter()->getValue());
	};

	addAndMakeVisible(adsrFreqAttackCurve_knob);
	adsrFreqAttackCurve_knob.assignParameter(module->getParameterByName("CutoffADSRAttackCurve"));
	adsrFreqAttackCurve_knob.ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	adsrFreqAttackCurve_knob.ParameterInfo::InfoSender::name = "ADSR Cutoff Attack Curve";
	adsrFreqAttackCurve_knob.ParameterInfo::InfoSender::description = "Rising curve of cutoff ADSR, exp < 0, lin = 0, log > 0.";
	adsrFreqAttackCurve_knob.ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return valueToStringTotalUnit5(adsrFreqAttackCurve_knob.getAssignedParameter()->getValue());
	};

	addAndMakeVisible(adsrFreqDecayCurve_knob);
	adsrFreqDecayCurve_knob.assignParameter(module->getParameterByName("CutoffADSRDecayCurve"));
	adsrFreqDecayCurve_knob.ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	adsrFreqDecayCurve_knob.ParameterInfo::InfoSender::name = "ADSR Cutoff Decay Curve";
	adsrFreqDecayCurve_knob.ParameterInfo::InfoSender::description = "Falling curve of cutoff ADSR, log < 0, lin = 0, exp > 0.";
	adsrFreqDecayCurve_knob.ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return valueToStringTotalUnit5(adsrFreqDecayCurve_knob.getAssignedParameter()->getValue());
	};

	addAndMakeVisible(adsrFreqReleaseCurve_knob);
	adsrFreqReleaseCurve_knob.assignParameter(module->getParameterByName("CutoffADSRReleaseCurve"));
	adsrFreqReleaseCurve_knob.ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	adsrFreqReleaseCurve_knob.ParameterInfo::InfoSender::name = "ADSR Cutoff Release Curve";
	adsrFreqReleaseCurve_knob.ParameterInfo::InfoSender::description = "Release curve of cutoff ADSR, log < 0, lin = 0, exp > 0.";
	adsrFreqReleaseCurve_knob.ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return valueToStringTotalUnit5(adsrFreqReleaseCurve_knob.getAssignedParameter()->getValue());
	};

	addAndMakeVisible(adsrAmpAttack_knob);
	adsrAmpAttack_knob.label = "A";
	adsrAmpAttack_knob.assignParameter(module->getParameterByName("AmpADSRAttack"));
	adsrAmpAttack_knob.ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	adsrAmpAttack_knob.ParameterInfo::InfoSender::name = "ADSR Amp Attack";
	adsrAmpAttack_knob.ParameterInfo::InfoSender::description = "Rising speed of amp ADSR.";
	adsrAmpAttack_knob.ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return timeToStringTotalUnit5(adsrAmpAttack_knob.getAssignedParameter()->getValue());
	};

	addAndMakeVisible(adsrAmpDecay_knob);
	adsrAmpDecay_knob.label = "D";
	adsrAmpDecay_knob.assignParameter(module->getParameterByName("AmpADSRDecay"));
	adsrAmpDecay_knob.ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	adsrAmpDecay_knob.ParameterInfo::InfoSender::name = "ADSR Amp Decay";
	adsrAmpDecay_knob.ParameterInfo::InfoSender::description = "Falling speed of amp ADSR while midi note is held.";
	adsrAmpDecay_knob.ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return timeToStringTotalUnit5(adsrAmpDecay_knob.getAssignedParameter()->getValue());
	};

	addAndMakeVisible(adsrAmpSustain_knob);
	adsrAmpSustain_knob.label = "S";
	adsrAmpSustain_knob.assignParameter(module->getParameterByName("AmpADSRSustain"));
	adsrAmpSustain_knob.ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	adsrAmpSustain_knob.ParameterInfo::InfoSender::name = "ADSR Amp Sustain";
	adsrAmpSustain_knob.ParameterInfo::InfoSender::description = "Sustain level of amp ADSR.";
	adsrAmpSustain_knob.ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return timeToStringTotalUnit5(adsrAmpSustain_knob.getAssignedParameter()->getValue());
	};

	addAndMakeVisible(adsrAmpRelease_knob);
	adsrAmpRelease_knob.label = "R";
	adsrAmpRelease_knob.assignParameter(module->getParameterByName("AmpADSRRelease"));
	adsrAmpRelease_knob.ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	adsrAmpRelease_knob.ParameterInfo::InfoSender::name = "ADSR Amp Release";
	adsrAmpRelease_knob.ParameterInfo::InfoSender::description = "Falling speed of amp ADSR while midi note is not held.";
	adsrAmpRelease_knob.ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return timeToStringTotalUnit5(adsrAmpRelease_knob.getAssignedParameter()->getValue());
	};

	addAndMakeVisible(adsrAmpAttackCurve_knob);
	adsrAmpAttackCurve_knob.assignParameter(module->getParameterByName("AmpADSRAttackCurve"));
	adsrAmpAttackCurve_knob.ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	adsrAmpAttackCurve_knob.ParameterInfo::InfoSender::name = "ADSR Amp Attack Curve";
	adsrAmpAttackCurve_knob.ParameterInfo::InfoSender::description = "Rising curve of amp ADSR, exp < 0, lin = 0, log > 0.";
	adsrAmpAttackCurve_knob.ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return valueToStringTotalUnit5(adsrAmpAttackCurve_knob.getAssignedParameter()->getValue());
	};

	addAndMakeVisible(adsrAmpDecayCurve_knob);
	adsrAmpDecayCurve_knob.assignParameter(module->getParameterByName("AmpADSRDecayCurve"));
	adsrAmpDecayCurve_knob.ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	adsrAmpDecayCurve_knob.ParameterInfo::InfoSender::name = "ADSR Amp Decay Curve";
	adsrAmpDecayCurve_knob.ParameterInfo::InfoSender::description = "Falling curve of amp ADSR, log < 0, lin = 0, exp > 0.";
	adsrAmpDecayCurve_knob.ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return valueToStringTotalUnit5(adsrAmpDecayCurve_knob.getAssignedParameter()->getValue());
	};

	addAndMakeVisible(adsrAmpReleaseCurve_knob);
	adsrAmpReleaseCurve_knob.assignParameter(module->getParameterByName("AmpADSRReleaseCurve"));
	adsrAmpReleaseCurve_knob.ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	adsrAmpReleaseCurve_knob.ParameterInfo::InfoSender::name = "ADSR Amp Release Curve";
	adsrAmpReleaseCurve_knob.ParameterInfo::InfoSender::description = "Release curve of amp ADSR, log < 0, lin = 0, exp > 0.";
	adsrAmpReleaseCurve_knob.ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return valueToStringTotalUnit5(adsrAmpReleaseCurve_knob.getAssignedParameter()->getValue());
	};

	addAndMakeVisible(oversampling_knob);
	oversampling_knob.label = "OVERSAMPLING";
	oversampling_knob.assignParameter(module->getParameterByName("Oversampling"));
	oversampling_knob.ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	oversampling_knob.ParameterInfo::InfoSender::name = "Oversampling";
	oversampling_knob.ParameterInfo::InfoSender::description = "Increases CPU usage for better production of high frequencies.";
	oversampling_knob.ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return elan::StringFunc0WithCapitolX(oversampling_knob.getAssignedParameter()->getValue());
	};

	addAndMakeVisible(smoothing_knob);
	smoothing_knob.label = "SMOOTHING";
	smoothing_knob.assignParameter(module->getParameterByName("ParamSmoothing"));
	smoothing_knob.ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	smoothing_knob.ParameterInfo::InfoSender::name = "Parameter Smoothing";
	smoothing_knob.ParameterInfo::InfoSender::description = "Controls how fast parameters respond to mouse movement and host automation. Smoothing time also affects preset changing speed (preset-morphing).";
	smoothing_knob.ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return timeToStringTotalUnit5(smoothing_knob.getAssignedParameter()->getValue());
	};

	addAndMakeVisible(monoStereo_button);
	monoStereo_button.assignParameter(module->getParameterByName("ChannelMode"));
	monoStereo_button.ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	monoStereo_button.ParameterInfo::InfoSender::name = "Channel Mode";
	monoStereo_button.ParameterInfo::InfoSender::description = "Sets plugin to operate in stereo or mono to save CPU.";
	monoStereo_button.ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return monoStereo_button.getAssignedParameter()->getValue() > 0.5 ? "STEREO" : "MONO";
	};

	addAndMakeVisible(freqEnvMode_button);
	freqEnvMode_button.assignParameter(module->getParameterByName("CutoffEnvMode"));
	freqEnvMode_button.ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	freqEnvMode_button.ParameterInfo::InfoSender::name = "Cutoff Env Mode";
	freqEnvMode_button.ParameterInfo::InfoSender::description = "Sets the frequency envelope mode to classic ADSR triggered via midi or envelope follower triggered via audio input.";
	freqEnvMode_button.ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return freqEnvMode_button.getAssignedParameter()->getValue() > 0.5 ? "ADSR" : "FOLLOWER";
	};
	freqEnvMode_button.addRButtonListener(this);

	addAndMakeVisible(ampEnvMode_button);
	ampEnvMode_button.assignParameter(module->getParameterByName("AmpEnvMode"));
	ampEnvMode_button.ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	ampEnvMode_button.ParameterInfo::InfoSender::name = "Amp Env Mode";
	ampEnvMode_button.ParameterInfo::InfoSender::description = "Sets the amplitude envelope mode to classic ADSR triggered via midi or envelope follower triggered via audio input.";
	ampEnvMode_button.ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return ampEnvMode_button.getAssignedParameter()->getValue() > 0.5 ? "ADSR" : "FOLLOWER";
	};
	ampEnvMode_button.addRButtonListener(this);

	addAndMakeVisible(freqAdsrResetModeAlways_button);
	freqAdsrResetModeAlways_button.assignParameter(module->getParameterByName("CutoffAdsrTrigMode"));
	freqAdsrResetModeAlways_button.ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	freqAdsrResetModeAlways_button.ParameterInfo::InfoSender::name = "Cutoff ADSR Reset Mode";
	freqAdsrResetModeAlways_button.ParameterInfo::InfoSender::description = "Sets frequency ADSR retrigger mode either for legato playing or retrigger on every note.";
	freqAdsrResetModeAlways_button.ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return freqAdsrResetModeAlways_button.getAssignedParameter()->getValue() > 0.5 ? "ALWAYS" : "LEGATO";
	};
	freqAdsrResetModeAlways_button.setFrameForOn(0);
	freqAdsrResetModeAlways_button.setFrameForOff(1);

	addAndMakeVisible(freqAdsrResetModeLegato_button);
	freqAdsrResetModeLegato_button.assignParameter(module->getParameterByName("CutoffAdsrTrigMode"));
	freqAdsrResetModeLegato_button.ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	freqAdsrResetModeLegato_button.ParameterInfo::InfoSender::name = "Cutoff ADSR Reset Mode";
	freqAdsrResetModeLegato_button.ParameterInfo::InfoSender::description = "Sets frequency ADSR retrigger mode either for legato playing or retrigger on every note.";
	freqAdsrResetModeLegato_button.ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return freqAdsrResetModeLegato_button.getAssignedParameter()->getValue() > 0.5 ? "ALWAYS" : "LEGATO";
	};
	freqAdsrResetModeLegato_button.setStateForOn(0);
	freqAdsrResetModeLegato_button.setFrameForOn(0);
	freqAdsrResetModeLegato_button.setFrameForOff(1);

	addAndMakeVisible(ampAdsrResetModeAlways_button);
	ampAdsrResetModeAlways_button.assignParameter(module->getParameterByName("AmpAdsrTrigMode"));
	ampAdsrResetModeAlways_button.ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	ampAdsrResetModeAlways_button.ParameterInfo::InfoSender::name = "Amp ADSR Reset Mode";
	ampAdsrResetModeAlways_button.ParameterInfo::InfoSender::description = "Sets amplitude ADSR retrigger mode either for legato playing or retrigger on every note.";
	ampAdsrResetModeAlways_button.ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return ampAdsrResetModeAlways_button.getAssignedParameter()->getValue() > 0.5 ? "ALWAYS" : "LEGATO";
	};
	ampAdsrResetModeAlways_button.setFrameForOn(0);
	ampAdsrResetModeAlways_button.setFrameForOff(1);

	addAndMakeVisible(ampAdsrResetModeLegato_button);
	ampAdsrResetModeLegato_button.assignParameter(module->getParameterByName("AmpAdsrTrigMode"));
	ampAdsrResetModeLegato_button.ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	ampAdsrResetModeLegato_button.ParameterInfo::InfoSender::name = "Amp ADSR Reset Mode";
	ampAdsrResetModeLegato_button.ParameterInfo::InfoSender::description = "Sets amplitude ADSR retrigger mode either for legato playing or retrigger on every note.";
	ampAdsrResetModeLegato_button.ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return ampAdsrResetModeLegato_button.getAssignedParameter()->getValue() > 0.5 ? "ALWAYS" : "LEGATO";
	};
	ampAdsrResetModeLegato_button.setStateForOn(0);
	ampAdsrResetModeLegato_button.setFrameForOn(0);
	ampAdsrResetModeLegato_button.setFrameForOff(1);

	parameterInfo.addListener(this);

	parameterValueInput.setTextEditorBoundsFunction = [this]()
	{
		valueEntry.setBoundsToComponent(parameterValueInput.state.component);
	};

	addAndMakeVisible(parameterName_text);
	parameterName_text.setFontJustification(juce::Justification::centred);
	parameterName_text.setText(cutoff_knob.ParameterInfo::InfoSender::name);
	parameterName_text.setDoDrawFittedText(true);
	parameterName_text.setFontColor({ 155, 152, 121 });

	addAndMakeVisible(parameterValue_text);
	parameterValue_text.setFontJustification(juce::Justification::centred);
	parameterValue_text.setText(cutoff_knob.ParameterInfo::InfoSender::valueStringFunction());
	parameterValue_text.setDoDrawFittedText(true);
	parameterValue_text.setFontColor({ 207, 203, 161 });

	addAndMakeVisible(parameterHelpName_text);
	parameterHelpName_text.setFontJustification(juce::Justification::centredLeft);
	parameterHelpName_text.setText("Oh no!");
	parameterHelpName_text.setDoDrawFittedText(true);
	parameterHelpName_text.setFontColor({ 255, 255, 255 });

	addAndMakeVisible(parameterHelpDescription_text);
	parameterHelpDescription_text.setText("I'm still stuck in cyberspace. My name is <errortype>, please send help!");
	parameterHelpDescription_text.setDrawMultilineText(true);
	parameterHelpDescription_text.setFontColor({ 204, 204, 204 });

	addAndMakeVisible(donate_button);
	donate_button.ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	donate_button.ParameterInfo::InfoSender::name = "Donate";
	donate_button.ParameterInfo::InfoSender::description = "Are you getting use out of Flower Child Filter? Show your support for Soundemote Labs, donate today! Every bit helps.";
	donate_button.ParameterInfo::InfoSender::valueStringFunction = [this]()
	{
		return "Thank you!";
	};

	setComponentVisibilityAndText();
}

void FlowerChildFilterEditor::rButtonClicked(RButton* button)
{
	AudioModuleEditor::rButtonClicked(button);

	repaint();

	setComponentVisibilityAndText();
}

void FlowerChildFilterEditor::updateWidgetsAccordingToState()
{
	AudioModuleEditor::updateWidgetsAccordingToState();
}

AudioModuleEditor * FlowerChildFilterModule::createEditor(int type)
{
	auto ptr = new FlowerChildFilterEditor(this);

	ptr->setSize(ptr->width_x1_scale, ptr->height_x1_scale);

	return ptr;
}

void FlowerChildFilterEditor::resized()
{
	ScopedLock scopedLock(*lock);

	int w = clip(getWidth(), int(width_x1_scale * 0.5f), int(width_x1_scale * 2.f));
	int h = int(w * width_height_ratio);
	setScale(w / (float)width_x1_scale);

	Component* parent = getParentComponent();
	if (dynamic_cast<AudioPluginEditor*>(getParentComponent()))
		parent->setSize(w, h);

	moduleToEdit->oscilloscopeModule->buffer.setBrightness(scale);

	presetBar.setFloatBounds(Rectangle<float>(scale*2, scale*2, float(getWidth()) - presetBarHeight * scale - 4.f*scale, presetBarHeight * scale), scale);

	setOffset({ 0, presetBar.getBottom() + int(scale) });

	burgerMenu_button.setBounds(ElanRect::place({ presetBar.getHeight(), presetBar.getHeight() }, presetBar.getBounds(), MidLeft, MidRight));	

	algorithmDropdownSkin.setSize(presetBar.presetName_button.getWidth(), presetBar.presetName_button.getHeight());
	algorithm_combobox.setFloatBounds(11.f, 7.f, 209.f, 23.f);
	algorithmDropdownSkin.setScale(scale);
	algorithmDropdownSkin.setSize(algorithm_combobox.getWidth(), algorithm_combobox.getHeight());

	parameterValueInput.update();
	parameterName_text.setFont(RESOURCES->font.TekoMedium.withPointHeight(14.f * scale));
	parameterName_text.setBounds(ElanRect::place({ int(73 * scale), int(14 * scale) }, getLocalBounds().withY(int(offset.y + 52.f * scale)), TopCenter, TopCenter));

	Font font = RESOURCES->font.ShareTechMonoRegular.withPointHeight(16.f * scale);
	font.setExtraKerningFactor(-0.1f);
	parameterValue_text.setFont(font);
	parameterValue_text.setBounds(ElanRect::place({ int(104 * scale), int(16 * scale) }, getLocalBounds().withY(int(offset.y + 67 * scale)), TopCenter, TopCenter));

	HELPStringTextArea = { int(431.f * scale), int(282.f * scale), int(40.f * scale), int(17.6f * scale) };
	HELPStringTextArea.translate(0, offset.y);
	parameterHelpName_text.setFont(RESOURCES->font.TekoMedium.withPointHeight(17.6f * scale));
	parameterHelpName_text.setBounds(int(470.f * scale), HELPStringTextArea.getY(), int(133.f * scale), HELPStringTextArea.getHeight());

	parameterHelpDescription_text.setFont(RESOURCES->font.RobotoRegular.withPointHeight(10.f * scale));
	parameterHelpDescription_text.setBounds(juce::Rectangle<int>{ int(414.f * scale), int(297.f * scale), int(210.f * scale), int(52.f * scale) }.translated(0, offset.y));

	cutoff_knob.setFloatBounds(108, 115, 70);
	resonance_knob.setFloatBounds(188.f, 186.f);
	chaos_knob.setFloatBounds(188.f, 75.f);
	chaos2_knob.setFloatBounds(188.f, 75.f);

	input_knob.setFloatBounds(412.f, 75.f);
	output_knob.setFloatBounds(412.f, 186.f);
	drywet_knob.setFloatBounds(477.f,  130.f);

	freqHold_knob.setFloatBounds(26, 137, 25, 5);
	freqAttack_knob.setFloatBounds(63, 116, 25, 5);
	freqDecay_knob.setFloatBounds(63, 159, 25, 5);

	adsrFreqAttack_knob.setFloatBounds(63, 116, 25, 5);
	adsrFreqDecay_knob.setFloatBounds(63, 159, 25, 5);
	adsrFreqSustain_knob.setFloatBounds(100, 79, 25, 5);
	adsrFreqRelease_knob.setFloatBounds(100, 196, 25, 5);

	adsrFreqAttackCurve_knob.setFloatBounds(32, 123, 19, 3.8f);
	adsrFreqDecayCurve_knob.setFloatBounds(32, 158, 19, 3.8f);
	adsrFreqReleaseCurve_knob.setFloatBounds(76, 223, 19, 3.8f);

	freqSend_knob.setFloatBounds(133, 226, 20, 20, scale, offset);

	ampHold_knob.setFloatBounds(551, 116, 25, 5);
	ampAttack_knob.setFloatBounds(588, 137, 25, 5);
	ampDecay_knob.setFloatBounds(551, 159, 25, 5);

	adsrAmpAttack_knob.setFloatBounds(551, 116, 25, 5);
	adsrAmpDecay_knob.setFloatBounds(551, 159, 25, 5);
	adsrAmpSustain_knob.setFloatBounds(514, 79, 25, 5);
	adsrAmpRelease_knob.setFloatBounds(514,196, 25, 5);

	adsrAmpAttackCurve_knob.setFloatBounds(588, 123, 19, 3.8f);
	adsrAmpDecayCurve_knob.setFloatBounds(588, 158, 19, 3.8f);
	adsrAmpReleaseCurve_knob.setFloatBounds(544, 223, 19, 3.8f);

	ampSend_knob.setFloatBounds(488, 226, 20, 20, scale, offset);

	smoothing_knob.setFloatBounds(260.f, 241.f, 34.f, 6.8f);
	oversampling_knob.setFloatBounds(347.f, 241.f, 34.f, 6.8f);

	monoStereo_button.setFloatBounds(307, 241, 26.f, 33.f);

	freqEnvMode_button.setFloatBounds(5.f, 42.f, 60.f, 23.f, scale, offset);
	ampEnvMode_button.setFloatBounds(575.f, 42.f, 60.f, 23.f, scale, offset);

	freqAdsrResetModeAlways_button.setFloatBounds(2, 77, 64.4f, 22.f, scale, offset);
	freqAdsrResetModeLegato_button.setFloatBounds(2, 201, 64.4f, 22.f, scale, offset);
	ampAdsrResetModeAlways_button.setFloatBounds(575.f, 77.f, 63.f, 21.f, scale, offset);
	ampAdsrResetModeLegato_button.setFloatBounds(575.f, 202.f, 63.f, 21.f, scale, offset);

	donate_button.setFloatBounds(92.5f, 317.f);

	oscilloscopeEditor->setWidgetsToHideForMouseover(false);
	oscilloscopeEditor->setBounds(juce::Rectangle<int>{ int(238.f * scale), int(89.f * scale), int(164.f * scale), int(122.f * scale) }.translated(0, offset.y));
	oscilloscopeArea.setBounds(oscilloscopeEditor->getBounds());
}
