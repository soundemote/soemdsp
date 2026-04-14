#include "PrettyScopeEditor.h"

#include <map>
#include <vector>

using namespace se;
using namespace jura;
using namespace rosic;
using namespace juce;

PrettyScopeRightWidgetSection::PrettyScopeRightWidgetSection(PrettyScopeModule *prettyScopeToEdit, OpenGLCanvas *displayToUse) 
  : BasicEditor(prettyScopeToEdit)

	, prettyScopeModule(prettyScopeToEdit)
	, display(displayToUse)
{
	ScopedLock scopedLock(*lock);  
  
  setHeadlineStyle(NO_HEADLINE);
	setWidgetAppearance(jura::ColourScheme::DARK_ON_BRIGHT);
  createWidgets();

  setSize(desiredWidth, desiredHeight);
}

void PrettyScopeRightWidgetSection::createWidgets()
{
	BasicEditor::createWidgets();

	auto module = prettyScopeModule;

	addWidget(updateShaders_button = new RClickButton("Update Shaders"));
	updateShaders_button->addRButtonListener(this);

	addWidget(buttonScreenShot = new RClickButton("ScrnShot"));
	buttonScreenShot->addRButtonListener(this);	

	addAndMakeVisible(clear_button = new AutomatableClickButton("Clear"));
	clear_button->assignParameter(module->getParameterByName("Clear"));
	//clear_button->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	//clear_button->ParameterInfo::InfoSender::name = "clear";
	//clear_button->ParameterInfo::InfoSender::description = "";

	addAndMakeVisible(fxMode_button = new ElanModulatableButton("Audio Effects On/Off"));
	fxMode_button->assignParameter(module->getParameterByName("FXMode"));
	//fxMode_button->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	fxMode_button->ParameterInfo::InfoSender::name = "FXMode";
	fxMode_button->ParameterInfo::InfoSender::description = "";

	addAndMakeVisible(parameterSmoothing_slider = new ElanModulatableSlider);
	parameterSmoothing_slider->assignParameter(module->getParameterByName("ParameterSmoothing"));
	//parameterSmoothing_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	parameterSmoothing_slider->ParameterInfo::InfoSender::name = "ParameterSmoothing";
	parameterSmoothing_slider->ParameterInfo::InfoSender::description = "";

	addAndMakeVisible(pause_button = new ElanModulatableButton("Pause"));
	pause_button->assignParameter(module->getParameterByName("Pause"));
	//pause_button->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	pause_button->ParameterInfo::InfoSender::name = "Pause";
	pause_button->ParameterInfo::InfoSender::description = "";

	addAndMakeVisible(autoPauseMode_menu = new ElanModulatableComboBox);
	autoPauseMode_menu->assignParameter(module->getParameterByName("AutoPauseMode"));
	//autoPauseMode_menu->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	autoPauseMode_menu->ParameterInfo::InfoSender::name = "AutoPauseMode";
	autoPauseMode_menu->ParameterInfo::InfoSender::description = "";

	addAndMakeVisible(freezePoints_button = new ElanModulatableButton("FreezePts"));
	freezePoints_button->assignParameter(module->getParameterByName("FreezePoints"));
	//freezePoints_button->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	freezePoints_button->ParameterInfo::InfoSender::name = "FreezePoints";
	freezePoints_button->ParameterInfo::InfoSender::description = "";

	addAndMakeVisible(showFPS_button = new ElanModulatableButton("FPS"));
	showFPS_button->assignParameter(module->getParameterByName("ShowFPS"));
	//showFPS_button->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	showFPS_button->ParameterInfo::InfoSender::name = "ShowFPS";
	showFPS_button->ParameterInfo::InfoSender::description = "";

	addAndMakeVisible(heatmap_button = new ElanModulatableButton("Heatmap"));
	heatmap_button->assignParameter(module->getParameterByName("Heatmap"));
	//heatmap_button->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	heatmap_button->ParameterInfo::InfoSender::name = "Heatmap";
	heatmap_button->ParameterInfo::InfoSender::description = "";

	addAndMakeVisible(blendMode_menu = new ElanModulatableComboBox);
	blendMode_menu->assignParameter(module->getParameterByName("BlendMode"));
	//blendMode_menu->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	blendMode_menu->ParameterInfo::InfoSender::name = "BlendMode";
	blendMode_menu->ParameterInfo::InfoSender::description = "";

	addAndMakeVisible(framerate_button = new ElanModulatableButton("VSync On/Off"));
	framerate_button->assignParameter(module->getParameterByName("Framerate"));
	//framerate_button->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	framerate_button->ParameterInfo::InfoSender::name = "Framerate";
	framerate_button->ParameterInfo::InfoSender::description = "";

	addAndMakeVisible(decaySpeed_slider = new ElanModulatableSlider);
	decaySpeed_slider->assignParameter(module->getParameterByName("DecaySpeed"));
	//decaySpeed_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	decaySpeed_slider->ParameterInfo::InfoSender::name = "DecaySpeed";
	decaySpeed_slider->ParameterInfo::InfoSender::description = "";

	addAndMakeVisible(pointsPerFrame_slider = new ElanModulatableSlider);
	pointsPerFrame_slider->assignParameter(module->getParameterByName("PointsPerFrame"));
	//pointsPerFrame_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	pointsPerFrame_slider->ParameterInfo::InfoSender::name = "PointsPerFrame";
	pointsPerFrame_slider->ParameterInfo::InfoSender::description = "";

	addAndMakeVisible(tailFade_slider = new ElanModulatableSlider);
	tailFade_slider->assignParameter(module->getParameterByName("TailFade"));
	//tailFade_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	tailFade_slider->ParameterInfo::InfoSender::name = "TailFade";
	tailFade_slider->ParameterInfo::InfoSender::description = "";

	addAndMakeVisible(dots_button = new ElanModulatableButton("Dots"));
	dots_button->assignParameter(module->getParameterByName("Dots"));
	//dots_button->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	dots_button->ParameterInfo::InfoSender::name = "Dots";
	dots_button->ParameterInfo::InfoSender::description = "";

	addAndMakeVisible(dotColorMode_menu = new ElanModulatableComboBox);
	dotColorMode_menu->assignParameter(module->getParameterByName("DotColorMode"));
	//dotColorMode_menu->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	dotColorMode_menu->ParameterInfo::InfoSender::name = "DotColorMode";
	dotColorMode_menu->ParameterInfo::InfoSender::description = "";

	addAndMakeVisible(dotBrightness_slider = new ElanModulatableSlider);
	dotBrightness_slider->assignParameter(module->getParameterByName("DotBrightness"));
	//dotBrightness_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	dotBrightness_slider->ParameterInfo::InfoSender::name = "DotBrightness";
	dotBrightness_slider->ParameterInfo::InfoSender::description = "";

	addAndMakeVisible(dotSize_slider = new ElanModulatableSlider);
	dotSize_slider->assignParameter(module->getParameterByName("DotSize"));
	//dotSize_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	dotSize_slider->ParameterInfo::InfoSender::name = "DotSize";
	dotSize_slider->ParameterInfo::InfoSender::description = "";

	addAndMakeVisible(dotBlur_slider = new ElanModulatableSlider);
	dotBlur_slider->assignParameter(module->getParameterByName("DotBlur"));
	//dotBlur_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	dotBlur_slider->ParameterInfo::InfoSender::name = "DotBlur";
	dotBlur_slider->ParameterInfo::InfoSender::description = "";

	addAndMakeVisible(dotProfile_menu = new ElanModulatableComboBox);
	dotProfile_menu->assignParameter(module->getParameterByName("DotProfile"));
	//dotProfile_menu->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	dotProfile_menu->ParameterInfo::InfoSender::name = "DotProfile";
	dotProfile_menu->ParameterInfo::InfoSender::description = "";

	addAndMakeVisible(drawOrder_button = new ElanModulatableButton("LinesOnTop"));
	drawOrder_button->assignParameter(module->getParameterByName("DrawOrder"));
	//drawOrder_button->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	drawOrder_button->ParameterInfo::InfoSender::name = "DrawOrder";
	drawOrder_button->ParameterInfo::InfoSender::description = "";

	addAndMakeVisible(lines_button = new ElanModulatableButton("Lines"));
	lines_button->assignParameter(module->getParameterByName("Lines"));
	//lines_button->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	lines_button->ParameterInfo::InfoSender::name = "Lines";
	lines_button->ParameterInfo::InfoSender::description = "";

	addAndMakeVisible(lineColorMode_menu = new ElanModulatableComboBox);
	lineColorMode_menu->assignParameter(module->getParameterByName("LineColorMode"));
	//lineColorMode_menu->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	lineColorMode_menu->ParameterInfo::InfoSender::name = "LineColorMode";
	lineColorMode_menu->ParameterInfo::InfoSender::description = "";

	addAndMakeVisible(lineBrightness_slider = new ElanModulatableSlider);
	lineBrightness_slider->assignParameter(module->getParameterByName("LineBrightness"));
	//lineBrightness_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	lineBrightness_slider->ParameterInfo::InfoSender::name = "LineBrightness";
	lineBrightness_slider->ParameterInfo::InfoSender::description = "";

	addAndMakeVisible(lineSize_slider = new ElanModulatableSlider);
	lineSize_slider->assignParameter(module->getParameterByName("LineSize"));
	//lineSize_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	lineSize_slider->ParameterInfo::InfoSender::name = "LineSize";
	lineSize_slider->ParameterInfo::InfoSender::description = "";

	addAndMakeVisible(lineBlur_slider = new ElanModulatableSlider);
	lineBlur_slider->assignParameter(module->getParameterByName("LineBlur"));
	//lineBlur_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	lineBlur_slider->ParameterInfo::InfoSender::name = "LineBlur";
	lineBlur_slider->ParameterInfo::InfoSender::description = "";

	addAndMakeVisible(lenFactor_slider = new ElanModulatableSlider);
	lenFactor_slider->assignParameter(module->getParameterByName("LenFactor"));
	//lenFactor_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	lenFactor_slider->ParameterInfo::InfoSender::name = "LenFactor";
	lenFactor_slider->ParameterInfo::InfoSender::description = "";

	addAndMakeVisible(lineProfile_menu = new ElanModulatableComboBox);
	lineProfile_menu->assignParameter(module->getParameterByName("LineProfile"));
	//lineProfile_menu->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	lineProfile_menu->ParameterInfo::InfoSender::name = "LineProfile";
	lineProfile_menu->ParameterInfo::InfoSender::description = "";

	addAndMakeVisible(dotColorR_slider = new ElanModulatableSlider);
	dotColorR_slider->assignParameter(module->getParameterByName("DotColorR"));
	dotColorR_slider->setSliderName("Red");
	//dotColorR_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	dotColorR_slider->ParameterInfo::InfoSender::name = "DotColorR";
	dotColorR_slider->ParameterInfo::InfoSender::description = "";

	addAndMakeVisible(dotColorG_slider = new ElanModulatableSlider);
	dotColorG_slider->assignParameter(module->getParameterByName("DotColorG"));
	dotColorG_slider->setSliderName("Grn");
	//dotColorG_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	dotColorG_slider->ParameterInfo::InfoSender::name = "DotColorG";
	dotColorG_slider->ParameterInfo::InfoSender::description = "";

	addAndMakeVisible(dotColorB_slider = new ElanModulatableSlider);
	dotColorB_slider->assignParameter(module->getParameterByName("DotColorB"));
	dotColorB_slider->setSliderName("Blu");
	//dotColorB_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	dotColorB_slider->ParameterInfo::InfoSender::name = "DotColorB";
	dotColorB_slider->ParameterInfo::InfoSender::description = "";

	addAndMakeVisible(dotColorH_slider = new ElanModulatableSlider);
	dotColorH_slider->assignParameter(module->getParameterByName("DotColorH"));
	dotColorH_slider->setSliderName("Hue");
	//dotColorH_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	dotColorH_slider->ParameterInfo::InfoSender::name = "DotColorH";
	dotColorH_slider->ParameterInfo::InfoSender::description = "";

	addAndMakeVisible(dotColorS_slider = new ElanModulatableSlider);
	dotColorS_slider->assignParameter(module->getParameterByName("DotColorS"));
	dotColorS_slider->setSliderName("Sat");
	//dotColorS_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	dotColorS_slider->ParameterInfo::InfoSender::name = "DotColorS";
	dotColorS_slider->ParameterInfo::InfoSender::description = "";

	addAndMakeVisible(dotColorL_slider = new ElanModulatableSlider);
	dotColorL_slider->assignParameter(module->getParameterByName("DotColorL"));
	dotColorL_slider->setSliderName("Lit");
	//dotColorL_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	dotColorL_slider->ParameterInfo::InfoSender::name = "DotColorL";
	dotColorL_slider->ParameterInfo::InfoSender::description = "";

	addAndMakeVisible(lineColorR_slider = new ElanModulatableSlider);
	lineColorR_slider->assignParameter(module->getParameterByName("LineColorR"));
	lineColorR_slider->setSliderName("Red");
	//lineColorR_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	lineColorR_slider->ParameterInfo::InfoSender::name = "LineColorR";
	lineColorR_slider->ParameterInfo::InfoSender::description = "";

	addAndMakeVisible(lineColorG_slider = new ElanModulatableSlider);
	lineColorG_slider->assignParameter(module->getParameterByName("LineColorG"));
	lineColorG_slider->setSliderName("Grn");
	//lineColorG_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	lineColorG_slider->ParameterInfo::InfoSender::name = "LineColorG";
	lineColorG_slider->ParameterInfo::InfoSender::description = "";

	addAndMakeVisible(lineColorB_slider = new ElanModulatableSlider);
	lineColorB_slider->assignParameter(module->getParameterByName("LineColorB"));
	lineColorB_slider->setSliderName("Blu");
	//lineColorB_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	lineColorB_slider->ParameterInfo::InfoSender::name = "LineColorB";
	lineColorB_slider->ParameterInfo::InfoSender::description = "";

	addAndMakeVisible(lineColorH_slider = new ElanModulatableSlider);
	lineColorH_slider->assignParameter(module->getParameterByName("LineColorH"));
	lineColorH_slider->setSliderName("Hue");
	//lineColorH_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	lineColorH_slider->ParameterInfo::InfoSender::name = "LineColorH";
	lineColorH_slider->ParameterInfo::InfoSender::description = "";

	addAndMakeVisible(lineColorS_slider = new ElanModulatableSlider);
	lineColorS_slider->assignParameter(module->getParameterByName("LineColorS"));
	lineColorS_slider->setSliderName("Sat");
	//lineColorS_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	lineColorS_slider->ParameterInfo::InfoSender::name = "LineColorS";
	lineColorS_slider->ParameterInfo::InfoSender::description = "";

	addAndMakeVisible(lineColorL_slider = new ElanModulatableSlider);
	lineColorL_slider->assignParameter(module->getParameterByName("LineColorL"));
	lineColorL_slider->setSliderName("Lit");
	//lineColorL_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	lineColorL_slider->ParameterInfo::InfoSender::name = "LineColorL";
	lineColorL_slider->ParameterInfo::InfoSender::description = "";

	addAndMakeVisible(dotColorR2_slider = new ElanModulatableSlider);
	dotColorR2_slider->assignParameter(module->getParameterByName("DotColorR2"));
	dotColorR2_slider->setSliderName("Red");
	//dotColorR2_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	dotColorR2_slider->ParameterInfo::InfoSender::name = "DotColorR2";
	dotColorR2_slider->ParameterInfo::InfoSender::description = "";

	addAndMakeVisible(dotColorG2_slider = new ElanModulatableSlider);
	dotColorG2_slider->assignParameter(module->getParameterByName("DotColorG2"));
	dotColorG2_slider->setSliderName("Grn");
	//dotColorG2_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	dotColorG2_slider->ParameterInfo::InfoSender::name = "DotColorG2";
	dotColorG2_slider->ParameterInfo::InfoSender::description = "";

	addAndMakeVisible(dotColorB2_slider = new ElanModulatableSlider);
	dotColorB2_slider->assignParameter(module->getParameterByName("DotColorB2"));
	dotColorB2_slider->setSliderName("Blu");
	//dotColorB2_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	dotColorB2_slider->ParameterInfo::InfoSender::name = "DotColorB2";
	dotColorB2_slider->ParameterInfo::InfoSender::description = "";

	addAndMakeVisible(dotColorH2_slider = new ElanModulatableSlider);
	dotColorH2_slider->assignParameter(module->getParameterByName("DotColorH2"));
	dotColorH2_slider->setSliderName("Hue");
	//dotColorH2_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	dotColorH2_slider->ParameterInfo::InfoSender::name = "DotColorH2";
	dotColorH2_slider->ParameterInfo::InfoSender::description = "";

	addAndMakeVisible(dotColorS2_slider = new ElanModulatableSlider);
	dotColorS2_slider->assignParameter(module->getParameterByName("DotColorS2"));
	dotColorS2_slider->setSliderName("Sat");
	//dotColorS2_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	dotColorS2_slider->ParameterInfo::InfoSender::name = "DotColorS2";
	dotColorS2_slider->ParameterInfo::InfoSender::description = "";

	addAndMakeVisible(dotColorL2_slider = new ElanModulatableSlider);
	dotColorL2_slider->assignParameter(module->getParameterByName("DotColorL2"));
	dotColorL2_slider->setSliderName("Lit");
	//dotColorL2_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	dotColorL2_slider->ParameterInfo::InfoSender::name = "DotColorL2";
	dotColorL2_slider->ParameterInfo::InfoSender::description = "";

	addAndMakeVisible(lineColorR2_slider = new ElanModulatableSlider);
	lineColorR2_slider->assignParameter(module->getParameterByName("LineColorR2"));
	lineColorR2_slider->setSliderName("Red");
	//lineColorR2_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	lineColorR2_slider->ParameterInfo::InfoSender::name = "LineColorR2";
	lineColorR2_slider->ParameterInfo::InfoSender::description = "";

	addAndMakeVisible(lineColorG2_slider = new ElanModulatableSlider);
	lineColorG2_slider->assignParameter(module->getParameterByName("LineColorG2"));
	lineColorG2_slider->setSliderName("Grn");
	//lineColorG2_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	lineColorG2_slider->ParameterInfo::InfoSender::name = "LineColorG2";
	lineColorG2_slider->ParameterInfo::InfoSender::description = "";

	addAndMakeVisible(lineColorB2_slider = new ElanModulatableSlider);
	lineColorB2_slider->assignParameter(module->getParameterByName("LineColorB2"));
	lineColorB2_slider->setSliderName("Blu");
	//lineColorB2_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	lineColorB2_slider->ParameterInfo::InfoSender::name = "LineColorB2";
	lineColorB2_slider->ParameterInfo::InfoSender::description = "";

	addAndMakeVisible(lineColorH2_slider = new ElanModulatableSlider);
	lineColorH2_slider->assignParameter(module->getParameterByName("LineColorH2"));
	lineColorH2_slider->setSliderName("Hue");
	//lineColorH2_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	lineColorH2_slider->ParameterInfo::InfoSender::name = "LineColorH2";
	lineColorH2_slider->ParameterInfo::InfoSender::description = "";

	addAndMakeVisible(lineColorS2_slider = new ElanModulatableSlider);
	lineColorS2_slider->assignParameter(module->getParameterByName("LineColorS2"));
	lineColorS2_slider->setSliderName("Sat");
	//lineColorS2_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	lineColorS2_slider->ParameterInfo::InfoSender::name = "LineColorS2";
	lineColorS2_slider->ParameterInfo::InfoSender::description = "";

	addAndMakeVisible(lineColorL2_slider = new ElanModulatableSlider);
	lineColorL2_slider->assignParameter(module->getParameterByName("LineColorL2"));
	lineColorL2_slider->setSliderName("Lit");
	//lineColorL2_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	lineColorL2_slider->ParameterInfo::InfoSender::name = "LineColorL2";
	lineColorL2_slider->ParameterInfo::InfoSender::description = "";

	addAndMakeVisible(lenColorIntensity_slider = new ElanModulatableSlider);
	lenColorIntensity_slider->assignParameter(module->getParameterByName("LenColorIntensity"));
	//lenColorIntensity_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	lenColorIntensity_slider->ParameterInfo::InfoSender::name = "LenColorIntensity";
	lenColorIntensity_slider->ParameterInfo::InfoSender::description = "";

	addAndMakeVisible(zoom_slider = new ElanModulatableSlider);
	zoom_slider->assignParameter(module->getParameterByName("Zoom"));
	//zoom_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	zoom_slider->ParameterInfo::InfoSender::name = "Zoom";
	zoom_slider->ParameterInfo::InfoSender::description = "";

	addAndMakeVisible(rotationX_slider = new ElanModulatableSlider);
	rotationX_slider->assignParameter(module->getParameterByName("RotationX"));
	//rotationX_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	rotationX_slider->ParameterInfo::InfoSender::name = "RotationX";
	rotationX_slider->ParameterInfo::InfoSender::description = "";

	addAndMakeVisible(rotationY_slider = new ElanModulatableSlider);
	rotationY_slider->assignParameter(module->getParameterByName("RotationY"));
	//rotationY_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	rotationY_slider->ParameterInfo::InfoSender::name = "RotationY";
	rotationY_slider->ParameterInfo::InfoSender::description = "";

	addAndMakeVisible(rotationZ_slider = new ElanModulatableSlider);
	rotationZ_slider->assignParameter(module->getParameterByName("RotationZ"));
	//rotationZ_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	rotationZ_slider->ParameterInfo::InfoSender::name = "RotationZ";
	rotationZ_slider->ParameterInfo::InfoSender::description = "";

	addAndMakeVisible(scaleX_slider = new ElanModulatableSlider);
	scaleX_slider->assignParameter(module->getParameterByName("ScaleX"));
	//scaleX_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	scaleX_slider->ParameterInfo::InfoSender::name = "ScaleX";
	scaleX_slider->ParameterInfo::InfoSender::description = "";

	addAndMakeVisible(scaleY_slider = new ElanModulatableSlider);
	scaleY_slider->assignParameter(module->getParameterByName("ScaleY"));
	//scaleY_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	scaleY_slider->ParameterInfo::InfoSender::name = "ScaleY";
	scaleY_slider->ParameterInfo::InfoSender::description = "";

	addAndMakeVisible(shiftX_slider = new ElanModulatableSlider);
	shiftX_slider->assignParameter(module->getParameterByName("ShiftX"));
	//shiftX_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	shiftX_slider->ParameterInfo::InfoSender::name = "ShiftX";
	shiftX_slider->ParameterInfo::InfoSender::description = "";

	addAndMakeVisible(shiftY_slider = new ElanModulatableSlider);
	shiftY_slider->assignParameter(module->getParameterByName("ShiftY"));
	//shiftY_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	shiftY_slider->ParameterInfo::InfoSender::name = "ShiftY";
	shiftY_slider->ParameterInfo::InfoSender::description = "";

	addAndMakeVisible(oneDimZoom_slider = new ElanModulatableSlider);
	oneDimZoom_slider->assignParameter(module->getParameterByName("OneDimZoom"));
	oneDimZoom_slider->setSliderName("Zoom");
	//oneDimZoom_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	oneDimZoom_slider->ParameterInfo::InfoSender::name = "OneDimZoom";
	oneDimZoom_slider->ParameterInfo::InfoSender::description = "";

	addAndMakeVisible(oneDimOffset_slider = new ElanModulatableSlider);
	oneDimOffset_slider->assignParameter(module->getParameterByName("OneDimOffset"));
	oneDimOffset_slider->setSliderName("Offset");
	//oneDimOffset_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	oneDimOffset_slider->ParameterInfo::InfoSender::name = "OneDimOffset";
	oneDimOffset_slider->ParameterInfo::InfoSender::description = "";

	addAndMakeVisible(oneDimensional_button = new ElanModulatableButton("1D Mode On/Off"));
	oneDimensional_button->assignParameter(module->getParameterByName("OneDimensional"));
	//oneDimensional_button->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	oneDimensional_button->ParameterInfo::InfoSender::name = "OneDimensional";
	oneDimensional_button->ParameterInfo::InfoSender::description = "";

	addAndMakeVisible(scanFrequency_slider = new ElanModulatableSlider);
	scanFrequency_slider->assignParameter(module->getParameterByName("ScanFrequency"));
	//scanFrequency_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	scanFrequency_slider->ParameterInfo::InfoSender::name = "ScanFrequency";
	scanFrequency_slider->ParameterInfo::InfoSender::description = "";

	addAndMakeVisible(syncMode_menu = new ElanModulatableComboBox);
	syncMode_menu->assignParameter(module->getParameterByName("SyncMode"));
	//syncMode_menu->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	syncMode_menu->ParameterInfo::InfoSender::name = "SyncMode";
	syncMode_menu->ParameterInfo::InfoSender::description = "";

	addAndMakeVisible(syncNumCycles_slider = new ElanModulatableSlider);
	syncNumCycles_slider->assignParameter(module->getParameterByName("SyncNumCycles"));
	syncNumCycles_slider->setSliderName("# Cycles");
	//syncNumCycles_slider->ParameterInfo::InfoSender::setParameterInfoReceiver(&parameterInfo);
	syncNumCycles_slider->ParameterInfo::InfoSender::name = "SyncNumCycles";
	syncNumCycles_slider->ParameterInfo::InfoSender::description = "";

	clear_button->addRButtonListener(this);

  colorMapLoader = new ColorMapLoader(prettyScopeModule->getColorMapPointer());
  addWidgetSet(colorMapLoader);

	syncMode_menu->registerComboBoxObserver(this);
	lineColorMode_menu->registerComboBoxObserver(this);
	dotColorMode_menu->registerComboBoxObserver(this);
}

