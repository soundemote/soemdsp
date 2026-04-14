#pragma once

#include "Resources.h"
#include "Graphics.h"
#include "PresetInterface.h"
#include "Widgetsets.h"
#include "ErrorWindow.h"

using jura::RButton;
using jura::RRadioButton;
using jura::RRadioButtonGroup;
using jura::RTextField;

//https://regexr.com/41j7l

class OscEditor : public AudioModuleEditor
{
public:
  
  OscEditor(OscilloscopeModule* module_, ParameterInfo& parameterInfo_);

	ParameterInfo& parameterInfo;
	OscilloscopeModule* module;
	ScopeDisplay display;
  
  ~OscEditor() = default;

	TempBox* bypass_title;
	TempBox* bypass_button;
	//OneStateImagePainter color_dropdown{ RESOURCES->images.dropdown_color };
	TempBox* oned_button;
	FilmStripKnob* cycles_slider;
	FilmStripKnob* z_slider;
	FilmStripKnob* brightness_slider;
	FilmStripKnob* afterglow_slider;
	FilmStripKnob* dotlimit_slider;
	TempBox* color_field;
	TempBox* oned_field;
	TempBox* cycles_field;
	TempBox* color_title;
	TempBox* view_title;
	//OneStateImagePainter z_title{ RESOURCES->images.icon_zdepth };
	//OneStateImagePainter brightness_title{ RESOURCES->images.icon_brightnes };
	//OneStateImagePainter afterglow_title{ RESOURCES->images.icon_overglow };
	//OneStateImagePainter dotlimit_title{ RESOURCES->images.dropdown_color };

protected:
	SharedResourcePointer<Resources> RESOURCES;

  void resized() override;  
	void paint(Graphics&) override {}
	void paintOverChildren(Graphics&) override {}
};

class JerobeamRadarEditor : public AudioModuleEditor
{
public:

	JerobeamRadarEditor(JerobeamRadarModule* module_, OMSModule* omsModule_, ParameterInfo& parameterInfo_);
	~JerobeamRadarEditor() = default;

	ParameterInfo& parameterInfo;
	JerobeamRadarModule* module;
	OMSModule* omsModule;

	TwoStateImageButton* bypass_button;
	MenuLG* reset_dropdown;
	FilmStripKnob* gain_slider;
	FilmStripKnob* phase_slider;
	FilmStripKnob* subphase_slider;
	FilmStripKnob* rotspeed_slider;
	BoxSlider* cylinderFlat_slider;
	FilmStripKnob* density_slider;
	FilmStripKnob* sharp_slider;
	FilmStripKnob* curve_slider;
	TwoStateImageButton* curveLeading_button;
	TwoStateImageButton* curveTrailing_button;
	FilmStripKnob* shade_slider;
	FilmStripKnob* direction_slider;
	FilmStripKnob* lap_slider;
	TwoStateImageButton* bend_button;
	BoxSlider* innerLenght_slider;
	BoxSlider* innerZDepth_slider;
	BoxSlider* outerLenght_slider;
	BoxSlider* outerZDepth_slider;
	BoxSlider* protrudeX_slider;
	BoxSlider* protrudeY_slider;
	FilmStripKnob* protrudeRatio_slider;
	TwoStateImageButton* ringcut_button;
	TwoStateImageButton* phaseInv_button;
	TwoStateImageButton* spiralReturn_button;
	TwoStateImageButton* tunnelInv_button;

protected:
	SharedResourcePointer<Resources> RESOURCES;
	void resized() override;
	void paint(Graphics&) override {}
	void paintOverChildren(Graphics&) override {}
};

