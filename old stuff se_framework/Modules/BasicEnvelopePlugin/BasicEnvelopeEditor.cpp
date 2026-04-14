#include "BasicEnvelopeEditor.h"

BasicEnvelopeEditor::BasicEnvelopeEditor(BasicEnvelopeModule* newBasicEnvelopeToEdit)
  : BasicEditor(newBasicEnvelopeToEdit)
{
  ScopedLock scopedLock(*lock);
  BasicEnvelopeToEdit = newBasicEnvelopeToEdit;

	BasicEditor::createWidgets();
  createWidgets();
}

void BasicEnvelopeEditor::createWidgets()
{
	addWidget(&ResetText);
	ResetText.setText("Reset:");
}

void BasicEnvelopeEditor::rButtonClicked(jura::RButton* button)
{
	AudioModuleEditor::rButtonClicked(button);
}

void BasicEnvelopeEditor::updateWidgetsAccordingToState()
{
	AudioModuleEditor::updateWidgetsAccordingToState();
}

void BasicEnvelopeEditor::resized()
{
  ScopedLock scopedLock(*lock);

  Editor::resized();

	auto & e = BasicEnvelopeToEdit;

  int x  = 8;
  int y  = 7;
  int w  = getWidth();
	int cw = 0; // current width
  int m  = 8; // margin
  int sh = 16; // slider height
	int s = sh+4; // spacing

	cw = w-m*2-54;
	x += 54;
	e->parAmplitude.setBounds(x, y, cw * 4/6, sh);
	x += cw * 4/6 + 4;
	e->parVelInfluence.setBounds(x, y, cw * 2/6 - 4 , sh);

	x = m;
	y+=s;
	cw = 54;
	ResetText.setBounds(x, y, cw, sh);
	x+=54-10;
	cw = (w-54)/3-m*2;
	e->parResetMode.setBounds(x, y, 77, sh);
	x+=77+4;
	e->parLoop.setBounds(x, y, cw, sh);
	x+=cw+4;
	e->parMakeBipolar.setBounds(x, y, w-x-8-24, sh); e->parInvert.setBounds(x+(w-x-8-24), y, 24, sh);

	y+=s;

	e->parGlobalFB.setBounds(m, y, w-m*2, sh); y+=sh-2;
	e->parDelay.setBounds(m, y, w-m*2, sh);

	y+=s;

	e->parAttackShape.setBounds(m, y, 46, sh); 
	e->parAttackFB.setBounds(m+46-2, y, w-m*2-46+2, sh); y+=sh-2;

	e->parDecayShape.setBounds(m, y, 46, sh);
	e->parDecayFB.setBounds(m+46-2, y, w-m*2-46+2, sh); y+=sh-2;

	e->parSustain.setBounds(m, y, w-m*2, sh); y+=sh-2;

	e->parReleaseShape.setBounds(m, y, 46, sh);
	e->parReleaseFB.setBounds(m+46-2, y, w-m*2-46+2, sh);

	y+=s;

	e->parGlobalTime.setBounds(m, y, w-m*2, sh);
}

