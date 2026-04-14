#include "XoxosOscillatorEditor.h"

XoxosOscillatorEditor::XoxosOscillatorEditor(XoxosOscillatorModule *newXoxosOscillatorToEdit)
  : jura::AudioModuleEditor(newXoxosOscillatorToEdit)
{
  ScopedLock scopedLock(*lock);
  XoxosOscillatorToEdit = newXoxosOscillatorToEdit;
	setHeadlineText("Basic Oscillator ("+juce::String(JucePlugin_VersionString)+")"+newXoxosOscillatorToEdit->moduleNameAppendix);
  createWidgets();
  setSize(initWidth, initHeight);
}

void XoxosOscillatorEditor::createWidgets()
{
	for (myparams * p : XoxosOscillatorToEdit->paramManager)
	{
		switch (p->type)
		{
		case BUTTON:
			addWidget(p->button = new jura::rsAutomatableButton(p->text));
			p->button->assignParameter(p->ptr);
			break;
		case SLIDER:
			addWidget(p->slider = new jura::rsModulatableSlider());
			p->slider->assignParameter(p->ptr);
			p->slider->setSliderName(p->text);
			p->slider->setStringConversionFunction(p->stringConvertFunc);
			break;
		case COMBOBOX:
			addWidget(p->combobox = new jura::rsAutomatableComboBox());
			p->combobox->assignParameter(p->ptr);
			//p->combobox->registerComboBoxObserver(this);
			break;
		}
		p->getWidget()->setDescriptionField(infoField);
		p->getWidget()->setDescription("");
	}
}

void XoxosOscillatorEditor::resized()
{
  AudioModuleEditor::resized(); // preliminary

  ScopedLock scopedLock(*lock);
  AudioModuleEditor::resized();

	auto & e = XoxosOscillatorToEdit;

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

	auto * sg = XoxosOscillatorToEdit;

	for (auto & p : e->paramManager)
	{
		p->setBounds(m, y, sw, sh); 
		y+=sh-2;
	}
}

void XoxosOscillatorEditor::rButtonClicked(jura::RButton* button)
{
  AudioModuleEditor::rButtonClicked(button);
}

//=================================================================================================

void XoxosOscillatorEditor::updateWidgetsAccordingToState()
{
	AudioModuleEditor::updateWidgetsAccordingToState();
}

jura::AudioModuleEditor* XoxosOscillatorModule::createEditor(int type)
{
	return new XoxosOscillatorEditor(this);
}