class FXPage : public Component
{
public:
	FXPage(OMSModule* module, ParameterInfo& parameterInfo, CriticalSection* lock)
	{
		/* EQUALIZER */
		//addAndMakeVisible(eq_range_dropdown = new MenuSM);

		addAndMakeVisible(equalizerEditor = new EqualizerModuleEditor(lock, module->equalizerModule));
		equalizerEditor->setPresetSectionPosition(AudioModuleEditor::positions::INVISIBLE);

		addAndMakeVisible(eq_bypass_button = new TwoStateImageButton(RESOURCES->images.led_button_small));
		eq_bypass_button->assignParameter(module->equalizerModule->getParameterByName("Bypass"));
		eq_bypass_button->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
		eq_bypass_button->ParameterInfo::InfoSender::name = "Bypass";
		eq_bypass_button->ParameterInfo::InfoSender::description = "Bypass equalizer.";
		eq_bypass_button->ParameterInfo::InfoSender::valueStringFunction = [this]()
		{
			return eq_bypass_button->getAssignedParameter()->getValue() > 0 ? "Bypassed" : "Active";
		};

		addAndMakeVisible(eq_gain_slider = new VerticalHandleSlider);
		eq_gain_slider->assignParameter(module->equalizerModule->getParameterByName("GlobalGain"));
		eq_gain_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
		eq_gain_slider->ParameterInfo::InfoSender::name = "Gain";
		eq_gain_slider->ParameterInfo::InfoSender::description = "Overall gain for equalizer";
		eq_gain_slider->ParameterInfo::InfoSender::valueStringFunction = [this]()
		{
			return elan::StringFunc4(eq_bypass_button->getAssignedParameter()->getValue());
		};

	/*
		addAndMakeVisible(eq_widgetset = new TempBox);		
		addAndMakeVisible(eq_band_field = new TempBox);
		addAndMakeVisible(eq_band_mode_dropdown = new MenuLG);
		addAndMakeVisible(eq_band_time_slider = new FilmStripKnob(RESOURCES->images.knob3_filmstrip));
		addAndMakeVisible(eq_band_gain_slider = new FilmStripKnob(RESOURCES->images.knob1_filmstrip));
		addAndMakeVisible(eq_band_level_slider = new FilmStripKnob(RESOURCES->images.knob3_filmstrip));*/

		/* FILTER */
		addAndMakeVisible(filter_bypass_button = new TwoStateImageButton(RESOURCES->images.led_button_small));
		filter_bypass_button->assignParameter(module->omsCore->bandpassModule->getParameterByName("Bypass"));
		filter_bypass_button->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
		filter_bypass_button->ParameterInfo::InfoSender::name = "Bypass";
		filter_bypass_button->ParameterInfo::InfoSender::description = "Bypass both highpass and lowpass filters.";
		filter_bypass_button->ParameterInfo::InfoSender::valueStringFunction = [this]()
		{
			return filter_bypass_button->getAssignedParameter()->getValue() > 0 ? "Bypassed" : "Active";
		};

		addAndMakeVisible(filter_lp_slider = new FilmStripKnob(RESOURCES->images.knob3_filmstrip));
		filter_lp_slider->assignParameter(module->omsCore->bandpassModule->getParameterByName("Lowpass"));
		filter_lp_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
		filter_lp_slider->ParameterInfo::InfoSender::name = "Lowpass";
		filter_lp_slider->ParameterInfo::InfoSender::description = "Sets frequency of lowpass filter.";
		filter_lp_slider->ParameterInfo::InfoSender::valueStringFunction = [this]()
		{
			return hertzToStringWithUnitTotal4(filter_lp_slider->getAssignedParameter()->getValue());
		};

		addAndMakeVisible(filter_hp_slider = new FilmStripKnob(RESOURCES->images.knob3_filmstrip));
		filter_hp_slider->assignParameter(module->omsCore->bandpassModule->getParameterByName("Highpass"));
		filter_hp_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
		filter_hp_slider->ParameterInfo::InfoSender::name = "Highpass";
		filter_hp_slider->ParameterInfo::InfoSender::description = "Sets frequency of highpass filter.";
		filter_hp_slider->ParameterInfo::InfoSender::valueStringFunction = [this]()
		{
			return hertzToStringWithUnitTotal4(filter_hp_slider->getAssignedParameter()->getValue());
		};

		/* DELAY */
		addAndMakeVisible(delay_bypass_button = new TwoStateImageButton(RESOURCES->images.led_button_small));
		delay_bypass_button->assignParameter(module->delayModule->getParameterByName("Bypass"));
		delay_bypass_button->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
		delay_bypass_button->ParameterInfo::InfoSender::name = "Bypass";
		delay_bypass_button->ParameterInfo::InfoSender::description = "Bypass the delay effect.";
		delay_bypass_button->ParameterInfo::InfoSender::valueStringFunction = [this]()
		{
			return elan::StringFunc4(delay_bypass_button->getAssignedParameter()->getValue());
		};

		addAndMakeVisible(delay_time_slider = new FilmStripKnob(RESOURCES->images.knob3_filmstrip));
		delay_time_slider->assignParameter(module->delayModule->getParameterByName("Delay_Time"));
		delay_time_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
		delay_time_slider->ParameterInfo::InfoSender::name = "Delay Time";
		delay_time_slider->ParameterInfo::InfoSender::description = "Delay time in beats or seconds if sync is not enabled";
		delay_time_slider->ParameterInfo::InfoSender::valueStringFunction = [this]()
		{
			if (delay_temposync_button->getAssignedParameter()->getValue() > 0)
				return elan::StringFunc3(delay_time_slider->getAssignedParameter()->getValue()) + " beets";
			
			return elan::secondsToStringWithUnitTotal4(delay_time_slider->getAssignedParameter()->getValue());
		};

		addAndMakeVisible(delay_send_slider = new FilmStripKnob(RESOURCES->images.knob3_filmstrip));
		delay_send_slider->assignParameter(module->delayModule->getParameterByName("Send_Amount"));
		delay_send_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
		delay_send_slider->ParameterInfo::InfoSender::name = "Lowpass";
		delay_send_slider->ParameterInfo::InfoSender::description = "Amount of signal sent to the delay effect.";
		delay_send_slider->ParameterInfo::InfoSender::valueStringFunction = [this]()
		{
			return elan::StringFunc4(delay_send_slider->getAssignedParameter()->getValue());
		};

		addAndMakeVisible(delay_fb_slider = new FilmStripKnob(RESOURCES->images.knob3_filmstrip));
		delay_fb_slider->assignParameter(module->delayModule->getParameterByName("Feedback_Amt"));
		delay_fb_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
		delay_fb_slider->ParameterInfo::InfoSender::name = "Feedback Amount";
		delay_fb_slider->ParameterInfo::InfoSender::description = "Amount of feedback in percent";
		delay_fb_slider->ParameterInfo::InfoSender::valueStringFunction = [this]()
		{
			return elan::percentToStringWith2Decimals(delay_fb_slider->getAssignedParameter()->getValue());
		};

		addAndMakeVisible(delay_pan_slider = new FilmStripKnob(RESOURCES->images.knob3_filmstrip));
		delay_pan_slider->assignParameter(module->delayModule->getParameterByName("Pan"));
		delay_pan_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
		delay_pan_slider->ParameterInfo::InfoSender::name = "Pan";
		delay_pan_slider->ParameterInfo::InfoSender::description = "Panoramic position of the first echo";
		delay_pan_slider->ParameterInfo::InfoSender::valueStringFunction = [this]()
		{
			return elan::StringFunc3(delay_pan_slider->getAssignedParameter()->getValue());
		};

		addAndMakeVisible(delay_temposync_button = new TwoStateImageButton(RESOURCES->images.led_button_small));
		delay_temposync_button->assignParameter(module->delayModule->getParameterByName("Tempo_Sync"));
		delay_temposync_button->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
		delay_temposync_button->ParameterInfo::InfoSender::name = "Tempo Sync";
		delay_temposync_button->ParameterInfo::InfoSender::description = "Toggle tempo synchronization on/off";
		delay_temposync_button->ParameterInfo::InfoSender::valueStringFunction = [this]()
		{
			return delay_temposync_button->getAssignedParameter()->getValue() > 0 ? "Sync on" : "Sync off";
		};

		addAndMakeVisible(delay_damp_high_slider = new FilmStripKnob(RESOURCES->images.knob3_filmstrip));
		delay_damp_high_slider->assignParameter(module->delayModule->getParameterByName("High_Damp"));
		delay_damp_high_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
		delay_damp_high_slider->ParameterInfo::InfoSender::name = "High Damp";
		delay_damp_high_slider->ParameterInfo::InfoSender::description = "Cutoff frequency for the high damping (lowpass) filter";
		delay_damp_high_slider->ParameterInfo::InfoSender::valueStringFunction = [this]()
		{
			return hertzToStringWithUnitTotal4(delay_damp_high_slider->getAssignedParameter()->getValue());
		};

		addAndMakeVisible(delay_damp_low_slider = new FilmStripKnob(RESOURCES->images.knob3_filmstrip));
		delay_damp_low_slider->assignParameter(module->delayModule->getParameterByName("Low_Damp"));
		delay_damp_low_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
		delay_damp_low_slider->ParameterInfo::InfoSender::name = "Low Damp";
		delay_damp_low_slider->ParameterInfo::InfoSender::description = "Cutoff frequency for the low damping (highpass) filter";
		delay_damp_low_slider->ParameterInfo::InfoSender::valueStringFunction = [this]()
		{
			return hertzToStringWithUnitTotal4(delay_damp_low_slider->getAssignedParameter()->getValue());
		};

		addAndMakeVisible(delay_pingpong_button = new TwoStateImageButton(RESOURCES->images.led_button_small));
		delay_pingpong_button->assignParameter(module->delayModule->getParameterByName("Ping_Pong"));
		delay_pingpong_button->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
		delay_pingpong_button->ParameterInfo::InfoSender::name = "Ping Pong";
		delay_pingpong_button->ParameterInfo::InfoSender::description = "Toggle ping-pong mode (alternating pan-positions) on/off";
		delay_pingpong_button->ParameterInfo::InfoSender::valueStringFunction = [this]()
		{
			return delay_pingpong_button->getAssignedParameter()->getValue() > 0 ? "On" : "Off";
		};

		addAndMakeVisible(delay_stereo_button = new TwoStateImageButton(RESOURCES->images.led_button_small));
		delay_stereo_button->assignParameter(module->delayModule->getParameterByName("Stereo"));
		delay_stereo_button->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
		delay_stereo_button->ParameterInfo::InfoSender::name = "Stereo";
		delay_stereo_button->ParameterInfo::InfoSender::description = "Toggle true-stereo mode on/off";
		delay_stereo_button->ParameterInfo::InfoSender::valueStringFunction = [this]()
		{
			return delay_stereo_button->getAssignedParameter()->getValue() > 0 ? "Stereo taps" : "Mono taps";
		};

		addAndMakeVisible(delay_drywet_slider = new BoxSlider);
		delay_drywet_slider->assignParameter(module->delayModule->getParameterByName("Dry_Wet"));
		delay_drywet_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
		delay_drywet_slider->ParameterInfo::InfoSender::name = "Dry/Wet";
		delay_drywet_slider->ParameterInfo::InfoSender::description = "Ratio between dry and wet signal";
		delay_drywet_slider->ParameterInfo::InfoSender::valueStringFunction = [this]()
		{
			return elan::percentToStringWith2Decimals(delay_drywet_slider->getAssignedParameter()->getValue());
		};

		/* LIMITER */
		addAndMakeVisible(limiter_bypass_button = new TwoStateImageButton(RESOURCES->images.led_button_small));
		limiter_bypass_button->assignParameter(module->limiterModule->getParameterByName("Bypass"));
		limiter_bypass_button->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
		limiter_bypass_button->ParameterInfo::InfoSender::name = "Bypass";
		limiter_bypass_button->ParameterInfo::InfoSender::description = "Bypass the limiter effect";
		limiter_bypass_button->ParameterInfo::InfoSender::valueStringFunction = [this]()
		{
			return limiter_bypass_button->getAssignedParameter()->getValue() > 0 ? "Bypassed" : "Active";
		};

		addAndMakeVisible(limiter_attack_slider = new FilmStripKnob(RESOURCES->images.knob3_filmstrip));
		limiter_attack_slider->assignParameter(module->limiterModule->getParameterByName("Attack"));
		limiter_attack_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
		limiter_attack_slider->ParameterInfo::InfoSender::name = "Attack";
		limiter_attack_slider->ParameterInfo::InfoSender::description = "Attack time for the limiter's envelope detector";
		limiter_attack_slider->ParameterInfo::InfoSender::valueStringFunction = [this]()
		{
			return elan::secondsToStringWithUnitTotal4(limiter_attack_slider->getAssignedParameter()->getValue());
		};

		addAndMakeVisible(limiter_release_slider = new FilmStripKnob(RESOURCES->images.knob3_filmstrip));
		limiter_release_slider->assignParameter(module->limiterModule->getParameterByName("Release"));
		limiter_release_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
		limiter_release_slider->ParameterInfo::InfoSender::name = "Release";
		limiter_release_slider->ParameterInfo::InfoSender::description = "Release time for the limiter's envelope detector";
		limiter_release_slider->ParameterInfo::InfoSender::valueStringFunction = [this]()
		{
			return elan::secondsToStringWithUnitTotal4(limiter_release_slider->getAssignedParameter()->getValue());
		};

		addAndMakeVisible(limiter_precomp_in_slider = new FilmStripKnob(RESOURCES->images.knob3_filmstrip));
		limiter_precomp_in_slider->assignParameter(module->limiterModule->getParameterByName("In_Level"));
		limiter_precomp_in_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
		limiter_precomp_in_slider->ParameterInfo::InfoSender::name = "In Level";
		limiter_precomp_in_slider->ParameterInfo::InfoSender::description = "Pre-compression gain in decibels";
		limiter_precomp_in_slider->ParameterInfo::InfoSender::valueStringFunction = [this]()
		{
			return elan::decibelsToStringWithUnit2(limiter_precomp_in_slider->getAssignedParameter()->getValue());
		};

		addAndMakeVisible(limiter_threshold_slider = new FilmStripKnob(RESOURCES->images.knob3_filmstrip));
		limiter_threshold_slider->assignParameter(module->limiterModule->getParameterByName("Threshold"));
		limiter_threshold_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
		limiter_threshold_slider->ParameterInfo::InfoSender::name = "Threshold";
		limiter_threshold_slider->ParameterInfo::InfoSender::description = "Limit above which the signal will be attenuated in decibels";
		limiter_threshold_slider->ParameterInfo::InfoSender::valueStringFunction = [this]()
		{
			return elan::decibelsToStringWithUnit2(limiter_threshold_slider->getAssignedParameter()->getValue());
		};

		addAndMakeVisible(limiter_postcomp_out_slider = new FilmStripKnob(RESOURCES->images.knob3_filmstrip));
		limiter_postcomp_out_slider->assignParameter(module->limiterModule->getParameterByName("Out_Level"));
		limiter_postcomp_out_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
		limiter_postcomp_out_slider->ParameterInfo::InfoSender::name = "Out Level";
		limiter_postcomp_out_slider->ParameterInfo::InfoSender::description = "Post-compression gain";
		limiter_postcomp_out_slider->ParameterInfo::InfoSender::valueStringFunction = [this]()
		{
			return elan::decibelsToStringWithUnit2(limiter_postcomp_out_slider->getAssignedParameter()->getValue());
		};

		addAndMakeVisible(limiter_drywet_slider = new BoxSlider);
		limiter_drywet_slider->assignParameter(module->limiterModule->getParameterByName("Dry_Wet"));
		limiter_drywet_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
		limiter_drywet_slider->ParameterInfo::InfoSender::name = "Dry/Wet";
		limiter_drywet_slider->ParameterInfo::InfoSender::description = "Mix ratio between original and compressed signal";
		limiter_drywet_slider->ParameterInfo::InfoSender::valueStringFunction = [this]()
		{
			return elan::percentToStringWith2Decimals(limiter_drywet_slider->getAssignedParameter()->getValue());
		};
	}
	~FXPage() = default;

