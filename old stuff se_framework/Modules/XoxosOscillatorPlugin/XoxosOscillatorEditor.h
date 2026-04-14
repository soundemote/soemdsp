#pragma once

#include "XoxosOscillatorModule.h"

class XoxosOscillatorEditor : public jura::AudioModuleEditor
{
public:
	XoxosOscillatorEditor(XoxosOscillatorModule *newXoxosOscillatorToEdit);

	virtual ~XoxosOscillatorEditor() = default;

  virtual void createWidgets();

  // overriden callbacks
  virtual void resized() override;
  virtual void rButtonClicked(jura::RButton* button) override;
  virtual void updateWidgetsAccordingToState() override;

protected:
	int initWidth = 685;
	int initHeight = 840;

	XoxosOscillatorModule *XoxosOscillatorToEdit;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(XoxosOscillatorEditor)
};