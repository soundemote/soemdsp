#include "ChaoticaEditor.h"

ChaoticaEditor::ChaoticaEditor(ChaoticaModule *newChaoticaToEdit)
  : jura::AudioModuleEditor(newChaoticaToEdit)
{
  ScopedLock scopedLock(*lock);
  ChaoticaToEdit = newChaoticaToEdit;

	setHeadlineText("Chaotica ("+juce::String(JucePlugin_VersionString)+")"+newChaoticaToEdit->moduleNameAppendix);

	//presetSectionPosition = AudioModuleEditor::positions::INVISIBLE;

  createWidgets();
  setSize(initWidth, initHeight);
}

void ChaoticaEditor::createWidgets()
{
	for (myparams * p : ChaoticaToEdit->paramManager)
	{
		switch (p->type)
		{
		case BUTTON:
			addWidget(p->button = new jura::AutomatableButton(p->text));
			p->button->assignParameter(p->ptr);
			break;
		case SLIDER:
			addWidget(p->slider = new jura::ModulatableSlider());
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
}

void ChaoticaEditor::resized()
{
  AudioModuleEditor::resized(); // preliminary

  ScopedLock scopedLock(*lock);
  AudioModuleEditor::resized();

	auto & e = ChaoticaToEdit;
	auto & p = e->paramManager;

	int x  = 0;
	int y  = 24;
	int w  = getWidth();
	int h  = getHeight();
	int m  = 8; // margin
	int sh = 16; // slider height

	// common
	e->parGain.setBounds(m, y, w-m*2, sh); y+=sh-2;
	e->parOversampling.setBounds(m, y, w-m*2, sh); y+=sh-2;

	y+=sh;

	// oscillator
	e->parStepSize.setBounds(m, y, w-m*2, sh); y+=sh-2;
	e->parRate.setBounds(m, y, w-m*2, sh); y+=sh-2;
	e->parChaoticaGain.setBounds(m, y, w-m*2, sh); y+=sh-2;
	e->parNLDrive.setBounds(m, y, w-m*2, sh); y+=sh-2;
	e->par2D.setBounds(m, y, w-m*2, sh); y+=sh-2;
	e->parDamping.setBounds(m, y, w-m*2, sh); y+=sh-2;
	e->parOffset.setBounds(m, y, w-m*2, sh); y+=sh-2;
	e->parQ.setBounds(m, y, w-m*2, sh); y+=sh-2;
	e->parXRotation.setBounds(m, y, w-m*2, sh); y+=sh-2;
	e->parYRotation.setBounds(m, y, w-m*2, sh); y+=sh-2;

	y+=sh;

	// filter
	e->parXFilter.setBounds(m, y, w-m*2, sh); y+=sh-2;
	e->parYFilter.setBounds(m, y, w-m*2, sh); y+=sh-2;
	e->parZFilter.setBounds(m, y, w-m*2, sh); y+=sh-2;

	y+=sh;
	e->parSawFreq.setBounds(m, y, w-m*2, sh); y+=sh-2;
	e->parSmoothing.setBounds(m, y, w-m*2, sh); y+=sh-2;
}

void ChaoticaEditor::rButtonClicked(jura::RButton* button)
{
  AudioModuleEditor::rButtonClicked(button);
}

//=================================================================================================

void ChaoticaEditor::updateWidgetsAccordingToState()
{
	AudioModuleEditor::updateWidgetsAccordingToState();
}

jura::AudioModuleEditor * ChaoticaModule::createEditor()
{
	return new ChaoticaEditor(this);
}

