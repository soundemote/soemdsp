#include "ChaosflyEditor.h"

ChaosflyEditor::ChaosflyEditor(ChaosflyModule *newChaosflyToEdit)
  : ChaosflyToEdit(newChaosflyToEdit)
	, BasicEditor(newChaosflyToEdit)
{
	ScopedLock scopedLock(*lock);

	createWidgets();

	setPresetSectionPosition(positions::RIGHT_TO_HEADLINE);
}

void ChaosflyEditor::createWidgets()
{
  // GUI page switching widgets:
  addWidget(pageButtonGlobal = new RRadioButton("Global"));
  pageButtonGlobal->addToRadioButtonGroup(&pageButtonGroup);
  pageButtonGlobal->addRButtonListener(this);

  addWidget(pageButtonModEnv = new RRadioButton("ModEnv"));
  pageButtonModEnv->addToRadioButtonGroup(&pageButtonGroup);
  pageButtonModEnv->addRButtonListener(this);

  addWidget(pageButtonAmpEnv = new RRadioButton("AmpEnv"));
  pageButtonAmpEnv->addToRadioButtonGroup(&pageButtonGroup);
  pageButtonAmpEnv->addRButtonListener(this);

  addWidget(pageButtonComments = new RRadioButton("Comments"));
  pageButtonComments->addToRadioButtonGroup(&pageButtonGroup);
  pageButtonComments->addRButtonListener(this);

  // sub-editors (must be added after the parameter widgets to be in front of them):
  editorModEnv = new BreakpointModulatorEditor(lock, ChaosflyToEdit->modEnvModule);
  editorModEnv->setHeadlineText("ModEnv");
  addChildEditor(editorModEnv);

  editorAmpEnv = new BreakpointModulatorEditor(lock, ChaosflyToEdit->ampEnvModule);
  editorAmpEnv->setHeadlineText("AmpEnv");
  addChildEditor(editorAmpEnv);
	
  editorComments = new RTextEditor("PatchCommentsEditor");
  editorComments->setMultiLine(true);
  editorComments->setReturnKeyStartsNewLine(true);
  editorComments->addListener(this);
  addWidget(editorComments);

	addWidget(FMD_Freq_slider = new ElanModulatableSlider());
	FMD_Freq_slider->assignParameter(moduleToEdit->getParameterByName("FMD_Freq"));
	FMD_Freq_slider->setName("FMD_Freq");
	FMD_Freq_slider->setDescription("");
	FMD_Freq_slider->setDescriptionField(infoField);
	FMD_Freq_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(FMD_Res_slider = new ElanModulatableSlider());
	FMD_Res_slider->assignParameter(moduleToEdit->getParameterByName("FMD_Res"));
	FMD_Res_slider->setName("FMD_Res");
	FMD_Res_slider->setDescription("");
	FMD_Res_slider->setDescriptionField(infoField);
	FMD_Res_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(FMD_Chaos_slider = new ElanModulatableSlider());
	FMD_Chaos_slider->assignParameter(moduleToEdit->getParameterByName("FMD_Chaos"));
	FMD_Chaos_slider->setName("FMD_Chaos");
	FMD_Chaos_slider->setDescription("");
	FMD_Chaos_slider->setDescriptionField(infoField);
	FMD_Chaos_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(FMD_Input_Clip_slider = new ElanModulatableSlider());
	FMD_Input_Clip_slider->assignParameter(moduleToEdit->getParameterByName("FMD_Input_Clip"));
	FMD_Input_Clip_slider->setName("FMD_Input_Clip");
	FMD_Input_Clip_slider->setDescription("");
	FMD_Input_Clip_slider->setDescriptionField(infoField);
	FMD_Input_Clip_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(FMD_Overdrive_slider = new ElanModulatableSlider());
	FMD_Overdrive_slider->assignParameter(moduleToEdit->getParameterByName("FMD_Overdrive"));
	FMD_Overdrive_slider->setName("FMD_Overdrive");
	FMD_Overdrive_slider->setDescription("");
	FMD_Overdrive_slider->setDescriptionField(infoField);
	FMD_Overdrive_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(FMD_Out_slider = new ElanModulatableSlider());
	FMD_Out_slider->assignParameter(moduleToEdit->getParameterByName("FMD_Out"));
	FMD_Out_slider->setName("FMD_Out");
	FMD_Out_slider->setDescription("");
	FMD_Out_slider->setDescriptionField(infoField);
	FMD_Out_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(FMD_Out_Adjust_slider = new ElanModulatableSlider());
	FMD_Out_Adjust_slider->assignParameter(moduleToEdit->getParameterByName("FMD_Out_Adjust"));
	FMD_Out_Adjust_slider->setName("FMD_Out_Adjust");
	FMD_Out_Adjust_slider->setDescription("");
	FMD_Out_Adjust_slider->setDescriptionField(infoField);
	FMD_Out_Adjust_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(Parameter_Smoothing_slider = new ElanModulatableSlider());
	Parameter_Smoothing_slider->assignParameter(moduleToEdit->getParameterByName("Parameter_Smoothing"));
	Parameter_Smoothing_slider->setName("Parameter_Smoothing");
	Parameter_Smoothing_slider->setDescription("");
	Parameter_Smoothing_slider->setDescriptionField(infoField);
	Parameter_Smoothing_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(Oversampling_slider = new ElanModulatableSlider());
	Oversampling_slider->assignParameter(moduleToEdit->getParameterByName("Oversampling"));
	Oversampling_slider->setName("Oversampling");
	Oversampling_slider->setDescription("");
	Oversampling_slider->setDescriptionField(infoField);
	Oversampling_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(Channel_Mode_combobox = new ElanModulatableComboBox());
	Channel_Mode_combobox->assignParameter(moduleToEdit->getParameterByName("Channel_Mode"));
	Channel_Mode_combobox->setName("Channel_Mode");
	Channel_Mode_combobox->setDescription("");
	Channel_Mode_combobox->setDescriptionField(infoField);

	addWidget(Gain_slider = new ElanModulatableSlider());
	Gain_slider->assignParameter(moduleToEdit->getParameterByName("Gain"));
	Gain_slider->setName("Gain");
	Gain_slider->setDescription("Output gain in decibels");
	Gain_slider->setDescriptionField(infoField);
	Gain_slider->setStringConversionFunction(&elan::decibelsToStringWithUnit1);

	addWidget(Bias_slider = new ElanModulatableSlider());
	Bias_slider->assignParameter(moduleToEdit->getParameterByName("Bias"));
	Bias_slider->setName("Bias");
	Bias_slider->setDescription("");
	Bias_slider->setDescriptionField(infoField);
	Bias_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(hpFilterOut_slider = new ElanModulatableSlider());
	hpFilterOut_slider->assignParameter(moduleToEdit->getParameterByName("hpFilterOut"));
	hpFilterOut_slider->setName("hpFilterOut");
	hpFilterOut_slider->setDescription("");
	hpFilterOut_slider->setDescriptionField(infoField);
	hpFilterOut_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(OutputClipLevel_slider = new ElanModulatableSlider());
	OutputClipLevel_slider->assignParameter(moduleToEdit->getParameterByName("OutputClipLevel"));
	OutputClipLevel_slider->setName("OutputClipLevel");
	OutputClipLevel_slider->setDescription("Output clipping level");
	OutputClipLevel_slider->setDescriptionField(infoField);
	OutputClipLevel_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(Osc1Amp_slider = new ElanModulatableSlider());
	Osc1Amp_slider->assignParameter(moduleToEdit->getParameterByName("Osc1Amp"));
	Osc1Amp_slider->setName("Osc1Amp");
	Osc1Amp_slider->setDescription("");
	Osc1Amp_slider->setDescriptionField(infoField);
	Osc1Amp_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(Osc2Amp_slider = new ElanModulatableSlider());
	Osc2Amp_slider->assignParameter(moduleToEdit->getParameterByName("Osc2Amp"));
	Osc2Amp_slider->setName("Osc2Amp");
	Osc2Amp_slider->setDescription("");
	Osc2Amp_slider->setDescriptionField(infoField);
	Osc2Amp_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(Osc_1_Bypass_button = new ElanModulatableButton());
	Osc_1_Bypass_button->setButtonText("Osc_1_Bypass");
	Osc_1_Bypass_button->assignParameter(moduleToEdit->getParameterByName("Osc_1_Bypass"));
	Osc_1_Bypass_button->setName("Osc_1_Bypass");
	Osc_1_Bypass_button->setDescription("");
	Osc_1_Bypass_button->setDescriptionField(infoField);

	addWidget(Osc_2_Bypass_button = new ElanModulatableButton());
	Osc_2_Bypass_button->setButtonText("Osc_2_Bypass");
	Osc_2_Bypass_button->assignParameter(moduleToEdit->getParameterByName("Osc_2_Bypass"));
	Osc_2_Bypass_button->setName("Osc_2_Bypass");
	Osc_2_Bypass_button->setDescription("");
	Osc_2_Bypass_button->setDescriptionField(infoField);

	addWidget(Feedback_Gain_slider = new ElanModulatableSlider());
	Feedback_Gain_slider->assignParameter(moduleToEdit->getParameterByName("Feedback_Gain"));
	Feedback_Gain_slider->setName("Feedback_Gain");
	Feedback_Gain_slider->setDescription("");
	Feedback_Gain_slider->setDescriptionField(infoField);
	Feedback_Gain_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(Output_Osc1_Pre_Filter_button = new ElanModulatableButton());
	Output_Osc1_Pre_Filter_button->setButtonText("Output_Osc1_Pre_Filter");
	Output_Osc1_Pre_Filter_button->assignParameter(moduleToEdit->getParameterByName("Output_Osc1_Pre_Filter"));
	Output_Osc1_Pre_Filter_button->setName("Output_Osc1_Pre_Filter");
	Output_Osc1_Pre_Filter_button->setDescription("");
	Output_Osc1_Pre_Filter_button->setDescriptionField(infoField);

	addWidget(Modulate_With_Osc1_Pre_Filter_button = new ElanModulatableButton());
	Modulate_With_Osc1_Pre_Filter_button->setButtonText("Modulate_With_Osc1_Pre_Filter");
	Modulate_With_Osc1_Pre_Filter_button->assignParameter(moduleToEdit->getParameterByName("Modulate_With_Osc1_Pre_Filter"));
	Modulate_With_Osc1_Pre_Filter_button->setName("Modulate_With_Osc1_Pre_Filter");
	Modulate_With_Osc1_Pre_Filter_button->setDescription("");
	Modulate_With_Osc1_Pre_Filter_button->setDescriptionField(infoField);

	addWidget(Tune_slider = new ElanModulatableSlider());
	Tune_slider->assignParameter(moduleToEdit->getParameterByName("Tune"));
	Tune_slider->setName("Tune");
	Tune_slider->setDescription("Master tuning in semitones");
	Tune_slider->setDescriptionField(infoField);
	Tune_slider->setStringConversionFunction(&elan::semitonesToStringWithUnit2);

	addWidget(Octave_slider = new ElanModulatableSlider());
	Octave_slider->assignParameter(moduleToEdit->getParameterByName("Octave"));
	Octave_slider->setName("Octave");
	Octave_slider->setDescription("");
	Octave_slider->setDescriptionField(infoField);
	Octave_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(Glide_slider = new ElanModulatableSlider());
	Glide_slider->assignParameter(moduleToEdit->getParameterByName("Glide"));
	Glide_slider->setName("Glide");
	Glide_slider->setDescription("Glide time in milliseconds");
	Glide_slider->setDescriptionField(infoField);
	Glide_slider->setStringConversionFunction(&millisecondsToStringWithUnit2);

	addWidget(InMix_slider = new ElanModulatableSlider());
	InMix_slider->assignParameter(moduleToEdit->getParameterByName("InMix"));
	InMix_slider->setName("InMix");
	InMix_slider->setDescription("Mix factor for input signal");
	InMix_slider->setDescriptionField(infoField);
	InMix_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(InDetune_slider = new ElanModulatableSlider());
	InDetune_slider->assignParameter(moduleToEdit->getParameterByName("InDetune"));
	InDetune_slider->setName("InDetune");
	InDetune_slider->setDescription("Input filter detune in semitones");
	InDetune_slider->setDescriptionField(infoField);
	InDetune_slider->setStringConversionFunction(&elan::semitonesToStringWithUnit2);

	addWidget(Ratio_slider = new ElanModulatableSlider());
	Ratio_slider->assignParameter(moduleToEdit->getParameterByName("Ratio"));
	Ratio_slider->setName("Ratio");
	Ratio_slider->setDescription("Frequency ratio of oscillators");
	Ratio_slider->setDescriptionField(infoField);
	Ratio_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(ModDepth_slider = new ElanModulatableSlider());
	ModDepth_slider->assignParameter(moduleToEdit->getParameterByName("ModDepth"));
	ModDepth_slider->setName("ModDepth");
	ModDepth_slider->setDescription("Modulation envelope depth");
	ModDepth_slider->setDescriptionField(infoField);
	ModDepth_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(Mod11_slider = new ElanModulatableSlider());
	Mod11_slider->assignParameter(moduleToEdit->getParameterByName("Mod11"));
	Mod11_slider->setName("Mod11");
	Mod11_slider->setDescription("");
	Mod11_slider->setDescriptionField(infoField);
	Mod11_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(Mod22_slider = new ElanModulatableSlider());
	Mod22_slider->assignParameter(moduleToEdit->getParameterByName("Mod22"));
	Mod22_slider->setName("Mod22");
	Mod22_slider->setDescription("");
	Mod22_slider->setDescriptionField(infoField);
	Mod22_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(Mod12_slider = new ElanModulatableSlider());
	Mod12_slider->assignParameter(moduleToEdit->getParameterByName("Mod12"));
	Mod12_slider->setName("Mod12");
	Mod12_slider->setDescription("Modulation 1 -> 2");
	Mod12_slider->setDescriptionField(infoField);
	Mod12_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(Mod21_slider = new ElanModulatableSlider());
	Mod21_slider->assignParameter(moduleToEdit->getParameterByName("Mod21"));
	Mod21_slider->setName("Mod21");
	Mod21_slider->setDescription("Modulation 2 -> 1");
	Mod21_slider->setDescriptionField(infoField);
	Mod21_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(FmVsPm11_slider = new ElanModulatableSlider());
	FmVsPm11_slider->assignParameter(moduleToEdit->getParameterByName("FmVsPm11"));
	FmVsPm11_slider->setName("FmVsPm11");
	FmVsPm11_slider->setDescription("FM vs PM 2 -> 1");
	FmVsPm11_slider->setDescriptionField(infoField);
	FmVsPm11_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(FmVsPm22_slider = new ElanModulatableSlider());
	FmVsPm22_slider->assignParameter(moduleToEdit->getParameterByName("FmVsPm22"));
	FmVsPm22_slider->setName("FmVsPm22");
	FmVsPm22_slider->setDescription("");
	FmVsPm22_slider->setDescriptionField(infoField);
	FmVsPm22_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(FmVsPm12_slider = new ElanModulatableSlider());
	FmVsPm12_slider->assignParameter(moduleToEdit->getParameterByName("FmVsPm12"));
	FmVsPm12_slider->setName("FmVsPm12");
	FmVsPm12_slider->setDescription("FM vs PM 1 -> 2");
	FmVsPm12_slider->setDescriptionField(infoField);
	FmVsPm12_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(FmVsPm21_slider = new ElanModulatableSlider());
	FmVsPm21_slider->assignParameter(moduleToEdit->getParameterByName("FmVsPm21"));
	FmVsPm21_slider->setName("FmVsPm21");
	FmVsPm21_slider->setDescription("");
	FmVsPm21_slider->setDescriptionField(infoField);
	FmVsPm21_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(Osc1FreqMin_slider = new ElanModulatableSlider());
	Osc1FreqMin_slider->assignParameter(moduleToEdit->getParameterByName("Osc1FreqMin"));
	Osc1FreqMin_slider->setName("Osc1FreqMin");
	Osc1FreqMin_slider->setDescription("Oscillator 1 frequency lower limit");
	Osc1FreqMin_slider->setDescriptionField(infoField);
	Osc1FreqMin_slider->setStringConversionFunction(&elan::hertzToStringWithUnitTotal5);

	addWidget(Osc1FreqMax_slider = new ElanModulatableSlider());
	Osc1FreqMax_slider->assignParameter(moduleToEdit->getParameterByName("Osc1FreqMax"));
	Osc1FreqMax_slider->setName("Osc1FreqMax");
	Osc1FreqMax_slider->setDescription("Oscillator 1 frequency upper limit");
	Osc1FreqMax_slider->setDescriptionField(infoField);
	Osc1FreqMax_slider->setStringConversionFunction(&elan::hertzToStringWithUnitTotal5);

	addWidget(Osc2FreqMin_slider = new ElanModulatableSlider());
	Osc2FreqMin_slider->assignParameter(moduleToEdit->getParameterByName("Osc2FreqMin"));
	Osc2FreqMin_slider->setName("Osc2FreqMin");
	Osc2FreqMin_slider->setDescription("Oscillator 2 frequency lower limit");
	Osc2FreqMin_slider->setDescriptionField(infoField);
	Osc2FreqMin_slider->setStringConversionFunction(&elan::hertzToStringWithUnitTotal5);

	addWidget(Osc2FreqMax_slider = new ElanModulatableSlider());
	Osc2FreqMax_slider->assignParameter(moduleToEdit->getParameterByName("Osc2FreqMax"));
	Osc2FreqMax_slider->setName("Osc2FreqMax");
	Osc2FreqMax_slider->setDescription("Oscillator 2 frequency upper limit");
	Osc2FreqMax_slider->setDescriptionField(infoField);
	Osc2FreqMax_slider->setStringConversionFunction(&elan::hertzToStringWithUnitTotal5);

	addWidget(ModEnvDepth_slider = new ElanModulatableSlider());
	ModEnvDepth_slider->assignParameter(moduleToEdit->getParameterByName("ModEnvDepth"));
	ModEnvDepth_slider->setName("ModEnvDepth");
	ModEnvDepth_slider->setDescription("Modulation envelope depth");
	ModEnvDepth_slider->setDescriptionField(infoField);
	ModEnvDepth_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(Filter1CutoffScale_slider = new ElanModulatableSlider());
	Filter1CutoffScale_slider->assignParameter(moduleToEdit->getParameterByName("Filter1CutoffScale"));
	Filter1CutoffScale_slider->setName("Filter1CutoffScale");
	Filter1CutoffScale_slider->setDescription("Filter 1 cutoff scaler");
	Filter1CutoffScale_slider->setDescriptionField(infoField);
	Filter1CutoffScale_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(Filter1Resonance_slider = new ElanModulatableSlider());
	Filter1Resonance_slider->assignParameter(moduleToEdit->getParameterByName("Filter1Resonance"));
	Filter1Resonance_slider->setName("Filter1Resonance");
	Filter1Resonance_slider->setDescription("Filter 1 resonance");
	Filter1Resonance_slider->setDescriptionField(infoField);
	Filter1Resonance_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(Filter2CutoffScale_slider = new ElanModulatableSlider());
	Filter2CutoffScale_slider->assignParameter(moduleToEdit->getParameterByName("Filter2CutoffScale"));
	Filter2CutoffScale_slider->setName("Filter2CutoffScale");
	Filter2CutoffScale_slider->setDescription("Filter 2 cutoff scaler");
	Filter2CutoffScale_slider->setDescriptionField(infoField);
	Filter2CutoffScale_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(Filter2Resonance_slider = new ElanModulatableSlider());
	Filter2Resonance_slider->assignParameter(moduleToEdit->getParameterByName("Filter2Resonance"));
	Filter2Resonance_slider->setName("Filter2Resonance");
	Filter2Resonance_slider->setDescription("Filter 2 resonance");
	Filter2Resonance_slider->setDescriptionField(infoField);
	Filter2Resonance_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(Filter1Mode_combobox = new ElanModulatableComboBox());
	Filter1Mode_combobox->assignParameter(moduleToEdit->getParameterByName("Filter1Mode"));
	Filter1Mode_combobox->setName("Filter1Mode");
	Filter1Mode_combobox->setDescription("Filter 1 frequency response type");
	Filter1Mode_combobox->setDescriptionField(infoField);

	addWidget(Filter2Mode_combobox = new ElanModulatableComboBox());
	Filter2Mode_combobox->assignParameter(moduleToEdit->getParameterByName("Filter2Mode"));
	Filter2Mode_combobox->setName("Filter2Mode");
	Filter2Mode_combobox->setDescription("Filter 2 frequency response type");
	Filter2Mode_combobox->setDescriptionField(infoField);

	addWidget(InputToFeedback_slider = new ElanModulatableSlider());
	InputToFeedback_slider->assignParameter(moduleToEdit->getParameterByName("InputToFeedback"));
	InputToFeedback_slider->setName("InputToFeedback");
	InputToFeedback_slider->setDescription("Route input signal into feedback path");
	InputToFeedback_slider->setDescriptionField(infoField);
	InputToFeedback_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(Filter3Detune_slider = new ElanModulatableSlider());
	Filter3Detune_slider->assignParameter(moduleToEdit->getParameterByName("Filter3Detune"));
	Filter3Detune_slider->setName("Filter3Detune");
	Filter3Detune_slider->setDescription("Filter 3 detune in semitones");
	Filter3Detune_slider->setDescriptionField(infoField);
	Filter3Detune_slider->setStringConversionFunction(&elan::semitonesToStringWithUnit2);

	addWidget(Filter3Gain_slider = new ElanModulatableSlider());
	Filter3Gain_slider->assignParameter(moduleToEdit->getParameterByName("Filter3Gain"));
	Filter3Gain_slider->setName("Filter3Gain");
	Filter3Gain_slider->setDescription("Filter 3 gain in decibels");
	Filter3Gain_slider->setDescriptionField(infoField);
	Filter3Gain_slider->setStringConversionFunction(&elan::decibelsToStringWithUnit2);

	addWidget(Filter3Width_slider = new ElanModulatableSlider());
	Filter3Width_slider->assignParameter(moduleToEdit->getParameterByName("Filter3Width"));
	Filter3Width_slider->setName("Filter3Width");
	Filter3Width_slider->setDescription("Filter 3 bandwidth in octaves");
	Filter3Width_slider->setDescriptionField(infoField);
	Filter3Width_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(OscInOctave_slider = new ElanModulatableSlider());
	OscInOctave_slider->assignParameter(moduleToEdit->getParameterByName("OscInOctave"));
	OscInOctave_slider->setName("OscInOctave");
	OscInOctave_slider->setDescription("");
	OscInOctave_slider->setDescriptionField(infoField);
	OscInOctave_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(OscInDetune_slider = new ElanModulatableSlider());
	OscInDetune_slider->assignParameter(moduleToEdit->getParameterByName("OscInDetune"));
	OscInDetune_slider->setName("OscInDetune");
	OscInDetune_slider->setDescription("Detuning of input oscillator in semitones");
	OscInDetune_slider->setDescriptionField(infoField);
	OscInDetune_slider->setStringConversionFunction(&elan::semitonesToStringWithUnit2);

	addWidget(OscInAmplitude_slider = new ElanModulatableSlider());
	OscInAmplitude_slider->assignParameter(moduleToEdit->getParameterByName("OscInAmplitude"));
	OscInAmplitude_slider->setName("OscInAmplitude");
	OscInAmplitude_slider->setDescription("Amplitude of input oscillator as raw multiplier");
	OscInAmplitude_slider->setDescriptionField(infoField);
	OscInAmplitude_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(OscInWave_combobox = new ElanModulatableComboBox());
	OscInWave_combobox->assignParameter(moduleToEdit->getParameterByName("OscInWave"));
	OscInWave_combobox->setName("OscInWave");
	OscInWave_combobox->setDescription("Input oscillator waveform");
	OscInWave_combobox->setDescriptionField(infoField);

	addWidget(Osc1FreqScale_slider = new ElanModulatableSlider());
	Osc1FreqScale_slider->assignParameter(moduleToEdit->getParameterByName("Osc1FreqScale"));
	Osc1FreqScale_slider->setName("Osc1FreqScale");
	Osc1FreqScale_slider->setDescription("Osc 1 frequency scaling");
	Osc1FreqScale_slider->setDescriptionField(infoField);
	Osc1FreqScale_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(Osc1FreqOffset_slider = new ElanModulatableSlider());
	Osc1FreqOffset_slider->assignParameter(moduleToEdit->getParameterByName("Osc1FreqOffset"));
	Osc1FreqOffset_slider->setName("Osc1FreqOffset");
	Osc1FreqOffset_slider->setDescription("Osc 1 frequency offset in Hz");
	Osc1FreqOffset_slider->setDescriptionField(infoField);
	Osc1FreqOffset_slider->setStringConversionFunction(&elan::hertzToStringWithUnitTotal5);

	addWidget(Osc1Phase_slider = new ElanModulatableSlider());
	Osc1Phase_slider->assignParameter(moduleToEdit->getParameterByName("Osc1Phase"));
	Osc1Phase_slider->setName("Osc1Phase");
	Osc1Phase_slider->setDescription("Osc 1 start phase in degrees");
	Osc1Phase_slider->setDescriptionField(infoField);
	Osc1Phase_slider->setStringConversionFunction(&degreesToStringWithUnit0);

	addWidget(Osc1ClipLevel_slider = new ElanModulatableSlider());
	Osc1ClipLevel_slider->assignParameter(moduleToEdit->getParameterByName("Osc1ClipLevel"));
	Osc1ClipLevel_slider->setName("Osc1ClipLevel");
	Osc1ClipLevel_slider->setDescription("Osc 1 clipping level");
	Osc1ClipLevel_slider->setDescriptionField(infoField);
	Osc1ClipLevel_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(Osc1ClipCenter_slider = new ElanModulatableSlider());
	Osc1ClipCenter_slider->assignParameter(moduleToEdit->getParameterByName("Osc1ClipCenter"));
	Osc1ClipCenter_slider->setName("Osc1ClipCenter");
	Osc1ClipCenter_slider->setDescription("Osc 1 clipping center");
	Osc1ClipCenter_slider->setDescriptionField(infoField);
	Osc1ClipCenter_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(Osc1Wave_combobox = new ElanModulatableComboBox());
	Osc1Wave_combobox->assignParameter(moduleToEdit->getParameterByName("Osc1Wave"));
	Osc1Wave_combobox->setName("Osc1Wave");
	Osc1Wave_combobox->setDescription("Oscillator 1 waveform");
	Osc1Wave_combobox->setDescriptionField(infoField);

	addWidget(Osc1EllipseA_slider = new ElanModulatableSlider());
	Osc1EllipseA_slider->assignParameter(moduleToEdit->getParameterByName("Osc1EllipseA"));
	Osc1EllipseA_slider->setName("Osc1EllipseA");
	Osc1EllipseA_slider->setDescription("");
	Osc1EllipseA_slider->setDescriptionField(infoField);
	Osc1EllipseA_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(Osc1EllipseB_slider = new ElanModulatableSlider());
	Osc1EllipseB_slider->assignParameter(moduleToEdit->getParameterByName("Osc1EllipseB"));
	Osc1EllipseB_slider->setName("Osc1EllipseB");
	Osc1EllipseB_slider->setDescription("");
	Osc1EllipseB_slider->setDescriptionField(infoField);
	Osc1EllipseB_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(Osc1EllipseC_slider = new ElanModulatableSlider());
	Osc1EllipseC_slider->assignParameter(moduleToEdit->getParameterByName("Osc1EllipseC"));
	Osc1EllipseC_slider->setName("Osc1EllipseC");
	Osc1EllipseC_slider->setDescription("");
	Osc1EllipseC_slider->setDescriptionField(infoField);
	Osc1EllipseC_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(Osc2FreqScale_slider = new ElanModulatableSlider());
	Osc2FreqScale_slider->assignParameter(moduleToEdit->getParameterByName("Osc2FreqScale"));
	Osc2FreqScale_slider->setName("Osc2FreqScale");
	Osc2FreqScale_slider->setDescription("Osc 2 frequency scaling");
	Osc2FreqScale_slider->setDescriptionField(infoField);
	Osc2FreqScale_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(Osc2FreqOffset_slider = new ElanModulatableSlider());
	Osc2FreqOffset_slider->assignParameter(moduleToEdit->getParameterByName("Osc2FreqOffset"));
	Osc2FreqOffset_slider->setName("Osc2FreqOffset");
	Osc2FreqOffset_slider->setDescription("Osc 2 frequency offset in Hz");
	Osc2FreqOffset_slider->setDescriptionField(infoField);
	Osc2FreqOffset_slider->setStringConversionFunction(&elan::hertzToStringWithUnitTotal5);

	addWidget(Osc2Phase_slider = new ElanModulatableSlider());
	Osc2Phase_slider->assignParameter(moduleToEdit->getParameterByName("Osc2Phase"));
	Osc2Phase_slider->setName("Osc2Phase");
	Osc2Phase_slider->setDescription("Osc 2 start phase in degrees");
	Osc2Phase_slider->setDescriptionField(infoField);
	Osc2Phase_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(Osc2ClipLevel_slider = new ElanModulatableSlider());
	Osc2ClipLevel_slider->assignParameter(moduleToEdit->getParameterByName("Osc2ClipLevel"));
	Osc2ClipLevel_slider->setName("Osc2ClipLevel");
	Osc2ClipLevel_slider->setDescription("Osc 2 clipping level");
	Osc2ClipLevel_slider->setDescriptionField(infoField);
	Osc2ClipLevel_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(Osc2ClipCenter_slider = new ElanModulatableSlider());
	Osc2ClipCenter_slider->assignParameter(moduleToEdit->getParameterByName("Osc2ClipCenter"));
	Osc2ClipCenter_slider->setName("Osc2ClipCenter");
	Osc2ClipCenter_slider->setDescription("Osc 2 clipping center");
	Osc2ClipCenter_slider->setDescriptionField(infoField);
	Osc2ClipCenter_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(Osc2Wave_combobox = new ElanModulatableComboBox());
	Osc2Wave_combobox->assignParameter(moduleToEdit->getParameterByName("Osc2Wave"));
	Osc2Wave_combobox->setName("Osc2Wave");
	Osc2Wave_combobox->setDescription("Oscillator 2 waveform");
	Osc2Wave_combobox->setDescriptionField(infoField);

	addWidget(Osc2EllipseA_slider = new ElanModulatableSlider());
	Osc2EllipseA_slider->assignParameter(moduleToEdit->getParameterByName("Osc2EllipseA"));
	Osc2EllipseA_slider->setName("Osc2EllipseA");
	Osc2EllipseA_slider->setDescription("");
	Osc2EllipseA_slider->setDescriptionField(infoField);
	Osc2EllipseA_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(Osc2EllipseB_slider = new ElanModulatableSlider());
	Osc2EllipseB_slider->assignParameter(moduleToEdit->getParameterByName("Osc2EllipseB"));
	Osc2EllipseB_slider->setName("Osc2EllipseB");
	Osc2EllipseB_slider->setDescription("");
	Osc2EllipseB_slider->setDescriptionField(infoField);
	Osc2EllipseB_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(Osc2EllipseC_slider = new ElanModulatableSlider());
	Osc2EllipseC_slider->assignParameter(moduleToEdit->getParameterByName("Osc2EllipseC"));
	Osc2EllipseC_slider->setName("Osc2EllipseC");
	Osc2EllipseC_slider->setDescription("");
	Osc2EllipseC_slider->setDescriptionField(infoField);
	Osc2EllipseC_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(SyncThresh_slider = new ElanModulatableSlider());
	SyncThresh_slider->assignParameter(moduleToEdit->getParameterByName("SyncThresh"));
	SyncThresh_slider->setName("SyncThresh");
	SyncThresh_slider->setDescription("Synchronization threshold");
	SyncThresh_slider->setDescriptionField(infoField);
	SyncThresh_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(SyncAmount_slider = new ElanModulatableSlider());
	SyncAmount_slider->assignParameter(moduleToEdit->getParameterByName("SyncAmount"));
	SyncAmount_slider->setName("SyncAmount");
	SyncAmount_slider->setDescription("Synchronization amount");
	SyncAmount_slider->setDescriptionField(infoField);
	SyncAmount_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(SyncFilters_button = new ElanModulatableButton());
	SyncFilters_button->setButtonText("SyncFilters");
	SyncFilters_button->assignParameter(moduleToEdit->getParameterByName("SyncFilters"));
	SyncFilters_button->setName("SyncFilters");
	SyncFilters_button->setDescription("Reset filters on sync events");
	SyncFilters_button->setDescriptionField(infoField);

	addWidget(SyncMode_combobox = new ElanModulatableComboBox());
	SyncMode_combobox->assignParameter(moduleToEdit->getParameterByName("SyncMode"));
	SyncMode_combobox->setName("SyncMode");
	SyncMode_combobox->setDescription("Synchronization mode");
	SyncMode_combobox->setDescriptionField(infoField);

	addWidget(Bypass_Ducking_button = new ElanModulatableButton());
	Bypass_Ducking_button->setButtonText("Bypass_Ducking");
	Bypass_Ducking_button->assignParameter(moduleToEdit->getParameterByName("Bypass_Ducking"));
	Bypass_Ducking_button->setName("Bypass_Ducking");
	Bypass_Ducking_button->setDescription("");
	Bypass_Ducking_button->setDescriptionField(infoField);

	addWidget(DuckRange_slider = new ElanModulatableSlider());
	DuckRange_slider->assignParameter(moduleToEdit->getParameterByName("DuckRange"));
	DuckRange_slider->setName("DuckRange");
	DuckRange_slider->setDescription("Ducking by input: width of allowed range");
	DuckRange_slider->setDescriptionField(infoField);
	DuckRange_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(DuckCenter_slider = new ElanModulatableSlider());
	DuckCenter_slider->assignParameter(moduleToEdit->getParameterByName("DuckCenter"));
	DuckCenter_slider->setName("DuckCenter");
	DuckCenter_slider->setDescription("Center of allowed range");
	DuckCenter_slider->setDescriptionField(infoField);
	DuckCenter_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(DuckFlatness_slider = new ElanModulatableSlider());
	DuckFlatness_slider->assignParameter(moduleToEdit->getParameterByName("DuckFlatness"));
	DuckFlatness_slider->setName("DuckFlatness");
	DuckFlatness_slider->setDescription("Relative length of flat region");
	DuckFlatness_slider->setDescriptionField(infoField);
	DuckFlatness_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(DuckShape_combobox = new ElanModulatableComboBox());
	DuckShape_combobox->assignParameter(moduleToEdit->getParameterByName("DuckShape"));
	DuckShape_combobox->setName("DuckShape");
	DuckShape_combobox->setDescription("Shape of transition");
	DuckShape_combobox->setDescriptionField(infoField);

	addWidget(DuckFill_slider = new ElanModulatableSlider());
	DuckFill_slider->assignParameter(moduleToEdit->getParameterByName("DuckFill"));
	DuckFill_slider->setName("DuckFill");
	DuckFill_slider->setDescription("Filling-the-square parameter");
	DuckFill_slider->setDescriptionField(infoField);
	DuckFill_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(Noise_Amp_slider = new ElanModulatableSlider());
	Noise_Amp_slider->assignParameter(moduleToEdit->getParameterByName("Noise_Amp"));
	Noise_Amp_slider->setName("Noise_Amp");
	Noise_Amp_slider->setDescription("");
	Noise_Amp_slider->setDescriptionField(infoField);
	Noise_Amp_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(Noise_LPF_slider = new ElanModulatableSlider());
	Noise_LPF_slider->assignParameter(moduleToEdit->getParameterByName("Noise_LPF"));
	Noise_LPF_slider->setName("Noise_LPF");
	Noise_LPF_slider->setDescription("");
	Noise_LPF_slider->setDescriptionField(infoField);
	Noise_LPF_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(Noise_HPF_slider = new ElanModulatableSlider());
	Noise_HPF_slider->assignParameter(moduleToEdit->getParameterByName("Noise_HPF"));
	Noise_HPF_slider->setName("Noise_HPF");
	Noise_HPF_slider->setDescription("");
	Noise_HPF_slider->setDescriptionField(infoField);
	Noise_HPF_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(Attack_slider = new ElanModulatableSlider());
	Attack_slider->assignParameter(moduleToEdit->getParameterByName("Attack"));
	Attack_slider->setName("Attack");
	Attack_slider->setDescription("");
	Attack_slider->setDescriptionField(infoField);
	Attack_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(Release_slider = new ElanModulatableSlider());
	Release_slider->assignParameter(moduleToEdit->getParameterByName("Release"));
	Release_slider->setName("Release");
	Release_slider->setDescription("");
	Release_slider->setDescriptionField(infoField);
	Release_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(LookAhead_slider = new ElanModulatableSlider());
	LookAhead_slider->assignParameter(moduleToEdit->getParameterByName("LookAhead"));
	LookAhead_slider->setName("LookAhead");
	LookAhead_slider->setDescription("");
	LookAhead_slider->setDescriptionField(infoField);
	LookAhead_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(InLevel_slider = new ElanModulatableSlider());
	InLevel_slider->assignParameter(moduleToEdit->getParameterByName("InLevel"));
	InLevel_slider->setName("InLevel");
	InLevel_slider->setDescription("");
	InLevel_slider->setDescriptionField(infoField);
	InLevel_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(OutLevel_slider = new ElanModulatableSlider());
	OutLevel_slider->assignParameter(moduleToEdit->getParameterByName("OutLevel"));
	OutLevel_slider->setName("OutLevel");
	OutLevel_slider->setDescription("");
	OutLevel_slider->setDescriptionField(infoField);
	OutLevel_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(Threshold_slider = new ElanModulatableSlider());
	Threshold_slider->assignParameter(moduleToEdit->getParameterByName("Threshold"));
	Threshold_slider->setName("Threshold");
	Threshold_slider->setDescription("");
	Threshold_slider->setDescriptionField(infoField);
	Threshold_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(CompRatio_slider = new ElanModulatableSlider());
	CompRatio_slider->assignParameter(moduleToEdit->getParameterByName("CompRatio"));
	CompRatio_slider->setName("CompRatio");
	CompRatio_slider->setDescription("");
	CompRatio_slider->setDescriptionField(infoField);
	CompRatio_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(KneeWidth_slider = new ElanModulatableSlider());
	KneeWidth_slider->assignParameter(moduleToEdit->getParameterByName("KneeWidth"));
	KneeWidth_slider->setName("KneeWidth");
	KneeWidth_slider->setDescription("");
	KneeWidth_slider->setDescriptionField(infoField);
	KneeWidth_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(Limit_button = new ElanModulatableButton());
	Limit_button->setButtonText("Limit");
	Limit_button->assignParameter(moduleToEdit->getParameterByName("Limit"));
	Limit_button->setName("Limit");
	Limit_button->setDescription("");
	Limit_button->setDescriptionField(infoField);

	addWidget(AutoGain_button = new ElanModulatableButton());
	AutoGain_button->setButtonText("AutoGain");
	AutoGain_button->assignParameter(moduleToEdit->getParameterByName("AutoGain"));
	AutoGain_button->setName("AutoGain");
	AutoGain_button->setDescription("");
	AutoGain_button->setDescriptionField(infoField);

	addWidget(Dry_Wet_slider = new ElanModulatableSlider());
	Dry_Wet_slider->assignParameter(moduleToEdit->getParameterByName("Dry_Wet"));
	Dry_Wet_slider->setName("Dry_Wet");
	Dry_Wet_slider->setDescription("");
	Dry_Wet_slider->setDescriptionField(infoField);
	Dry_Wet_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(osc1ResetMode_combobox = new ElanModulatableComboBox());
	osc1ResetMode_combobox->assignParameter(moduleToEdit->getParameterByName("osc1ResetMode"));
	osc1ResetMode_combobox->setName("osc1ResetMode");
	osc1ResetMode_combobox->setDescription("When to reset oscillator 1 based on note events");
	osc1ResetMode_combobox->setDescriptionField(infoField);

	addWidget(osc2ResetMode_combobox = new ElanModulatableComboBox());
	osc2ResetMode_combobox->assignParameter(moduleToEdit->getParameterByName("osc2ResetMode"));
	osc2ResetMode_combobox->setName("osc2ResetMode");
	osc2ResetMode_combobox->setDescription("When to reset oscillator 2 based on note events");
	osc2ResetMode_combobox->setDescriptionField(infoField);

	addWidget(ampEnvResetMode_combobox = new ElanModulatableComboBox());
	ampEnvResetMode_combobox->assignParameter(moduleToEdit->getParameterByName("ampEnvResetMode"));
	ampEnvResetMode_combobox->setName("ampEnvResetMode");
	ampEnvResetMode_combobox->setDescription("When to reset amplitude envelope based on note events");
	ampEnvResetMode_combobox->setDescriptionField(infoField);

	addWidget(modEnvResetMode_combobox = new ElanModulatableComboBox());
	modEnvResetMode_combobox->assignParameter(moduleToEdit->getParameterByName("modEnvResetMode"));
	modEnvResetMode_combobox->setName("modEnvResetMode");
	modEnvResetMode_combobox->setDescription("When to reset mod envelope based on note events");
	modEnvResetMode_combobox->setDescriptionField(infoField);

	addWidget(ampEnvTrigMode_combobox = new ElanModulatableComboBox());
	ampEnvTrigMode_combobox->assignParameter(moduleToEdit->getParameterByName("ampEnvTrigMode"));
	ampEnvTrigMode_combobox->setName("ampEnvTrigMode");
	ampEnvTrigMode_combobox->setDescription("When to retrigger amp envelope based on note events");
	ampEnvTrigMode_combobox->setDescriptionField(infoField);

	addWidget(modEnvTrigMode_combobox = new ElanModulatableComboBox());
	modEnvTrigMode_combobox->assignParameter(moduleToEdit->getParameterByName("modEnvTrigMode"));
	modEnvTrigMode_combobox->setName("modEnvTrigMode");
	modEnvTrigMode_combobox->setDescription("When to retrigger mod envelope based on note events");
	modEnvTrigMode_combobox->setDescriptionField(infoField);

	addWidget(glideMode_combobox = new ElanModulatableComboBox());
	glideMode_combobox->assignParameter(moduleToEdit->getParameterByName("glideMode"));
	glideMode_combobox->setName("glideMode");
	glideMode_combobox->setDescription("When to do pitch glide based on note events");
	glideMode_combobox->setDescriptionField(infoField);

	componentsForGlobalPage = vector<Component*>
	{
		FMD_Freq_slider,
		FMD_Res_slider,
		FMD_Chaos_slider,
		FMD_Input_Clip_slider,
		FMD_Overdrive_slider,
		FMD_Out_slider,
		FMD_Out_Adjust_slider,
		Parameter_Smoothing_slider,
		Oversampling_slider,
		Channel_Mode_combobox,
		Gain_slider,
		Bias_slider,
		hpFilterOut_slider,
		OutputClipLevel_slider,
		Osc1Amp_slider,
		Osc2Amp_slider,
		Osc_1_Bypass_button,
		Osc_2_Bypass_button,
		Feedback_Gain_slider,
		Output_Osc1_Pre_Filter_button,
		Modulate_With_Osc1_Pre_Filter_button,
		Tune_slider,
		Octave_slider,
		Glide_slider,
		InMix_slider,
		InDetune_slider,
		Ratio_slider,
		ModDepth_slider,
		Mod11_slider,
		Mod22_slider,
		Mod12_slider,
		Mod21_slider,
		FmVsPm11_slider,
		FmVsPm22_slider,
		FmVsPm12_slider,
		FmVsPm21_slider,
		Osc1FreqMin_slider,
		Osc1FreqMax_slider,
		Osc2FreqMin_slider,
		Osc2FreqMax_slider,
		ModEnvDepth_slider,
		Filter1CutoffScale_slider,
		Filter1Resonance_slider,
		Filter1Mode_combobox,
		Filter2CutoffScale_slider,
		Filter2Resonance_slider,
		Filter2Mode_combobox,
		InputToFeedback_slider,
		Filter3Detune_slider,
		Filter3Gain_slider,
		Filter3Width_slider,
		OscInOctave_slider,
		OscInDetune_slider,
		OscInAmplitude_slider,
		OscInWave_combobox,
		Osc1FreqScale_slider,
		Osc1FreqOffset_slider,
		Osc1Phase_slider,
		Osc1ClipLevel_slider,
		Osc1ClipCenter_slider,
		Osc1Wave_combobox,
		Osc1EllipseA_slider,
		Osc1EllipseB_slider,
		Osc1EllipseC_slider,
		Osc2FreqScale_slider,
		Osc2FreqOffset_slider,
		Osc2Phase_slider,
		Osc2ClipLevel_slider,
		Osc2ClipCenter_slider,
		Osc2Wave_combobox,
		Osc2EllipseA_slider,
		Osc2EllipseB_slider,
		Osc2EllipseC_slider,
		SyncThresh_slider,
		SyncAmount_slider,
		SyncFilters_button,
		SyncMode_combobox,
		Bypass_Ducking_button,
		DuckRange_slider,
		DuckCenter_slider,
		DuckFlatness_slider,
		DuckShape_combobox,
		DuckFill_slider,
		Noise_Amp_slider,
		Noise_LPF_slider,
		Noise_HPF_slider,
		Attack_slider,
		Release_slider,
		LookAhead_slider,
		InLevel_slider,
		OutLevel_slider,
		Threshold_slider,
		CompRatio_slider,
		KneeWidth_slider,
		Limit_button,
		AutoGain_button,
		Dry_Wet_slider,
		osc1ResetMode_combobox,
		osc2ResetMode_combobox,
		ampEnvResetMode_combobox,
		modEnvResetMode_combobox,
		ampEnvTrigMode_combobox,
		modEnvTrigMode_combobox,
		glideMode_combobox,
	};

}

