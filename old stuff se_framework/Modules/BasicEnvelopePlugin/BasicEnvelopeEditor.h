#pragma once

using jura::AudioModuleEditor;
using jura::RComboBoxObserver;
using jura::RTextField;

class BasicEnvelopeEditor : public BasicEditor, public RComboBoxObserver
{
	friend BasicEnvelopeModule;

public:
	BasicEnvelopeEditor(BasicEnvelopeModule *newBasicEnvelopeToEdit);

	virtual ~BasicEnvelopeEditor() = default;

  virtual void createWidgets() override;

  // overriden callbacks
  virtual void resized() override;
  virtual void rButtonClicked(jura::RButton* button) override;
	virtual void rComboBoxChanged(jura::RComboBox*) override { resized(); }
  virtual void updateWidgetsAccordingToState() override;

protected:
	int initWidth = 200;
	int initHeight = 108;

	BasicEnvelopeModule* BasicEnvelopeToEdit;

	RTextField ResetText;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BasicEnvelopeEditor);

private:
#ifdef DEBUG
	String debug_str = "DEBUG: ";
#else
	String debug_str = "";
#endif
};
