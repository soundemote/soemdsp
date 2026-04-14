#pragma once

#include "JerobeamBoingModule.h"

class JerobeamBoingEditor : public jura::AudioModuleEditor
{
public:
  JerobeamBoingEditor(JerobeamBoingModule *newJerobeamBoingToEdit);

	virtual ~JerobeamBoingEditor() = default;

  virtual void createWidgets();

  // overriden callbacks
  virtual void resized() override;
  virtual void rButtonClicked(jura::RButton* button) override;
  virtual void updateWidgetsAccordingToState() override;

protected:
	int initWidth = 685;
	int initHeight = 840;

  JerobeamBoingModule *JerobeamBoingToEdit;
	jura::BreakpointModulatorEditor *breakpointEnv1Editor;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(JerobeamBoingEditor)
};