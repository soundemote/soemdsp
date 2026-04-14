#pragma once

#include <map>

class PrettyScopeEditor;

/** Section of widgets for the PrettyScopeEditor. */

enum brush { line1, dot1, line2, dot2 };

class JUCE_API PrettyScopeRightWidgetSection : public BasicEditor, public jura::RComboBoxObserver
{
public:
  PrettyScopeRightWidgetSection(PrettyScopeModule *prettyScopeToEdit, OpenGLCanvas *displayToUse);
	~PrettyScopeRightWidgetSection() = default;

  void createWidgets();

  void resized() override;
  void rButtonClicked(jura::RButton* button) override;
	void rComboBoxChanged(jura::RComboBox* comboBoxThatHasChanged) override;

  // desired size:
  int desiredWidth = 200;
  int desiredHeight = 750;

protected:

	jura::RClickButton* buttonScreenShot;
	jura::RClickButton* buttonClearCanvas;
	jura::RClickButton* updateShaders_button;

	AutomatableClickButton* clear_button;
	ElanModulatableButton* fxMode_button;
	ElanModulatableSlider* parameterSmoothing_slider;
	ElanModulatableButton* pause_button;
	ElanModulatableComboBox* autoPauseMode_menu;
	ElanModulatableButton* freezePoints_button;
	ElanModulatableButton* showFPS_button;
	ElanModulatableButton* heatmap_button;
	ElanModulatableComboBox* blendMode_menu;
	ElanModulatableButton* framerate_button;
	ElanModulatableSlider* decaySpeed_slider;
	ElanModulatableSlider* pointsPerFrame_slider;
	ElanModulatableSlider* tailFade_slider;

	ElanModulatableButton* dots_button;
	ElanModulatableComboBox* dotColorMode_menu;
	ElanModulatableSlider* dotBrightness_slider;
	ElanModulatableSlider* dotSize_slider;
	ElanModulatableSlider* dotBlur_slider;
	ElanModulatableComboBox* dotProfile_menu;
	ElanModulatableButton* drawOrder_button;

	ElanModulatableButton* lines_button;
	ElanModulatableComboBox* lineColorMode_menu;
	ElanModulatableSlider* lineBrightness_slider;
	ElanModulatableSlider* lineSize_slider;
	ElanModulatableSlider* lineBlur_slider;
	ElanModulatableSlider* lenFactor_slider;
	ElanModulatableSlider* lenColorIntensity_slider;
	ElanModulatableComboBox* lineProfile_menu;

	ElanModulatableSlider* dotColorR_slider;
	ElanModulatableSlider* dotColorG_slider;
	ElanModulatableSlider* dotColorB_slider;
	ElanModulatableSlider* dotColorH_slider;
	ElanModulatableSlider* dotColorS_slider;
	ElanModulatableSlider* dotColorL_slider;

	ElanModulatableSlider* lineColorR_slider;
	ElanModulatableSlider* lineColorG_slider;
	ElanModulatableSlider* lineColorB_slider;
	ElanModulatableSlider* lineColorH_slider;
	ElanModulatableSlider* lineColorS_slider;
	ElanModulatableSlider* lineColorL_slider;

	ElanModulatableSlider* dotColorR2_slider;
	ElanModulatableSlider* dotColorG2_slider;
	ElanModulatableSlider* dotColorB2_slider;
	ElanModulatableSlider* dotColorH2_slider;
	ElanModulatableSlider* dotColorS2_slider;
	ElanModulatableSlider* dotColorL2_slider;

	ElanModulatableSlider* lineColorR2_slider;
	ElanModulatableSlider* lineColorG2_slider;
	ElanModulatableSlider* lineColorB2_slider;
	ElanModulatableSlider* lineColorH2_slider;
	ElanModulatableSlider* lineColorS2_slider;
	ElanModulatableSlider* lineColorL2_slider;

	ElanModulatableSlider* zoom_slider;
	ElanModulatableSlider* rotationX_slider;
	ElanModulatableSlider* rotationY_slider;
	ElanModulatableSlider* rotationZ_slider;
	ElanModulatableSlider* scaleX_slider;
	ElanModulatableSlider* scaleY_slider;
	ElanModulatableSlider* shiftX_slider;
	ElanModulatableSlider* shiftY_slider;
	ElanModulatableSlider* oneDimZoom_slider;
	ElanModulatableSlider* oneDimOffset_slider;
	ElanModulatableButton* oneDimensional_button;
	ElanModulatableSlider* scanFrequency_slider;
	ElanModulatableComboBox* syncMode_menu;
	ElanModulatableSlider* syncNumCycles_slider;

  jura::ColorMapLoader * colorMapLoader;	
  PrettyScopeModule * prettyScopeModule; // pointer to the PrettyScope object to be edited
  OpenGLCanvas * display; // pointer to the display (for screenshots)
};

class JUCE_API PrettyScopeEditor
	: public BasicEditorWithLicensing
	, PluginFileManager::FileMemory
{
public:

  PrettyScopeEditor(PrettyScopeModule *newPhaseScopeToEdit);

  void resized() override;
  void paint(Graphics& g) override;

	PrettyScopeModule *prettyScopeModule;
	OpenGLCanvas openGLCanvas;

	bool userClickedClear = false;

  PrettyScopeRightWidgetSection rightWidgetSection;
  jura::ComponentScrollContainer rightWidgetScroller;

protected:

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PrettyScopeEditor);

};