void PrettyScopeRightWidgetSection::resized()
{
	int x = 4;
	int y = 4;
	int w = getWidth() - x - 8; // slider width
	int h = 16;                 // slider height
	int dy = h - 2;               // vertical distance ("delta-y") between widgets
	int w2 = (w - 8) / 2;
	int w3 = (w - 8) / 3;
	int xm0 = x;
	int xm1 = x + w2;

	// settings
	updateShaders_button->setBounds(x, y, w, h); y += dy; y += 8;
	pause_button->setBounds(xm0, y, w2 - 8, h); freezePoints_button->setBounds(xm1 + 8, y, w2, h); y += dy;
	autoPauseMode_menu->setBounds(x, y, w, h); y += dy;
	clear_button->setBounds(x, y, w / 4, h);
	showFPS_button->setBounds(x + w / 4, y, w / 4, h);
	buttonScreenShot->setBounds(x + w / 2, y, w / 2, h);  y += dy;
	y += 8;
	heatmap_button->setBounds(x, y, w, h); y += dy;
	y += 8;
	blendMode_menu->setBounds(x, y, w, h);  y += dy;
	y += 8;
	// general scope controls:
	framerate_button->setBounds(x, y, w, h);  y += dy;
	decaySpeed_slider->setBounds(x, y, w, h);  y += dy;
	pointsPerFrame_slider->setBounds(x, y, w, h);  y += dy;
	tailFade_slider->setBounds(x, y, w, h);  y += dy;

	bool showLineLen = lineColorMode_menu->getAssignedParameter()->getValue() >= 2.0;

	bool showDotRGB = dotColorMode_menu->getAssignedParameter()->getValue() == 0.0;
	bool showDotHSL = !showDotRGB;
	bool showLineRGB = !showLineLen && lineColorMode_menu->getAssignedParameter()->getValue() == 0.0;
	bool showLineHSL = !showLineLen && !showLineRGB;

	// dot controls:
	y += 8;
	dots_button->setBounds(x, y, w3, h); dotColorMode_menu->setBounds(x + w3 + 4, y, w - w3 - 4, h);  y += dy;
	dotBrightness_slider->setBounds(x, y, w, h); y += dy;
	dotSize_slider->setBounds(x, y, w, h); y += dy;
	dotBlur_slider->setBounds(x, y, w, h); y += dy;
	dotProfile_menu->setBounds(x, y, w, h); y += dy;
	y += 8;
	dotColorR_slider->setBounds(x, y, w / 2 - 1, h); dotColorR_slider->setVisible(showDotRGB);
	dotColorH_slider->setBounds(x, y, w / 2 - 1, h); dotColorH_slider->setVisible(showDotHSL);
	dotColorR2_slider->setBounds(x + w / 2 + 1, y, w / 2 - 1, h); dotColorR2_slider->setVisible(showDotRGB);
	dotColorH2_slider->setBounds(x + w / 2 + 1, y, w / 2 - 1, h); dotColorH2_slider->setVisible(showDotHSL);
	y += dy;

	dotColorG_slider->setBounds(x, y, w / 2 - 1, h); dotColorG_slider->setVisible(showDotRGB);
	dotColorS_slider->setBounds(x, y, w / 2 - 1, h); dotColorS_slider->setVisible(showDotHSL);
	dotColorG2_slider->setBounds(x + w / 2 + 1, y, w / 2 - 1, h); dotColorG2_slider->setVisible(showDotRGB);
	dotColorS2_slider->setBounds(x + w / 2 + 1, y, w / 2 - 1, h); dotColorS2_slider->setVisible(showDotHSL);
	y += dy;

	dotColorB_slider->setBounds(x, y, w / 2 - 1, h); dotColorB_slider->setVisible(showDotRGB);
	dotColorL_slider->setBounds(x, y, w / 2 - 1, h); dotColorL_slider->setVisible(showDotHSL);
	dotColorB2_slider->setBounds(x + w / 2 + 1, y, w / 2 - 1, h); dotColorB2_slider->setVisible(showDotRGB);
	dotColorL2_slider->setBounds(x + w / 2 + 1, y, w / 2 - 1, h); dotColorL2_slider->setVisible(showDotHSL);
	y += dy;

	//other
	y += 8;
	drawOrder_button->setBounds(x, y, w2, h); y += dy;

	// line controls:
	y += 8;
	lines_button->setBounds(x, y, w3, h); lineColorMode_menu->setBounds(x + w3 + 4, y, w - w3 - 4, h); y += dy;
	lineBrightness_slider->setBounds(x, y, w, h); y += dy;
	lineSize_slider->setBounds(x, y, w, h); y += dy;
	lineBlur_slider->setBounds(x, y, w, h); y += dy;
	lenFactor_slider->setBounds(x, y, w, h); y += dy;
	lineProfile_menu->setBounds(x, y, w, h); y += dy;
	y += 8;

	lenColorIntensity_slider->setBounds(x, y, w, h); lenColorIntensity_slider->setVisible(showLineLen);
	lineColorR_slider->setBounds(x, y, w / 2 - 1, h); lineColorR_slider->setVisible(showLineRGB);
	lineColorH_slider->setBounds(x, y, w / 2 - 1, h); lineColorH_slider->setVisible(showLineHSL);
	lineColorR2_slider->setBounds(x + w / 2 + 1, y, w / 2 - 1, h); lineColorR2_slider->setVisible(showLineRGB);
	lineColorH2_slider->setBounds(x + w / 2 + 1, y, w / 2 - 1, h); lineColorH2_slider->setVisible(showLineHSL);
	y += dy;

	lineColorG_slider->setBounds(x, y, w / 2 - 1, h); lineColorG_slider->setVisible(showLineRGB);
	lineColorS_slider->setBounds(x, y, w / 2 - 1, h); lineColorS_slider->setVisible(showLineHSL);
	lineColorG2_slider->setBounds(x + w / 2 + 1, y, w / 2 - 1, h); lineColorG2_slider->setVisible(showLineRGB);
	lineColorS2_slider->setBounds(x + w / 2 + 1, y, w / 2 - 1, h); lineColorS2_slider->setVisible(showLineHSL);
	y += dy;

	lineColorB_slider->setBounds(x, y, w / 2 - 1, h); lineColorB_slider->setVisible(showLineRGB);
	lineColorL_slider->setBounds(x, y, w / 2 - 1, h); lineColorL_slider->setVisible(showLineHSL);
	lineColorB2_slider->setBounds(x + w / 2 + 1, y, w / 2 - 1, h); lineColorB2_slider->setVisible(showLineRGB);
	lineColorL2_slider->setBounds(x + w / 2 + 1, y, w / 2 - 1, h); lineColorL2_slider->setVisible(showLineHSL);
	y += dy;

	// transform controls:
	y += 8;
	fxMode_button->setBounds(x, y, w, h); y += dy;
	parameterSmoothing_slider->setBounds(x, y, w, h); y += dy;
	zoom_slider->setBounds(x, y, w, h); y += dy;
	rotationX_slider->setBounds(x, y, w, h); y += dy;
	rotationY_slider->setBounds(x, y, w, h); y += dy;
	rotationZ_slider->setBounds(x, y, w, h); y += dy;
	scaleX_slider->setBounds(x, y, w, h); y += dy;
	scaleY_slider->setBounds(x, y, w, h); y += dy;
	shiftX_slider->setBounds(x, y, w, h); y += dy;
	shiftY_slider->setBounds(x, y, w, h); y += dy;

	y += 8;
	oneDimensional_button->setBounds(x, y, w, h); y += dy;
	syncMode_menu->setBounds(x, y, w, h); y += dy;

	bool showNumCyclesSlider = syncMode_menu->getAssignedParameter()->getValue() > 0.0;
	scanFrequency_slider->setVisible(!showNumCyclesSlider);
	syncNumCycles_slider->setVisible(showNumCyclesSlider);
	scanFrequency_slider->setBounds(x, y, w, h); syncNumCycles_slider->setBounds(x, y, w, h); y += dy;

	y += 8;
	oneDimZoom_slider->setBounds(x, y, w / 2 - 1, h);
	oneDimOffset_slider->setBounds(x + w / 2 + 1, y, w / 2 - 1, h); y += dy;

	// colormap loader:
	y += 8;
	colorMapLoader->setBounds(x, y, w, 48);
}

