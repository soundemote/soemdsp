#pragma once

class MidiModSourceEditor : public BasicEditor, public jura::RComboBoxObserver
{
	friend MidiModSourceModule;

public:
	MidiModSourceEditor(MidiModSourceModule *newMidiModSourceToEdit);

	virtual ~MidiModSourceEditor() = default;

  virtual void createWidgets() override;

  // overriden callbacks
  virtual void resized() override;
  virtual void rButtonClicked(jura::RButton* button) override;
	virtual void rComboBoxChanged(jura::RComboBox* /*comboBoxThatHasChanged*/) override { resized(); }

  virtual void updateWidgetsAccordingToState() override;

protected:
	int initWidth = 200;
	int initHeight = 108;

	MidiModSourceModule *MidiModSourceToEdit;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiModSourceEditor);

private:
#if JUCE_DEBUG
	String debug_str = "DEBUG: ";
#else
	String debug_str = "";
#endif
};
