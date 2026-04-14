#include "JerobeamBoingEditor.h"

JerobeamBoingEditor::JerobeamBoingEditor(JerobeamBoingModule *newJerobeamBoingToEdit)
  : jura::AudioModuleEditor(newJerobeamBoingToEdit)
{
  ScopedLock scopedLock(*lock);
  JerobeamBoingToEdit = newJerobeamBoingToEdit;
	setHeadlineText("Jerobeam Boing ("+juce::String(JucePlugin_VersionString)+")"+newJerobeamBoingToEdit->moduleNameAppendix);
  createWidgets();
  setSize(initWidth, initHeight);
}

void JerobeamBoingEditor::createWidgets()
{
	for (myparams * p : JerobeamBoingToEdit->paramStrIds)
	{
		switch (p->type)
		{
		case BUTTON:
			addWidget(p->button = new jura::AutomatableButton(p->text));
			p->button->assignParameter(p->ptr);
			break;
		case SLIDER:
			addWidget(p->slider = new jura::AutomatableSlider());
			p->slider->assignParameter(p->ptr);
			p->slider->setSliderName(p->text);
			p->slider->setStringConversionFunction(p->stringConvertFunc);
			break;
		case COMBOBOX:
			addWidget(p->combobox = new jura::AutomatableComboBox());
			p->combobox->assignParameter(p->ptr);
			//p->combobox->registerComboBoxObserver(this);
			break;
		}
		p->getWidget()->setDescriptionField(infoField);
		p->getWidget()->setDescription("");
	}

	// sub-editors (must be added after the parameter widgets to be in front of them):
	breakpointEnv1Editor = new jura::BreakpointModulatorEditor(lock, JerobeamBoingToEdit->breakpointEnv1Module);
	breakpointEnv1Editor->setHeadlineText("ModEnv");
	addChildEditor(breakpointEnv1Editor);
}

void JerobeamBoingEditor::resized()
{
  AudioModuleEditor::resized(); // preliminary

  ScopedLock scopedLock(*lock);
  AudioModuleEditor::resized();

	auto & e = JerobeamBoingToEdit;

  int x  = 0;
  int y  = getPresetSectionBottom()+8;
  int w  = getWidth();
	int w2 = w/2;
  int w3 = w/3;
  int h  = getHeight();

  int m  = 8;      // margin

  int sh = 16;     // slider height
	int sh2 = (int)sh*1.5;   // bigger slider height
  int dy = sh+8;   // delta y between between widget groups
  int sw = w3-2*m; // slider width
	int sw2 = sw/2; // slider half width
	int sw3 = sw/3;

  int pbw = 70;    // page button width

	 /* COLUMN 1: GENERAL */
	m = w3*0+8;

	auto * sg = JerobeamBoingToEdit;

	for (auto & p : e->paramStrIds)
	{
		p->setBounds(m, y, sw, sh); 
		y+=sh-2;
	}

	breakpointEnv1Editor->setBounds(m, y, w-m-m, h-y);
}

void JerobeamBoingEditor::rButtonClicked(jura::RButton* button)
{
  AudioModuleEditor::rButtonClicked(button);
}

void JerobeamBoingEditor::updateWidgetsAccordingToState()
{
	AudioModuleEditor::updateWidgetsAccordingToState();
}

jura::AudioModuleEditor * JerobeamBoingModule::createEditor()
{
	return new JerobeamBoingEditor(this);
}