void PrettyScopeRightWidgetSection::rButtonClicked(RButton* button)
{
	if (button == buttonScreenShot)
		display->isSavingScreenshot = true;
	else if (button == static_cast<RButton*>(clear_button))
		display->userClickedClearCanvas = true;
	else if (button == static_cast<RButton*>(updateShaders_button))
		display->updateShaders();
  else
    AudioModuleEditor::rButtonClicked(button);
}

void PrettyScopeRightWidgetSection::rComboBoxChanged(jura::RComboBox * comboBoxThatHasChanged)
{
	if (comboBoxThatHasChanged == dotColorMode_menu ||
		comboBoxThatHasChanged ==  lineColorMode_menu ||
		comboBoxThatHasChanged == syncMode_menu)
		resized();
}

PrettyScopeEditor::PrettyScopeEditor(PrettyScopeModule* newPrettyScopeModuleToEdit)
  : BasicEditorWithLicensing(newPrettyScopeModuleToEdit, this, "PrettyScope", "PrettyScope", "https://www.soundemote.com/products/prettyscope", "https://www.soundemote.com/products/prettyscope")
  , openGLCanvas(newPrettyScopeModuleToEdit, &newPrettyScopeModuleToEdit->audioBuffer, &newPrettyScopeModuleToEdit->brushes)
  , rightWidgetSection(newPrettyScopeModuleToEdit, &openGLCanvas)
  , rightWidgetScroller(&rightWidgetSection, false)
	, prettyScopeModule(newPrettyScopeModuleToEdit)
{
	//setHeadlineText("PrettyScope ("+juce::String(JucePlugin_VersionString)+")"+ pluginFileManager.getDebugText() + newPrettyScopeModuleToEdit->moduleNameAppendix);	 
	//newPrettyScopeModuleToEdit->setActiveDirectory(getFactoryPresetFolder());

	setHeadlineText("PrettyScope");
	
	openGLCanvas.setScreenshotPath(pluginFileManager.desktopPath + "/PrettyScope Screenshots");
	openGLCanvas.setShaderPath(pluginFileManager.userFolder.module + "/Shaders");
	prettyScopeModule->colorMap.setActiveDirectory(pluginFileManager.programFolder.module + "/ColorMaps");

  addAndMakeVisible(openGLCanvas);

  addAndMakeVisible(rightWidgetScroller);
  rightWidgetScroller.fixScrollBars(false, true);

  setSize(840, 676);
}

