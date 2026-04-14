#include "BasicOscillatorEditor.h"

BasicOscillatorEditor::BasicOscillatorEditor(BasicOscillatorModule *newBasicOscillatorToEdit)
  :BasicEditor(newBasicOscillatorToEdit)
{
  ScopedLock scopedLock(*lock);

  basicOscillatorToEdit = newBasicOscillatorToEdit;

	BasicEditor::createWidgets();
  createWidgets();
}

void BasicOscillatorEditor::createWidgets()
{
	addWidget(&ResetText);
	addWidget(&ShapeText);
	ResetText.setText("Reset:");
	ShapeText.setText("Shape:");

	static_cast<rsAutomatableButton *>(basicOscillatorToEdit->parKeytracking.widget)->addRButtonListener(this);
}

void BasicOscillatorEditor::rButtonClicked(jura::RButton* button)
{
  AudioModuleEditor::rButtonClicked(button);

	if (button == basicOscillatorToEdit->parKeytracking.widget)
	{
		auto * widgetTune = basicOscillatorToEdit->parTune.widget;
		auto * widgetFreqCoarse = basicOscillatorToEdit->parFreqCoarse.widget;

		widgetTune->setVisible(basicOscillatorToEdit->parKeytracking == 1.0);
		widgetFreqCoarse->setVisible(basicOscillatorToEdit->parKeytracking == 0.0);
	}
}

void BasicOscillatorEditor::updateWidgetsAccordingToState()
{
	AudioModuleEditor::updateWidgetsAccordingToState();
	basicOscillatorToEdit->parTune.widget->setVisible(basicOscillatorToEdit->parKeytracking == 1.0);
	basicOscillatorToEdit->parFreqCoarse.widget->setVisible(basicOscillatorToEdit->parKeytracking == 0.0);
}

void BasicOscillatorEditor::resized()
{
	ScopedLock scopedLock(*lock);
	Editor::resized();

	auto & e = basicOscillatorToEdit;

	int y  = 7;
	int w  = getWidth();
	int m  = 8; // margin
	int sh = 16; // slider height
	int spacing = sh+4;

	e->parGain.setBounds(46+8, y, w-(8+46+8), sh); y+=spacing;
	e->parUnipolar.setBounds(8, y, 56/2, sh); e->parInvert.setBounds(8+56/2, y, 56/2, sh);
	e->parNoiseAmp.setBounds(56+8+8, y, w-(8+56+8+8), sh);

	y+=spacing;

	// common
	ShapeText.setBounds(m, y, 46, sh);
	e->parWaveform.setBounds(m+46, y, w-m*2-46, sh);

	y+=spacing;

	ResetText.setBounds(m, y, 46, sh);
	e->parResetMode.setBounds(m+46, y, w-m*2-46, sh);

	y+=spacing;

	// oscillator	

	e->parPhaseOffset.setBounds(m, y, w-m*2, sh);

	y+=spacing;
	
	e->parTempo.setBounds(m, y, w-m*2, sh); y+=sh-2;
	e->parFrequency.setBounds(m, y, w-m*2, sh); y+=sh-2;
	e->parKeytracking.setBounds(m, y, 48, sh);
	e->parFreqCoarse.setBounds(m+48-2+4, y, w-m*2-48+2-4, sh);
	e->parTune.setBounds(m+48-2+4, y, w-m*2-48+2-4, sh);	

	y+=spacing;

	// adsr
	e->parEnvDelay.setBounds(m, y, w-m*2, sh); y+=sh-2;
	e->parEnvAttack.setBounds(m, y, w-m*2, sh); y+=sh-2;
	e->parEnvDecay.setBounds(m, y, w-m*2, sh); y+=sh-2;
	e->parEnvSustain.setBounds(m, y, w-m*2, sh); y+=sh-2;
	e->parEnvRelease.setBounds(m, y, w-m*2, sh);

	y+=spacing;

	// filter
	e->parHPF.setBounds(m, y, w-m*2, sh); y+=sh-2;
	e->parLPF.setBounds(m, y, w-m*2, sh);

	//y+=spacing;

	//e->parOversampling.setBounds(m, y, w-m*2, sh);
}