	EqualizerModuleEditor* equalizerEditor = nullptr;
	TwoStateImageButton* eq_bypass_button;
	VerticalHandleSlider* eq_gain_slider;

	//MenuSM* eq_range_dropdown;

	//TempBox* eq_widgetset;
	
	//TempBox* eq_band_field;
	//MenuLG* eq_band_mode_dropdown;
	//FilmStripKnob* eq_band_time_slider;
	//FilmStripKnob* eq_band_gain_slider;
	//FilmStripKnob* eq_band_level_slider;

	TwoStateImageButton* filter_bypass_button;
	FilmStripKnob* filter_lp_slider;
	FilmStripKnob* filter_hp_slider;

	TwoStateImageButton* delay_bypass_button;
	FilmStripKnob* delay_time_slider;
	FilmStripKnob* delay_send_slider;
	FilmStripKnob* delay_fb_slider;
	FilmStripKnob* delay_pan_slider;
	TwoStateImageButton* delay_temposync_button;
	FilmStripKnob* delay_damp_high_slider;
	FilmStripKnob* delay_damp_low_slider;
	TwoStateImageButton* delay_pingpong_button;
	TwoStateImageButton* delay_stereo_button;
	BoxSlider* delay_drywet_slider;

	TwoStateImageButton* limiter_bypass_button;
	FilmStripKnob* limiter_attack_slider;
	FilmStripKnob* limiter_release_slider;
	FilmStripKnob* limiter_precomp_in_slider;
	FilmStripKnob* limiter_threshold_slider;
	FilmStripKnob* limiter_postcomp_out_slider;
	BoxSlider* limiter_drywet_slider;

protected:
	void paint(Graphics& g) override
	{
		g.drawImage(*background, background->getBounds().translated(0,-1).toFloat());
	}

