#include "JerobeamSphereEditor.h"

JerobeamSphereEditor::JerobeamSphereEditor(JerobeamSphereModule *newJerobeamSphereToEdit)
	: BasicEditor(newJerobeamSphereToEdit)
{
	ScopedLock scopedLock(*lock);

	JerobeamSphereToEdit = newJerobeamSphereToEdit;

	BasicEditor::createWidgets();
	createWidgets();
}

void JerobeamSphereEditor::createWidgets()
{
	addWidget(&StereoText);
	addWidget(&FilterText);
	addWidget(&SettingsText);
	addWidget(&FeedbackText);
	addWidget(&KeytrackingText);
	addWidget(&TempoText);
	addWidget(&PitchText);
	addWidget(&AmpEnvText);
	addWidget(&EnvResetText);
	addWidget(&BP1ResetText);
	addWidget(&BP2ResetText);
	addWidget(&FBSourceText);
	addWidget(&StereoModeText);
	addWidget(&ClippingModeText);

	addWidget(&SphereText);
	addWidget(&SphereResetText);
	addWidget(&ToneModText);

	addWidget(&HelpText);
	HelpText.setFontStyle("Arial", 14);
	HelpText.setDrawMultilineText(true);

	addWidget(pageADSR_1_3 = new RRadioButton("ADSR 1 - 3"));
	pageADSR_1_3->addToRadioButtonGroup(&pageButtonGroup);
	pageADSR_1_3->addRButtonListener(this);

	addWidget(pageADSR_4_6 = new RRadioButton("ADSR 4 - 6"));
	pageADSR_4_6->addToRadioButtonGroup(&pageButtonGroup);
	pageADSR_4_6->addRButtonListener(this);

	addWidget(pageLFO_1_4 = new RRadioButton("LFO 1 - 4"));
	pageLFO_1_4->addToRadioButtonGroup(&pageButtonGroup);
	pageLFO_1_4->addRButtonListener(this);

	addWidget(pageLFO_5_8 = new RRadioButton("LFO 5 - 8"));
	pageLFO_5_8->addToRadioButtonGroup(&pageButtonGroup);
	pageLFO_5_8->addRButtonListener(this);

	addWidget(pageBreakpoint1 = new RRadioButton("Breakpoint 1"));
	pageBreakpoint1->addToRadioButtonGroup(&pageButtonGroup);
	pageBreakpoint1->addRButtonListener(this);

	addWidget(pageBreakpoint2 = new RRadioButton("Breakpoint 2"));
	pageBreakpoint2->addToRadioButtonGroup(&pageButtonGroup);
	pageBreakpoint2->addRButtonListener(this);

	addWidget(pageMidiMod = new RRadioButton("Midi Source"));
	pageMidiMod->addToRadioButtonGroup(&pageButtonGroup);
	pageMidiMod->addRButtonListener(this);

	addWidget(pageHelp = new RRadioButton("Help"));
	pageHelp->addToRadioButtonGroup(&pageButtonGroup);
	pageHelp->addRButtonListener(this);

	int i = 0; for (auto & obj : BasicEnvelopeEditorArray)
	{
		obj = new BasicEnvelopeEditor(JerobeamSphereToEdit->BasicEnvelopeModuleArray[i]);
		obj->setHeadlineText("ADSR "+String(i+1));
		addChildEditor(obj);
		++i;
	}

	i = 0; for (auto & obj : BasicOscillatorEditorArray)
	{
		obj = new BasicOscillatorEditor(JerobeamSphereToEdit->BasicOscillatorModuleArray[i]);
		obj->setHeadlineText("LFO "+String(i+1));
		addChildEditor(obj);
		++i;
	}

	i = 0; for (auto & obj : MidiModSourceEditorArray)
	{
		obj = new MidiModSourceEditor(JerobeamSphereToEdit->MidiModSourceModuleArray[i]);
		obj->setHeadlineText("MIDI "+String(i+1));
		addChildEditor(obj);
		++i;
	}

	Breakpoint1Editor = new jura::BreakpointModulatorEditor(lock, JerobeamSphereToEdit->Breakpoint1Module);
	Breakpoint1Editor->setHeadlineText("Breakpoint 1");
	addChildEditor(Breakpoint1Editor);

	Breakpoint2Editor = new jura::BreakpointModulatorEditor(lock, JerobeamSphereToEdit->Breakpoint2Module);
	Breakpoint2Editor->setHeadlineText("Breakpoint 2");

	addChildEditor(Breakpoint2Editor);

	jura::WidgetColourScheme tealColorScheme;
	tealColorScheme.setAppearance(jura::WidgetColourScheme::DARK_ON_BRIGHT);
	tealColorScheme.setSaturationMultiplier(0.5f);
	tealColorScheme.setCentralHue(0.49f);
	tealColorScheme.setBrightnessGamma(0.8f);

	jura::WidgetColourScheme yellowColorScheme;
	yellowColorScheme.setAppearance(jura::WidgetColourScheme::DARK_ON_BRIGHT);
	yellowColorScheme.setSaturationMultiplier(.5f);
	yellowColorScheme.setCentralHue(0.16f);
	yellowColorScheme.setBrightnessGamma(0.8f);

	jura::WidgetColourScheme pinkColorScheme;
	pinkColorScheme.setAppearance(jura::WidgetColourScheme::DARK_ON_BRIGHT);
	pinkColorScheme.setSaturationMultiplier(0.5f);
	pinkColorScheme.setCentralHue(0.83f);
	pinkColorScheme.setBrightnessGamma(0.8f);

	auto & e = JerobeamSphereToEdit;
	//e->parWanderSpd.widget->setColourScheme(pinkColorScheme);
}

