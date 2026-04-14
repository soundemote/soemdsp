#pragma once

using jura::RTextField;
using jura::AudioModuleEditor;

class BasicOscillatorEditor : public BasicEditor
{
	friend BasicOscillatorModule;

public:
	BasicOscillatorEditor(BasicOscillatorModule *newBasicOscillatorToEdit);

	virtual ~BasicOscillatorEditor() = default;

  virtual void createWidgets();

  // overriden callbacks
  virtual void resized() override;
  virtual void rButtonClicked(jura::RButton* button) override;
  virtual void updateWidgetsAccordingToState() override;

protected:
	int initWidth = 685;
	int initHeight = 840;
	
	RTextField
		ResetText,
		ShapeText;

	BasicOscillatorModule *basicOscillatorToEdit;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BasicOscillatorEditor)
};