void PrettyScopeEditor::paint(Graphics& g)
{
  AudioModuleEditor::paint(g);
}

AudioModuleEditor* PrettyScopeModule::createEditor(int type)
{
	auto editor = new PrettyScopeEditor(this);

	editor->BasicEditorWithLicensing::keyValidator.setProductIndex(se::KeyGenerator::productIndices::PRETTYSCOPE);
	editor->initializePlugIn();

	return editor;
}

void PrettyScopeEditor::resized()
{
	BasicEditorWithLicensing::resized();

	int widgetMargin = rightWidgetSection.desiredWidth;

	int w = std::max<float>(widgetMargin+10, getWidth());
	int h = std::max<float>(10, getHeight());

	if (prettyScopeModule)
	{
		if (h > 0)
		{
			const double screenAR = w / static_cast<double>(h);

			//ignore_unused(screenAR);

			// We shouldn't call directly into audio processor
			//prettyScopeModule->setScreenAspectRatio(screenAR);
		}
	}
		
	if (openGLCanvas.hideGUI)
	{
		stateWidgetSet->setVisible(false);
		rightWidgetScroller.setVisible(false);
	}
	else
	{
		stateWidgetSet->setVisible(true);
		rightWidgetScroller.setVisible(true);

		rightWidgetScroller.setBounds
		(
			getWidth() - rightWidgetSection.desiredWidth,
			getPresetSectionBottom(),
			rightWidgetSection.desiredWidth,
			getHeight() - 24
		);

		rightWidgetScroller.setScrolleeSize
		(
			rightWidgetSection.desiredWidth - rightWidgetScroller.getScrollBarThickness(),
			rightWidgetSection.desiredHeight
		);
	}

	if (alertBox.isVisible())
	{
		openGLCanvas.setSizeAndBounds(0, 0, 1, 1);
	}
	else if (openGLCanvas.hideGUI)
	{
		openGLCanvas.setSizeAndBounds(0, 0, getWidth(),	getHeight()	);
	}
	else
	{
		openGLCanvas.setSizeAndBounds
		(
			0,
			getPresetSectionBottom(),
			getWidth() - rightWidgetSection.desiredWidth,
			getHeight() - 24
		);
	}
}
