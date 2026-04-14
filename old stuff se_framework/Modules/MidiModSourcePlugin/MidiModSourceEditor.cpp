#include "MidiModSourceEditor.h"

MidiModSourceEditor::MidiModSourceEditor(MidiModSourceModule *newMidiModSourceToEdit)
  : BasicEditor(newMidiModSourceToEdit)
{
  ScopedLock scopedLock(*lock);

  MidiModSourceToEdit = newMidiModSourceToEdit;

	BasicEditor::createWidgets();
  createWidgets();
}

void MidiModSourceEditor::createWidgets()
{
	static_cast<ElanModulatableComboBox *>(MidiModSourceToEdit->parSource.widget)->registerComboBoxObserver(this);
}

void MidiModSourceEditor::resized()
{
  ScopedLock scopedLock(*lock);

  Editor::resized();

	auto & e = MidiModSourceToEdit;

  int x  = 0;
  int y  = 24;
  int w  = getWidth();
  int m  = 8; // margin
  int sh = 16; // slider height

	e->parCCSelection.widget->setVisible(int(e->parSource) == MidiModSourceModule::MidiSrcSel::cc);
	e->parKeytrackCenter.widget->setVisible(int(e->parSource) == MidiModSourceModule::MidiSrcSel::keytrack);

	e->parSource.setBounds(x+m, y, w-m*2, sh); y+=sh-2;

	e->parCCSelection.setBounds(x+m, y, w-m*2, sh); e->parKeytrackCenter.setBounds(x+m, y, w-m*2, sh); y+=sh-2;

	e->parSmoothing.setBounds(x+m, y, w-m*2, sh); y+=sh-2;
	e->parAmplitude.setBounds(x+m, y, w-m*2, sh); y+=sh-2;
	e->parOffset.setBounds(x+m, y, w-m*2, sh); y+=sh-2;
}

void MidiModSourceEditor::rButtonClicked(jura::RButton* button)
{
  AudioModuleEditor::rButtonClicked(button);
}

//=================================================================================================

void MidiModSourceEditor::updateWidgetsAccordingToState()
{
	AudioModuleEditor::updateWidgetsAccordingToState();

	auto & e = MidiModSourceToEdit;
	e->parCCSelection.widget->setVisible(int(e->parSource) == MidiModSourceModule::MidiSrcSel::cc);
	e->parKeytrackCenter.widget->setVisible(int(e->parSource) == MidiModSourceModule::MidiSrcSel::keytrack);
}
