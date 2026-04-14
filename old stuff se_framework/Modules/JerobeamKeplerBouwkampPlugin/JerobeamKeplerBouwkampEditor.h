#pragma once

#include "JerobeamKeplerBouwkampModule.h"

class JerobeamKeplerBouwkampEditor : public jura::AudioModuleEditor
{
public:
  JerobeamKeplerBouwkampEditor(JerobeamKeplerBouwkampModule *newJerobeamKeplerBouwkampToEdit);

	virtual ~JerobeamKeplerBouwkampEditor() = default;

  virtual void createWidgets();

  // overriden callbacks
  virtual void resized() override;
  virtual void rButtonClicked(jura::RButton* button) override;
  virtual void updateWidgetsAccordingToState() override;

protected:
	int initWidth = 685;
	int initHeight = 840;

  JerobeamKeplerBouwkampModule *JerobeamMushroomToEdit;
	jura::BreakpointModulatorEditor *breakpointEnv1Editor;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(JerobeamKeplerBouwkampEditor)
};