void JerobeamSphereEditor::rButtonClicked(jura::RButton* button)
{
	AudioModuleEditor::rButtonClicked(button);

	if (isGuiPageButton(button))
	{
		BasicEnvelopeEditorArray[0]->setVisible(pageADSR_1_3->getToggleState());
		BasicEnvelopeEditorArray[1]->setVisible(pageADSR_1_3->getToggleState());
		BasicEnvelopeEditorArray[2]->setVisible(pageADSR_1_3->getToggleState());

		BasicEnvelopeEditorArray[3]->setVisible(pageADSR_4_6->getToggleState());
		BasicEnvelopeEditorArray[4]->setVisible(pageADSR_4_6->getToggleState());
		BasicEnvelopeEditorArray[5]->setVisible(pageADSR_4_6->getToggleState());

		BasicOscillatorEditorArray[0]->setVisible(pageLFO_1_4->getToggleState());
		BasicOscillatorEditorArray[1]->setVisible(pageLFO_1_4->getToggleState());
		BasicOscillatorEditorArray[2]->setVisible(pageLFO_1_4->getToggleState());
		BasicOscillatorEditorArray[3]->setVisible(pageLFO_1_4->getToggleState());

		BasicOscillatorEditorArray[4]->setVisible(pageLFO_5_8->getToggleState());
		BasicOscillatorEditorArray[5]->setVisible(pageLFO_5_8->getToggleState());
		BasicOscillatorEditorArray[6]->setVisible(pageLFO_5_8->getToggleState());
		BasicOscillatorEditorArray[7]->setVisible(pageLFO_5_8->getToggleState());

		Breakpoint1Editor->setVisible(pageBreakpoint1->getToggleState());
		Breakpoint2Editor->setVisible(pageBreakpoint2->getToggleState());

		HelpText.setVisible(pageHelp->getToggleState());
		webLink->setVisible(pageHelp->getToggleState());

		for (auto m : MidiModSourceEditorArray)
			m->setVisible(pageMidiMod->getToggleState());
	}
}

void JerobeamSphereEditor::updateWidgetsAccordingToState()
{
	AudioModuleEditor::updateWidgetsAccordingToState();
}

jura::AudioModuleEditor * JerobeamSphereModule::createEditor()
{
	auto ptr = new JerobeamSphereEditor(this);

	ptr->setSize(ptr->initWidth, ptr->initHeight);

	ptr->pageLFO_1_4->setToggleState(true, true);

	return ptr;
}