void ChaosflyEditor::rButtonClicked(RButton* button)
{
	if (pageButtonGroup.isButtonMemberOfGroup(button))
	{
		editorModEnv->setVisible(pageButtonModEnv->getToggleState());
		editorAmpEnv->setVisible(pageButtonAmpEnv->getToggleState());
		editorComments->setVisible(pageButtonComments->getToggleState());
		for (const auto & c : componentsForGlobalPage)
			c->setVisible(pageButtonGlobal->getToggleState());
	}
}

void ChaosflyEditor::rTextEditorTextChanged(RTextEditor& editor)
{
	if (&editor == editorComments)
	{
		ChaosflyToEdit->patchComments = editor.getText();
		ChaosflyToEdit->markStateAsDirty();
	}
}

void ChaosflyEditor::updateWidgetsAccordingToState()
{
	AudioModuleEditor::updateWidgetsAccordingToState();

	editorModEnv->updateWidgetsAccordingToState();
	editorAmpEnv->updateWidgetsAccordingToState();
	editorComments->setText(ChaosflyToEdit->patchComments);
}

AudioModuleEditor * ChaosflyModule::createEditor(int /*type*/)
{
	auto ptr = new ChaosflyEditor(this);

	ptr->setSize(600, 524);

	ptr->pageButtonGlobal->setToggleState(true, true);

	return ptr;
}

