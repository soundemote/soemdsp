#pragma once

#include "JerobeamBlubbModule.h"

class JerobeamBlubbEditor : public jura::AudioModuleEditor
{
public:
  JerobeamBlubbEditor(JerobeamBlubbModule *newJerobeamBlubbToEdit);

	virtual ~JerobeamBlubbEditor() = default;

  virtual void createWidgets();

  // overriden callbacks
  virtual void resized() override;
  virtual void rButtonClicked(jura::RButton* button) override;
  virtual void updateWidgetsAccordingToState() override;

protected:
	int initWidth = 685;
	int initHeight = 840;

  JerobeamBlubbModule *JerobeamBlubbToEdit;
	jura::BreakpointModulatorEditor *breakpointEnv1Editor;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(JerobeamBlubbEditor)
};