void JerobeamSphereEditor::resized()
{
	ScopedLock scopedLock(*lock);
	BasicEditor::resized();

	int w = int(round(getWidth()/6.0)*6 - 1);
	int h  = int(round(getHeight()/2.0)*2);
	auto & e = JerobeamSphereToEdit;

	int w3 = w/3;

	int m  = 8; // margin

	int sh = 16; // slider height
	int bsh = 24; // big slider height

	int sw = w3-2*m; // slider width

	int th = 18; // text height
	int bth = 36; // big text height

	int ssp = sh+6; // section spacing;
	int mssp = 4; // mini slider section spacing

	int bw; // button width

	int x;

	infoField->setBounds(8, 442, w-12, sh);
	int webLinkWidth = jura::BitmapFontRoundedBoldA10D0::instance.getTextPixelWidth(webLink->getButtonText(), 1);
	webLink->setBounds(8, getHeight()-16+3, webLinkWidth, 16);

	/** COLUMN 1 **/
	m = w3*0+8;
	int y = getPresetSectionBottom()+8;
	/* STEREO */
	StereoText.setBounds(m, y, sw, th); y+=th;
	x = m;
	StereoModeText.setBounds(x, y, 54, th); x+=54;
	e->parChannelMode.setBounds(x, y, 88, sh); y+=sh+mssp;
	e->parStereoRotate.setBounds(m, y, sw, sh); y+=sh+mssp;
	x = m;
	ClippingModeText.setBounds(x, y, 54, th); x+=54;
	e->parClippingMode.setBounds(x, y, 88, sh); y+=sh+mssp;
	e->parClippingLevel.setBounds(m, y, sw, sh); y+=sh-2;
	e->parGain.setBounds(m, y, sw, sh);

	y+=ssp;

	/* FILTER */
	FilterText.setBounds(m, y, sw, th); y+=th;
	e->parDC.setBounds(m, y, sw, sh); y+=sh-2;
	e->parX.setBounds(m+8*1, y, sw/2-8*2, sh);
	e->parY.setBounds(m+8*1+(sw/2-8*2)+8*2, y, sw/2-8*2, sh); y+=sh+mssp;;
	e->parHPF.setBounds(m, y, sw, sh); y+=sh-2;
	e->parLPF.setBounds(m, y, sw, sh);

	y+=ssp;

	/* FEEDBACK */
	FeedbackText.setBounds(m, y, sw, th); y+=th;
	x = m;
	FBSourceText.setBounds(x, y, 54, th); x+=54;
	e->parFeedbackSrc.setBounds(x, y, 88, sh); y+=sh+mssp;
	e->parFeedbackHPF.setBounds(m, y, sw, sh); y+=sh-2;
	e->parFeedbackLPF.setBounds(m, y, sw, sh); y+=sh+mssp;
	e->parFeedbackAmp.setBounds(m, y, sw, sh);

	y+=ssp;

	/* SETTINGS */
	SettingsText.setBounds(m, y, sw, th); y+=th;
	e->parSmoothing.setBounds(m, y, sw, sh); y+=sh+mssp;
	e->parOversampling.setBounds(m, y, sw, sh); y+=sh+mssp;
	x = m;
	BP1ResetText.setBounds(x, y, 124, sh); x+=124;
	e->parBP1Reset.setBounds(x, y, 88, sh); y+=sh+mssp;
	x = m;
	BP2ResetText.setBounds(x, y, 124, sh); x+=124;
	e->parBP2Reset.setBounds(x, y, 88, sh); y+=sh+mssp;

	/** COLUMN 2 **/
	m = w3*1+8;
	y = getPresetSectionBottom()+8;
	/* SPHERE */
	SphereText.setBounds(m, y, sw, bth); y+=bth;
	x = m;
	SphereResetText.setBounds(x, y, 54, sh);
	x += 46;
	e->parResetMode.setBounds(x, y, 77, sh);
	x += 77+4;
	e->parCoreGain.setBounds(x, y, w-(8+x+w3), sh);
	y+=sh+mssp;

	y+=sh-2;
	/* Sphere Controls */
	e->parPhase.setBounds(m, y, sw, sh); y+=sh-2;
	//e->parSubPhase.setBounds(m, y, sw, sh); y+=sh+mssp;

	e->parDensity.setBounds(m, y, sw, sh); y+=sh-2;
	e->parSharpness.setBounds(m, y, sw, sh); y+=sh-2;
	e->parRotX.setBounds(m, y, sw, sh); y+=sh-2;
	e->parRotY.setBounds(m, y, sw, sh); y+=sh-2;
	e->parZDepth.setBounds(m, y, sw, sh); y+=sh-2;
	e->parZAmount.setBounds(m, y, sw, sh); y+=sh-2;
	e->parEnds.setBounds(m, y, sw, sh); y+=sh-2;
	e->parBoing.setBounds(m, y, sw, sh); y+=sh-2;
	e->parJump.setBounds(m, y, sw, sh); y+=sh-2;
	e->parDirection.setBounds(m, y, sw, sh); y+=sh-2;
	e->parShape.setBounds(m, y, sw, sh); y+=sh-2;
	e->parVolume.setBounds(m, y, sw, sh); y+=sh-2;
	e->parVolumePreJump.setBounds(m, y, sw, sh); y+=sh-2;


	/** COLUMN 3 **/
	m = w3*2+8;
	y = getPresetSectionBottom()+8;
	/* PITCH & TEMPO */
	KeytrackingText.setBounds(m, y, sw, th); y+=th;
	e->parFreqMode.setBounds(m, y, sw/2-18, sh);
	e->parNoteChangeMode.setBounds(m+sw/2-18, y, sw/2+18, sh);

	y+=ssp;

	/* TEMPO */
	TempoText.setBounds(m, y, sw, th); y+=th;
	e->parResetWhenBeatChanges.setBounds(m, y, sw, sh); y+=sh+mssp;
	e->parBeatFreq.setBounds(m, y, sw, sh); y+=sh-2;
	e->parBeatTempo.setBounds(m, y, sw, sh); y+=sh-2;
	e->parBeatMult.setBounds(m, y, sw, sh); y+=sh+mssp;
	e->parFreqMult.setBounds(m, y, sw, sh);

	y+=ssp;

	/* PITCH */
	PitchText.setBounds(m, y, sw, th); y+=th;
	e->parGlideMode.setBounds(m, y, sw, sh); y+=sh+mssp;
	e->parGlideAmt.setBounds(m, y, sw, sh); y+=sh+mssp;
	e->parTune.setBounds(m, y, sw, sh); y+=sh-2;
	e->parOctave.setBounds(m, y, sw, sh); y+=sh-2;
	e->parHarmonicMult.setBounds(m, y, sw, sh); y+=sh+mssp;
	e->parHarmOctGlideAmt.setBounds(m, y, sw, sh);

	y+=ssp;

	/* AMP ENV */
	AmpEnvText.setBounds(m, y, sw, th); y+=th;
	x = m;
	EnvResetText.setBounds(x, y, 54, sh);
	x += 46;
	e->parEnvResetMode.setBounds(x, y, 77, sh);
	x += 77 + 4;
	e->parEnvVelInfluence.setBounds(x, y, sw-(77+4+46), sh); y+=sh+mssp;
	e->parEnvGlobalFB.setBounds(m, y, sw, sh); y+=sh+mssp;
	e->parEnvAttackShape.setBounds(m, y, 46, sh);
	e->parEnvAttackFB.setBounds(m+46-2, y, sw-46+2, sh); y+=sh-2;
	e->parEnvDecayShape.setBounds(m, y, 46, sh);
	e->parEnvDecayFB.setBounds(m+46-2, y, sw-46+2, sh); y+=sh-2;
	e->parEnvSustain.setBounds(m, y, sw, sh); y+=sh-2;
	e->parEnvReleaseShape.setBounds(m, y, 46, sh);
	e->parEnvReleaseFB.setBounds(m+46-2, y, sw-46+2, sh); y+=sh+mssp;
	e->parEnvGlobalTime.setBounds(m, y, sw, sh); y+=sh-2;

	int ysms = 462; // start of modulator section

	bw = 80; // button width
	int bws = 80; // button spacing
	int bx = 0; // button x;
	int bh = 20; // button height

	pageLFO_1_4->setBounds(bx+8*1, ysms, bw, bh); bx+=bws;
	pageLFO_5_8->setBounds(bx+8*2, ysms, bw, bh); bx+=bws;
	pageADSR_1_3->setBounds(bx+8*3, ysms, bw, bh); bx+=bws;
	pageADSR_4_6->setBounds(bx+8*4, ysms, bw, bh); bx+=bws;
	pageBreakpoint1->setBounds(bx+8*5, ysms, bw, bh); bx+=bws;
	pageBreakpoint2->setBounds(bx+8*6, ysms, bw, bh); bx+=bws;
	pageMidiMod->setBounds(bx+8*7, ysms, bw, bh); bx+=bws;
	pageHelp->setBounds(bx+8*8, ysms, bw, bh);

	ysms+=ssp+4;

	Breakpoint1Editor->setBounds(8, ysms, w-8*2, h-ysms-16);
	Breakpoint2Editor->setBounds(8, ysms, w-8*2, h-ysms-16);

	HelpText.setBounds(8, ysms, w-8*2, h-ysms-16);

	w3 = w/3-6;

	BasicEnvelopeEditorArray[0]->setBounds(w3*0+8, ysms, w3+2, 168);
	BasicEnvelopeEditorArray[1]->setBounds(w3*1+8, ysms, w3+2, 168);
	BasicEnvelopeEditorArray[2]->setBounds(w3*2+8, ysms, w3+2, 168);

	BasicEnvelopeEditorArray[3]->setBounds(w3*0+8, ysms, w3+2, 168);
	BasicEnvelopeEditorArray[4]->setBounds(w3*1+8, ysms, w3+2, 168);
	BasicEnvelopeEditorArray[5]->setBounds(w3*2+8, ysms, w3+2, 168);

	int w4 = w/4-4;

	BasicOscillatorEditorArray[0]->setBounds(w4*0+8, ysms, w4+2, 264);
	BasicOscillatorEditorArray[1]->setBounds(w4*1+8, ysms, w4+2, 264);
	BasicOscillatorEditorArray[2]->setBounds(w4*2+8, ysms, w4+2, 264);
	BasicOscillatorEditorArray[3]->setBounds(w4*3+8, ysms, w4+2, 264);

	BasicOscillatorEditorArray[4]->setBounds(w4*0+8, ysms, w4+2, 264);
	BasicOscillatorEditorArray[5]->setBounds(w4*1+8, ysms, w4+2, 264);
	BasicOscillatorEditorArray[6]->setBounds(w4*2+8, ysms, w4+2, 264);
	BasicOscillatorEditorArray[7]->setBounds(w4*3+8, ysms, w4+2, 264);

	int w6 = w/6-3;
	MidiModSourceEditorArray[0]->setBounds(w6*0+8, ysms, w6+2, 103);
	MidiModSourceEditorArray[1]->setBounds(w6*1+8, ysms, w6+2, 103);
	MidiModSourceEditorArray[2]->setBounds(w6*2+8, ysms, w6+2, 103);
	MidiModSourceEditorArray[3]->setBounds(w6*3+8, ysms, w6+2, 103);
	MidiModSourceEditorArray[4]->setBounds(w6*4+8, ysms, w6+2, 103);
	MidiModSourceEditorArray[5]->setBounds(w6*5+8, ysms, w6+2, 103);

	ysms+=103-2;

	MidiModSourceEditorArray[6]->setBounds(w6*0+8, ysms, w6+2, 103);
	MidiModSourceEditorArray[7]->setBounds(w6*1+8, ysms, w6+2, 103);
	MidiModSourceEditorArray[8]->setBounds(w6*2+8, ysms, w6+2, 103);
	MidiModSourceEditorArray[9]->setBounds(w6*3+8, ysms, w6+2, 103);
	MidiModSourceEditorArray[10]->setBounds(w6*4+8, ysms, w6+2, 103);
	MidiModSourceEditorArray[11]->setBounds(w6*5+8, ysms, w6+2, 103);
}