void ChaosflyEditor::resized()
{
  ScopedLock scopedLock(*lock);

	BasicEditor::resized();

	//const auto & e = ChaosflyToEdit;

  int x  = 0;
  int y  = 16+8;
  int w  = getWidth();
  int w3 = w/3;
  int h  = getHeight();

  int m  = 4;      // margin

  int sh = 16;     // slider height
  int dy = sh+8;   // delta y between between widget groups
  int sw = w3-2*m; // slider width

  int pbw = 70;    // page button width
  pageButtonGlobal->setBounds(x+m, y, pbw, 20); x += pbw+m;
  pageButtonModEnv->setBounds(x+m, y, pbw, 20); x += pbw+m;
  pageButtonAmpEnv->setBounds(x+m, y, pbw, 20); x += pbw+m;
  pageButtonComments->setBounds(x+m, y, pbw, 20); x += pbw+m;
	
  x = 0;
  y = pageButtonGlobal->getBottom() + 4;

  // set up bounds for sub-editors:
  editorModEnv->setBounds(x, y, w, h-y);
  editorAmpEnv->setBounds(x, y, w, h-y);
  editorComments->setBounds(x, y, w, h-y);

  // left column:
	Oversampling_slider->setBounds(m, y, sw, sh); y += sh - 2;
	Parameter_Smoothing_slider->setBounds(m, y, sw, sh); y += dy;

	Channel_Mode_combobox->setBounds(m, y, sw, sh); y += sh - 2;
	Gain_slider->setBounds(m, y, sw, sh); y += sh - 2;
	Bias_slider->setBounds(m, y, sw, sh); y += sh - 2;
	hpFilterOut_slider->setBounds(m, y, sw, sh); y += sh - 2;
	OutputClipLevel_slider->setBounds(m, y, sw, sh); y += sh - 2;
	Osc1Amp_slider->setBounds(m, y, sw, sh); y += sh - 2;
	Osc2Amp_slider->setBounds(m, y, sw, sh); y += dy;

	Octave_slider->setBounds(m, y, sw, sh); y += sh - 2;
	Tune_slider->setBounds(m, y, sw, sh); y += sh - 2;
	Glide_slider->setBounds(m, y, sw, sh); y += dy;

	InMix_slider->setBounds(m, y, sw, sh); y += sh - 2;
	InDetune_slider->setBounds(m, y, sw, sh); y += dy;

	Ratio_slider->setBounds(m, y, sw, sh); y += sh - 2;
	ModDepth_slider->setBounds(m, y, sw, sh); y += sh - 2;
	ModEnvDepth_slider->setBounds(m, y, sw, sh); y += dy;

	Osc_1_Bypass_button->setBounds(m, y, sw, sh); y += sh - 2;
	Mod11_slider->setBounds(m, y, sw, sh); y += sh - 2;
	//FmVsPm11->setBounds(m, y, sw, sh); y += sh-2;
	Mod12_slider->setBounds(m, y, sw, sh); y += sh - 2;
	FmVsPm12_slider->setBounds(m, y, sw, sh); y += dy;

	Osc_2_Bypass_button->setBounds(m, y, sw, sh); y += sh - 2;
	Mod22_slider->setBounds(m, y, sw, sh); y += sh - 2;
	//FmVsPm22->setBounds(m, y, sw, sh); y += sh-2;
	Mod21_slider->setBounds(m, y, sw, sh); y += sh - 2;
	FmVsPm21_slider->setBounds(m, y, sw, sh); y += dy;

	Osc1FreqMin_slider->setBounds(m, y, sw, sh); y += sh - 2;
	Osc1FreqMax_slider->setBounds(m, y, sw, sh); y += sh - 2;
	Osc2FreqMin_slider->setBounds(m, y, sw, sh); y += sh - 2;
	Osc2FreqMax_slider->setBounds(m, y, sw, sh); y += dy;

	osc1ResetMode_combobox->setBounds(m, y, sw, sh); y += sh - 2;
	osc2ResetMode_combobox->setBounds(m, y, sw, sh); y += dy;

	ampEnvResetMode_combobox->setBounds(m, y, sw, sh); y += sh - 2;
	modEnvResetMode_combobox->setBounds(m, y, sw, sh); y += sh - 2;
	ampEnvTrigMode_combobox->setBounds(m, y, sw, sh); y += sh - 2;
	modEnvTrigMode_combobox->setBounds(m, y, sw, sh); y += dy;

	glideMode_combobox->setBounds(m, y, sw, sh); y += dy;

	// middle column:
	x = w3;
	y = pageButtonGlobal->getBottom() + 4;

	FMD_Freq_slider->setBounds(x + m, y, sw, sh); y += sh - 2;
	FMD_Res_slider->setBounds(x + m, y, sw, sh); y += sh - 2;
	FMD_Chaos_slider->setBounds(x + m, y, sw, sh); y += sh - 2;
	FMD_Input_Clip_slider->setBounds(x + m, y, sw, sh); y += sh - 2;
	OscInAmplitude_slider->setBounds(x + m, y, sw, sh); y += sh - 2;
	FMD_Overdrive_slider->setBounds(x + m, y, sw, sh); y += sh - 2;
	FMD_Out_slider->setBounds(x + m, y, sw, sh); y += sh - 2;
	FMD_Out_Adjust_slider->setBounds(x + m, y, sw, sh); y += dy;

	OscInOctave_slider->setBounds(x + m, y, sw, sh); y += sh - 2;
	OscInDetune_slider->setBounds(x + m, y, sw, sh); y += sh - 2;
	OscInWave_combobox->setBounds(x + m, y, sw, sh); y += dy;

	Osc1FreqScale_slider->setBounds(x + m, y, sw, sh); y += sh - 2;
	Osc1FreqOffset_slider->setBounds(x + m, y, sw, sh); y += sh - 2;
	Osc1Phase_slider->setBounds(x + m, y, sw, sh); y += sh - 2;
	Osc1ClipLevel_slider->setBounds(x + m, y, sw, sh); y += sh - 2;
	Osc1ClipCenter_slider->setBounds(x + m, y, sw, sh); y += sh - 2;
	Osc1Wave_combobox->setBounds(x + m, y, sw, sh); y += dy;

	Osc1EllipseA_slider->setBounds(x + m, y, sw, sh); y += sh - 2;
	Osc1EllipseB_slider->setBounds(x + m, y, sw, sh); y += sh - 2;
	Osc1EllipseC_slider->setBounds(x + m, y, sw, sh); y += dy;

	Osc2FreqScale_slider->setBounds(x + m, y, sw, sh); y += sh - 2;
	Osc2FreqOffset_slider->setBounds(x + m, y, sw, sh); y += sh - 2;
	Osc2Phase_slider->setBounds(x + m, y, sw, sh); y += sh - 2;
	Osc2ClipLevel_slider->setBounds(x + m, y, sw, sh); y += sh - 2;
	Osc2ClipCenter_slider->setBounds(x + m, y, sw, sh); y += sh - 2;
	Osc2Wave_combobox->setBounds(x + m, y, sw, sh); y += dy;

	Osc2EllipseA_slider->setBounds(x + m, y, sw, sh); y += sh - 2;
	Osc2EllipseB_slider->setBounds(x + m, y, sw, sh); y += sh - 2;
	Osc2EllipseC_slider->setBounds(x + m, y, sw, sh); y += dy;

	SyncThresh_slider->setBounds(x + m, y, sw, sh); y += sh - 2;
	SyncAmount_slider->setBounds(x + m, y, sw, sh); y += sh - 2;
	SyncMode_combobox->setBounds(x + m, y, sw, sh); y += sh - 2;
	SyncFilters_button->setBounds(x + m, y, sw, sh); y += dy;

	// right column:
	x = 2 * w3;
	y = pageButtonGlobal->getBottom() + 4;

	Feedback_Gain_slider->setBounds(x + m, y, sw, sh); y += sh - 2;
	Output_Osc1_Pre_Filter_button->setBounds(x + m, y, sw, sh); y += sh - 2;
	Modulate_With_Osc1_Pre_Filter_button->setBounds(x + m, y, sw, sh); y += dy;

	Filter1CutoffScale_slider->setBounds(x + m, y, sw, sh); y += sh - 2;
	Filter1Resonance_slider->setBounds(x + m, y, sw, sh); y += sh - 2;
	Filter1Mode_combobox->setBounds(x + m, y, sw, sh); y += dy;

	Filter2CutoffScale_slider->setBounds(x + m, y, sw, sh); y += sh - 2;
	Filter2Resonance_slider->setBounds(x + m, y, sw, sh); y += sh - 2;
	Filter2Mode_combobox->setBounds(x + m, y, sw, sh); y += dy;

	InputToFeedback_slider->setBounds(x + m, y, sw, sh); y += sh - 2;
	Filter3Detune_slider->setBounds(x + m, y, sw, sh); y += sh - 2;
	Filter3Gain_slider->setBounds(x + m, y, sw, sh); y += sh - 2;
	Filter3Width_slider->setBounds(x + m, y, sw, sh); y += dy;

	Noise_Amp_slider->setBounds(x + m, y, sw, sh); y += sh - 2;
	Noise_LPF_slider->setBounds(x + m, y, sw, sh); y += sh - 2;
	Noise_HPF_slider->setBounds(x + m, y, sw, sh); y += dy;

	Attack_slider->setBounds(x + m, y, sw, sh); y += sh - 2;
	Release_slider->setBounds(x + m, y, sw, sh); y += sh - 2;
	LookAhead_slider->setBounds(x + m, y, sw, sh); y += sh - 2;
	InLevel_slider->setBounds(x + m, y, sw, sh); y += sh - 2;
	OutLevel_slider->setBounds(x + m, y, sw, sh); y += sh - 2;
	Threshold_slider->setBounds(x + m, y, sw, sh); y += sh - 2;
	CompRatio_slider->setBounds(x + m, y, sw, sh); y += sh - 2;
	KneeWidth_slider->setBounds(x + m, y, sw, sh); y += sh - 2;
	Limit_button->setBounds(x + m, y, sw, sh); y += sh - 2;
	AutoGain_button->setBounds(x + m, y, sw, sh); y += sh - 2;
	Dry_Wet_slider->setBounds(x + m, y, sw, sh); y += dy;

	Bypass_Ducking_button->setBounds(x + m, y, sw, sh); y += sh - 2;
	DuckRange_slider->setBounds(x + m, y, sw, sh); y += sh - 2;
	DuckCenter_slider->setBounds(x + m, y, sw, sh); y += sh - 2;
	DuckFlatness_slider->setBounds(x + m, y, sw, sh); y += sh - 2;
	DuckFill_slider->setBounds(x + m, y, sw, sh); y += sh - 2;
	DuckShape_combobox->setBounds(x + m, y, sw, sh); y += dy;
}
