#pragma once

#include "JerobeamWirdoSpiralModule.h"

class JerobeamWirdoSpiralEditor : public jura::AudioModuleEditor
{
public:
  JerobeamWirdoSpiralEditor(JerobeamWirdoSpiralModule *newJerobeamWirdoSpiralToEdit);

	virtual ~JerobeamWirdoSpiralEditor() = default;

  virtual void createWidgets();

  // overriden callbacks
  virtual void resized() override;
  virtual void rButtonClicked(jura::RButton* button) override;
  virtual void updateWidgetsAccordingToState() override;

protected:
	int initWidth = 685;
	int initHeight = 840;

  JerobeamWirdoSpiralModule *JerobeamWirdoSpiralToEdit;
	jura::BreakpointModulatorEditor *breakpointEnv1Editor;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(JerobeamWirdoSpiralEditor)
};