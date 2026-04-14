#pragma once

using namespace se;

class ChaosflyEditor
	: public BasicEditor
	, public RTextEditorListener
	, public PluginFileManager::FileMemory
{
public:
  ChaosflyEditor(ChaosflyModule *newChaosflyToEdit);

	virtual ~ChaosflyEditor() = default;

	void createWidgets();

  // overriden callbacks
  virtual void resized() override;
  virtual void rButtonClicked(RButton* button) override;
  virtual void rTextEditorTextChanged(RTextEditor& editor) override;
  virtual void updateWidgetsAccordingToState() override;

	// buttons for switching the GUI page:
	RRadioButton *pageButtonGlobal, *pageButtonModEnv, *pageButtonAmpEnv, *pageButtonComments;
	RRadioButtonGroup pageButtonGroup;

	ChaosflyModule *ChaosflyToEdit;

	vector<Component*> componentsForGlobalPage;

protected:

	ElanModulatableSlider * FMD_Freq_slider;
	ElanModulatableSlider * FMD_Res_slider;
	ElanModulatableSlider * FMD_Chaos_slider;
	ElanModulatableSlider * FMD_Input_Clip_slider;
	ElanModulatableSlider * FMD_Overdrive_slider;
	ElanModulatableSlider * FMD_Out_slider;
	ElanModulatableSlider * FMD_Out_Adjust_slider;
	ElanModulatableSlider * Parameter_Smoothing_slider;
	ElanModulatableSlider * Oversampling_slider;
	ElanModulatableComboBox * Channel_Mode_combobox;
	ElanModulatableSlider * Gain_slider;
	ElanModulatableSlider * Bias_slider;
	ElanModulatableSlider * hpFilterOut_slider;
	ElanModulatableSlider * OutputClipLevel_slider;
	ElanModulatableSlider * Osc1Amp_slider;
	ElanModulatableSlider * Osc2Amp_slider;
	ElanModulatableButton * Osc_1_Bypass_button;
	ElanModulatableButton * Osc_2_Bypass_button;
	ElanModulatableSlider * Feedback_Gain_slider;
	ElanModulatableButton * Output_Osc1_Pre_Filter_button;
	ElanModulatableButton * Modulate_With_Osc1_Pre_Filter_button;
	ElanModulatableSlider * Tune_slider;
	ElanModulatableSlider * Octave_slider;
	ElanModulatableSlider * Glide_slider;
	ElanModulatableSlider * InMix_slider;
	ElanModulatableSlider * InDetune_slider;
	ElanModulatableSlider * Ratio_slider;
	ElanModulatableSlider * ModDepth_slider;
	ElanModulatableSlider * Mod11_slider;
	ElanModulatableSlider * Mod22_slider;
	ElanModulatableSlider * Mod12_slider;
	ElanModulatableSlider * Mod21_slider;
	ElanModulatableSlider * FmVsPm11_slider;
	ElanModulatableSlider * FmVsPm22_slider;
	ElanModulatableSlider * FmVsPm12_slider;
	ElanModulatableSlider * FmVsPm21_slider;
	ElanModulatableSlider * Osc1FreqMin_slider;
	ElanModulatableSlider * Osc1FreqMax_slider;
	ElanModulatableSlider * Osc2FreqMin_slider;
	ElanModulatableSlider * Osc2FreqMax_slider;
	ElanModulatableSlider * ModEnvDepth_slider;
	ElanModulatableSlider * Filter1CutoffScale_slider;
	ElanModulatableSlider * Filter1Resonance_slider;
	ElanModulatableComboBox * Filter1Mode_combobox;
	ElanModulatableSlider * Filter2CutoffScale_slider;
	ElanModulatableSlider * Filter2Resonance_slider;
	ElanModulatableComboBox * Filter2Mode_combobox;
	ElanModulatableSlider * InputToFeedback_slider;
	ElanModulatableSlider * Filter3Detune_slider;
	ElanModulatableSlider * Filter3Gain_slider;
	ElanModulatableSlider * Filter3Width_slider;
	ElanModulatableSlider * OscInOctave_slider;
	ElanModulatableSlider * OscInDetune_slider;
	ElanModulatableSlider * OscInAmplitude_slider;
	ElanModulatableComboBox * OscInWave_combobox;
	ElanModulatableSlider * Osc1FreqScale_slider;
	ElanModulatableSlider * Osc1FreqOffset_slider;
	ElanModulatableSlider * Osc1Phase_slider;
	ElanModulatableSlider * Osc1ClipLevel_slider;
	ElanModulatableSlider * Osc1ClipCenter_slider;
	ElanModulatableComboBox * Osc1Wave_combobox;
	ElanModulatableSlider * Osc1EllipseA_slider;
	ElanModulatableSlider * Osc1EllipseB_slider;
	ElanModulatableSlider * Osc1EllipseC_slider;
	ElanModulatableSlider * Osc2FreqScale_slider;
	ElanModulatableSlider * Osc2FreqOffset_slider;
	ElanModulatableSlider * Osc2Phase_slider;
	ElanModulatableSlider * Osc2ClipLevel_slider;
	ElanModulatableSlider * Osc2ClipCenter_slider;
	ElanModulatableComboBox * Osc2Wave_combobox;
	ElanModulatableSlider * Osc2EllipseA_slider;
	ElanModulatableSlider * Osc2EllipseB_slider;
	ElanModulatableSlider * Osc2EllipseC_slider;
	ElanModulatableSlider * SyncThresh_slider;
	ElanModulatableSlider * SyncAmount_slider;
	ElanModulatableButton * SyncFilters_button;
	ElanModulatableComboBox * SyncMode_combobox;
	ElanModulatableButton * Bypass_Ducking_button;
	ElanModulatableSlider * DuckRange_slider;
	ElanModulatableSlider * DuckCenter_slider;
	ElanModulatableSlider * DuckFlatness_slider;
	ElanModulatableComboBox * DuckShape_combobox;
	ElanModulatableSlider * DuckFill_slider;
	ElanModulatableSlider * Noise_Amp_slider;
	ElanModulatableSlider * Noise_LPF_slider;
	ElanModulatableSlider * Noise_HPF_slider;
	ElanModulatableSlider * Attack_slider;
	ElanModulatableSlider * Release_slider;
	ElanModulatableSlider * LookAhead_slider;
	ElanModulatableSlider * InLevel_slider;
	ElanModulatableSlider * OutLevel_slider;
	ElanModulatableSlider * Threshold_slider;
	ElanModulatableSlider * CompRatio_slider;
	ElanModulatableSlider * KneeWidth_slider;
	ElanModulatableButton * Limit_button;
	ElanModulatableButton * AutoGain_button;
	ElanModulatableSlider * Dry_Wet_slider;

	ElanModulatableComboBox* osc1ResetMode_combobox;
	ElanModulatableComboBox* osc2ResetMode_combobox;

	ElanModulatableComboBox* ampEnvResetMode_combobox;
	ElanModulatableComboBox* modEnvResetMode_combobox;
	ElanModulatableComboBox* ampEnvTrigMode_combobox;
	ElanModulatableComboBox* modEnvTrigMode_combobox;

	ElanModulatableComboBox* glideMode_combobox;

  // sub-editors:
  jura::BreakpointModulatorEditor *editorModEnv, *editorAmpEnv;

  jura::RTextEditor *editorComments;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChaosflyEditor)
};