	void resized() override
	{
		eq_bypass_button->setBounds(536, 16, 12, 12, MidCenter);
		eq_gain_slider->setBounds(eq_gain_slider->getBoundsForSliderArea(414, 59, 6, 144));

		//eq_range_dropdown->setBounds(46, 30, 48, 19);
		//eq_widgetset->setBounds(11, 54, 400, 169);		
		//eq_band_field->setBounds(523, 67, 13, 9);
		//eq_band_mode_dropdown->setBounds(449, 105, 70, 19);
		//eq_band_time_slider->setBounds(437, 155, 22, 22);
		//eq_band_gain_slider->setBounds(468, 150, 32, 32);
		//eq_band_level_slider->setBounds(509, 155, 22, 22);

		equalizerEditor->setBounds(11, 54, 400, 169);

		filter_bypass_button->setBounds(627, 16, 12, 12, MidCenter);
		filter_lp_slider->setBounds(589, 81, 22, 22);
		filter_hp_slider->setBounds(589, 131, 22, 22);

		delay_bypass_button->setBounds(746, 16, 12, 12, MidCenter);
		delay_time_slider->setBounds(665, 36, 22, 22);
		delay_send_slider->setBounds(665, 83, 22, 22);
		delay_fb_slider->setBounds(665, 130, 22, 22);
		delay_pan_slider->setBounds(665, 177, 22, 22);
		delay_temposync_button->setBounds(707, 46, 12, 12, MidCenter);
		delay_damp_high_slider->setBounds(712, 83, 22, 22);
		delay_damp_low_slider->setBounds(712, 130, 22, 22);
		delay_pingpong_button->setBounds(707, 187, 12, 12, MidCenter);
		delay_stereo_button->setBounds(707, 205, 12, 12, MidCenter);
		delay_drywet_slider->setBounds(680, 221, 49, 8);

		limiter_bypass_button->setBounds(866, 16, 12, 12, MidCenter);
		limiter_attack_slider->setBounds(784, 36, 22, 22);
		limiter_release_slider->setBounds(842, 36, 22, 22);
		limiter_precomp_in_slider->setBounds(784, 130, 22, 22);
		limiter_threshold_slider->setBounds(824, 141, 29, 29, MidCenter);
		limiter_postcomp_out_slider->setBounds(842, 130, 22, 22);
		limiter_drywet_slider->setBounds(800, 221, 49, 8);
	}

	SharedResourcePointer<Resources> RESOURCES;
	Image* background = &RESOURCES->images.fx_bg.getImage();
};

class OMSEditor
	: public BasicEditorWithLicensing
	, public ParameterInfo::Listener
	, public ParameterValueInput::Listener
	, public PresetInformation::SenderReceiver
	, public PluginFileManager::FileMemory
{
public:

	OMSEditor(OMSModule* newOMSModule);
	~OMSEditor();

	OMSModule* module = nullptr;

	//EDITORS
	JerobeamRadarEditor* radarGenerator_editor;

	OscilloscopeEditor* oscilloscopeEditor = nullptr;
	Component oscilloscopeArea;

	PrettyScopeEditor* psedit = nullptr;

	// ERROR MESSAGE
	ErrorWindow errorWindow;

	// HELP
	ParameterInfo parameterInfo;
	elan::StaticText ParameterNameText;
	elan::StaticText ParameterValueText;
	elan::StaticText ParameterDescriptionText;

	//VALUE INPUT
	ValueEntry valueEntry;
	ParameterValueInput parameterValueInput{ this, &valueEntry, valueEntry.editor };

	//PRESETS
	PopupMenu mainMenu;
	PresetInterface presetInterface;

	//OCTAVE
	ModulatableNumberDisplay* octave_slider;
	ClickableArea* octave_up;
	ClickableArea* octave_dn;
	
	//OVERSAMPLING
	NumberDisplay* oversample_slider;
	ClickableArea* oversample_up;
	ClickableArea* oversample_dn;
	
	//STEREO
	MenuLG* channelmode_dropdown;
	//CLIPPING
	TwoStateImageButton* clipping_soft_button;
	TwoStateImageButton* clipping_hard_button;
	FilmStripKnob* clipping_gain_slider;
	FilmStripKnob* clipping_limit_slider;
	FilmStripKnob* parameterSmoothing_slider;
	FilmStripKnob* rotate_slider;
	//GAIN
	FilmStripKnob* dc_offset_slider;
	FilmStripKnob* dc_X_slider;
	FilmStripKnob* dc_Y_slider;
	//CHAOS
	MenuLG* chaos_source_dropdown;
	FilmStripKnob* chaos_LP_slider;
	FilmStripKnob* chaos_HP_slider;
	FilmStripKnob* chaos_amount_slider;
	//KEYTRACKING
	TwoStateImageButton* keytracking_button;
	TwoStateImageButton* changePitch_always_button;
	TwoStateImageButton* changePitch_legato_button;
	//TEMPO
	TwoStateImageButton* tempo_reset_button;
	FilmStripKnob* tempo_freq_slider;
	FilmStripKnob* tempo_tempo_slider;
	FilmStripKnob* tempo_multiply_slider;
	FilmStripKnob* masterRate_slider;
	//PITCH
	TwoStateImageButton* glide_always_button;
	TwoStateImageButton* glide_legato_button;
	FilmStripKnob* pitch_glide_slider;
	FilmStripKnob* pitch_tune_slider;
	FilmStripKnob* pitch_harmonic_slider;
	FilmStripKnob* pitch_harmOctGlide_slider;
	//AMP-ENV
	MenuSM* ampEnv_reset_dropdown;
	FilmStripKnob* ampEnv_speed_slider;
	FilmStripKnob* ampEnv_velScale_slider;
	VerticalHandleSlider* ampEnv_attack_slider;
	VerticalHandleSlider* ampEnv_decay_slider;
	VerticalHandleSlider* ampEnv_sustain_slider;
	VerticalHandleSlider* ampEnv_release_slider;
	VerticalHandleSlider* ampEnv_lc_slider;
	MenuSM* ampEnv_attackCurve_dropdown;
	MenuSM* ampEnv_decayCurve_dropdown;
	MenuSM* ampEnv_releaseCurve_dropdown;

	//TABS AND PAGES
	TabHandler sectionTabHandler;

	SectionTab fx_tab{ "FX" };
	FXPage fx_page{ module, parameterInfo, lock };

	// overriden callbacks
	void rButtonClicked(jura::RButton* button) override;
	void updateWidgetsAccordingToState() override;

protected:
	void resized() override;

	void paint(Graphics& g) override
	{
		RESOURCES->images.Background.draw(g);
	}

	SharedResourcePointer<Resources> RESOURCES;

	void infoComponentChanged()
	{
		ParameterNameText.setText(getParameterInfoState().name);
		ParameterDescriptionText.setText(getParameterInfoState().description);
		ParameterValueText.setText(getParameterInfoState().value);
	}

	void mouseEnter(const MouseEvent&) override
	{		
		if (oscilloscopeEditor->isMouseOverOrDragging(true))
			oscilloscopeEditor->setWidgetsToHideForMouseover(true);
		else
			oscilloscopeEditor->setWidgetsToHideForMouseover(false);
	}

	void mouseExit(const MouseEvent&) override
	{
		if (!oscilloscopeEditor->isMouseOverOrDragging(true))
			oscilloscopeEditor->setWidgetsToHideForMouseover(false);
	}

	void mouseUp(const MouseEvent& e) override
	{
		if (!oscilloscopeEditor->isMouseOverOrDragging(true))
			oscilloscopeEditor->setWidgetsToHideForMouseover(false);
	}

	void mouseDrag(const MouseEvent&) override
	{
		if (!oscilloscopeEditor->isMouseOverOrDragging(true))
			oscilloscopeEditor->setWidgetsToHideForMouseover(false);
	}

	int lk = 0, hk = 127;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OMSEditor)
};
