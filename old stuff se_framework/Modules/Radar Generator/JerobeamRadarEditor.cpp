#include "JerobeamRadarEditor.h"

JerobeamRadarEditor::JerobeamRadarEditor(JerobeamRadarModule *newJerobeamRadarToEdit)
	: BasicEditorWithLicensing(newJerobeamRadarToEdit)
{
	ScopedLock scopedLock(*lock);

	JerobeamRadarToEdit = newJerobeamRadarToEdit;
	JerobeamRadarToEdit->addChangeListener(this);

	addMouseListener(&oscilloscopeArea, false);
	oscilloscopeArea.setInterceptsMouseClicks(false, true);

	BasicEditor::createWidgets();
	JerobeamRadarEditor::createWidgets();

	FeedbackText.setInterceptsMouseClicks(false, false);

	setHeadlinePosition(headlinePositions::TOP_LEFT);
	setHeadlineStyle(headlineStyles::SUB_HEADLINE);
	setPresetSectionPosition(positions::RIGHT_TO_HEADLINE);
	infoField->setVisible(true);
}

void JerobeamRadarEditor::createWidgets()
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

	addWidget(&RadarText);
	addWidget(&RadarResetText);
	addWidget(&ToneModText);

	addWidget(&HelpText);
	HelpText.setFontStyle("Arial", 14);
	HelpText.setDrawMultilineText(true);

	addWidget(&pageADSR_1_3);
	pageADSR_1_3.addToRadioButtonGroup(&pageButtonGroup);
	pageADSR_1_3.addRButtonListener(this);

	addWidget(&pageADSR_4_6);
	pageADSR_4_6.addToRadioButtonGroup(&pageButtonGroup);
	pageADSR_4_6.addRButtonListener(this);

	addWidget(&pageLFO_1_4);
	pageLFO_1_4.addToRadioButtonGroup(&pageButtonGroup);
	pageLFO_1_4.addRButtonListener(this);

	addWidget(&pageLFO_5_8);
	pageLFO_5_8.addToRadioButtonGroup(&pageButtonGroup);
	pageLFO_5_8.addRButtonListener(this);

	addWidget(&pageBreakpoint1);
	pageBreakpoint1.addToRadioButtonGroup(&pageButtonGroup);
	pageBreakpoint1.addRButtonListener(this);

	addWidget(&pageBreakpoint2);
	pageBreakpoint2.addToRadioButtonGroup(&pageButtonGroup);
	pageBreakpoint2.addRButtonListener(this);

	addWidget(&pageMidiMod);
	pageMidiMod.addToRadioButtonGroup(&pageButtonGroup);
	pageMidiMod.addRButtonListener(this);

	addWidget(&pageHelp);
	pageHelp.addToRadioButtonGroup(&pageButtonGroup);
	pageHelp.addRButtonListener(this);

	addWidget(&pageFX);
	pageFX.addToRadioButtonGroup(&pageButtonGroup);
	pageFX.addRButtonListener(this);

	int i = 0; for (auto & obj : BasicEnvelopeEditorArray)
	{
		obj = new BasicEnvelopeEditor(JerobeamRadarToEdit->BasicEnvelopeModuleArray[i]);
		obj->setHeadlineText("ADSR "+String(i+1));
		addChildEditor(obj);
		++i;
	}

	i = 0; for (auto & obj : BasicOscillatorEditorArray)
	{
		obj = new BasicOscillatorEditor(JerobeamRadarToEdit->BasicOscillatorModuleArray[i]);
		obj->setHeadlineText("LFO "+String(i+1));
		addChildEditor(obj);
		++i;
	}

	i = 0; for (auto & obj : MidiModSourceEditorArray)
	{
		obj = new MidiModSourceEditor(JerobeamRadarToEdit->MidiModSourceModuleArray[i]);
		obj->setHeadlineText("MIDI "+String(i+1));
		addChildEditor(obj);
		++i;
	}

	Breakpoint1Editor = new jura::BreakpointModulatorEditor(lock, JerobeamRadarToEdit->Breakpoint1Module);
	Breakpoint1Editor->setHeadlineText("Breakpoint 1");
	addChildEditor(Breakpoint1Editor);

	Breakpoint2Editor = new jura::BreakpointModulatorEditor(lock, JerobeamRadarToEdit->Breakpoint2Module);
	Breakpoint2Editor->setHeadlineText("Breakpoint 2");
	addChildEditor(Breakpoint2Editor);

	equalizerEditor = new EqualizerEditor(lock, JerobeamRadarToEdit->equalizerModule);
	equalizerEditor->setHeadlineText("Equalizer");
	addChildEditor(equalizerEditor);

	limiterEditor = new LimiterEditor(JerobeamRadarToEdit->limiterModule);
	limiterEditor->setHeadlineText("Limiter");
	addChildEditor(limiterEditor);

	bandpassEditor = new BandpassEditor(JerobeamRadarToEdit->jbRadarCore.bandpassModule);
	bandpassEditor->setHeadlineText("Filter");
	addChildEditor(bandpassEditor);

	delayEditor = new DelayEditor(JerobeamRadarToEdit->delayModule);
	delayEditor->setHeadlineText("Delay");
	addChildEditor(delayEditor);

	oscilloscopeEditor = new OscilloscopeEditor(JerobeamRadarToEdit->oscilloscopeModule);
	oscilloscopeEditor->setHeadlineText("Oscilloscope");
	addChildEditor(oscilloscopeEditor);
	oscilloscopeEditor->changeListenerCallback(nullptr);
	oscilloscopeEditor->addMouseListener(this, true);

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

	auto & e = JerobeamRadarToEdit;
	e->parPhase.widget->setColourScheme(tealColorScheme);
	e->parSubPhase.widget->setColourScheme(tealColorScheme);
	e->parSubPhaseRotation.widget->setColourScheme(pinkColorScheme);
	e->parMorph.widget->setColourScheme(pinkColorScheme);
	e->parDensity.widget->setColourScheme(tealColorScheme);
	e->parSharp.widget->setColourScheme(yellowColorScheme);
	e->parSharpCurve.widget->setColourScheme(yellowColorScheme);
	e->parCurveStart.widget->setColourScheme(yellowColorScheme);
	e->parCurveEnd.widget->setColourScheme(yellowColorScheme);
	e->parDirection.widget->setColourScheme(tealColorScheme);
	e->parShade.widget->setColourScheme(tealColorScheme);
	e->parLap.widget->setColourScheme(tealColorScheme);
	e->parPow2Bend.widget->setColourScheme(tealColorScheme);
	e->parLength.widget->setColourScheme(tealColorScheme);
	e->parFrontRing.widget->setColourScheme(tealColorScheme);
	e->parInner.widget->setColourScheme(tealColorScheme);
	e->parZDepth.widget->setColourScheme(tealColorScheme);
	e->parProtrudeX.widget->setColourScheme(pinkColorScheme);
	e->parProtrudeY.widget->setColourScheme(pinkColorScheme);
	e->parRatio.widget->setColourScheme(pinkColorScheme);
}

void JerobeamRadarEditor::rButtonClicked(jura::RButton* button)
{
	BasicEditorWithLicensing::rButtonClicked(button);

	if (isGuiPageButton(button))
	{
		BasicEnvelopeEditorArray[0]->setVisible(pageADSR_1_3.getToggleState());
		BasicEnvelopeEditorArray[1]->setVisible(pageADSR_1_3.getToggleState());
		BasicEnvelopeEditorArray[2]->setVisible(pageADSR_1_3.getToggleState());

		BasicEnvelopeEditorArray[3]->setVisible(pageADSR_4_6.getToggleState());
		BasicEnvelopeEditorArray[4]->setVisible(pageADSR_4_6.getToggleState());
		BasicEnvelopeEditorArray[5]->setVisible(pageADSR_4_6.getToggleState());

		BasicOscillatorEditorArray[0]->setVisible(pageLFO_1_4.getToggleState());
		BasicOscillatorEditorArray[1]->setVisible(pageLFO_1_4.getToggleState());
		BasicOscillatorEditorArray[2]->setVisible(pageLFO_1_4.getToggleState());
		BasicOscillatorEditorArray[3]->setVisible(pageLFO_1_4.getToggleState());

		BasicOscillatorEditorArray[4]->setVisible(pageLFO_5_8.getToggleState());
		BasicOscillatorEditorArray[5]->setVisible(pageLFO_5_8.getToggleState());
		BasicOscillatorEditorArray[6]->setVisible(pageLFO_5_8.getToggleState());
		BasicOscillatorEditorArray[7]->setVisible(pageLFO_5_8.getToggleState());

		Breakpoint1Editor->setVisible(pageBreakpoint1.getToggleState());
		Breakpoint2Editor->setVisible(pageBreakpoint2.getToggleState());

		const bool isBPResetVisible = pageBreakpoint1.getToggleState() || pageBreakpoint2.getToggleState();

		JerobeamRadarToEdit->parBP1Reset.widget->setVisible(isBPResetVisible);
		JerobeamRadarToEdit->parBP2Reset.widget->setVisible(isBPResetVisible);
		BP1ResetText.setVisible(isBPResetVisible);
		BP2ResetText.setVisible(isBPResetVisible);

		HelpText.setVisible(pageHelp.getToggleState());
		webLink->setVisible(pageHelp.getToggleState());

		equalizerEditor->setVisible(pageFX.getToggleState());
		limiterEditor->setVisible(pageFX.getToggleState());
		bandpassEditor->setVisible(pageFX.getToggleState());
		delayEditor->setVisible(pageFX.getToggleState());

		for (auto m : MidiModSourceEditorArray)
			m->setVisible(pageMidiMod.getToggleState());
	}
}

void JerobeamRadarEditor::updateWidgetsAccordingToState()
{
	AudioModuleEditor::updateWidgetsAccordingToState();

	for (auto & obj : MidiModSourceEditorArray)
		obj->updateWidgetsAccordingToState();
	for (auto & obj : BasicOscillatorEditorArray)
		obj->updateWidgetsAccordingToState();

	Breakpoint1Editor->updateWidgetsAccordingToState();
	Breakpoint2Editor->updateWidgetsAccordingToState();
}

jura::AudioModuleEditor * JerobeamRadarModule::createEditor(int id)
{
	auto ptr = new JerobeamRadarEditor(this);

	ptr->setSize(ptr->initWidth, ptr->initHeight);

	ptr->pageLFO_1_4.setToggleState(true, true);

	ptr->updateWidgetsAccordingToState();

	ptr->BasicEditorWithLicensing::keyValidator.setProductIndex(se::KeyGenerator::productIndices::RADARGENERATOR);
	ptr->BasicEditorWithLicensing::initializePlugIn();

	return ptr;
}

void JerobeamRadarEditor::resized()
{
	ScopedLock scopedLock(*lock);
	BasicEditorWithLicensing::resized();

	const int margin = 4;
	const int smallSliderHeight = 16;

	int w = int(round(getWidth()/6.0)*6 - 1);
	int h  = int(round(getHeight()/2.0)*2);
	auto & e = JerobeamRadarToEdit;

	int w3 = w/3;

	int m  = 8; // margin
	int hm = m/2; // half margin

	int sh = 16; // slider height
	int bsh = 24; // big slider height

	int sw = w3-2*m; // slider width

	int th = 18; // text height
	int bth = 36; // big text height

	int ssp = sh+6; // section spacing;
	int mssp = 4; // mini slider section spacing

	int bw; // button width

	int x;

	infoField->setBounds(m, 442, w-12, sh);
	int webLinkWidth = jura::BitmapFontRoundedBoldA10D0::instance.getTextPixelWidth(webLink->getButtonText(), 1);
	webLink->setBounds(m, getHeight()-16+3, webLinkWidth, 16);

	/** COLUMN 1 **/
	m = w3*0+4;
	int y = getPresetSectionBottom()+8;

	{ // stereo
		x = m;

		StereoText.setBounds(m, y, 124, th);

		x = StereoText.getRight();

		int w = w3 - m - hm - StereoText.getWidth();

		e->parGain.setBounds(x, y, w - 4, sh);

		y+=th;

		x = m;

		StereoModeText.setBounds(x, y, 44, th);

		x = StereoModeText.getRight() - 8;

		e->parChannelMode.setBounds(x+4, y, 88, sh);

		x = e->parChannelMode.widget->getRight() + 4;

		w = sw - StereoModeText.getWidth() - e->parChannelMode.widget->getWidth();

		e->parStereoRotate.setBounds(x, y, w+4, sh);
	}

	y+=sh+mssp;

	{ // clipping
		x = m;

		ClippingModeText.setBounds(x, y, 54, th);

		x = ClippingModeText.getRight();

		e->parClippingMode.setBounds(x+4, y, 36+4, sh);

		x = e->parClippingMode.widget->getRight() + 4;

		int w =	sw
			-ClippingModeText.getWidth()
			-e->parClippingMode.widget->getWidth()
			-4 * 2;

		e->parClippingLevel.setBounds(x, y, w+4, sh);
	}

	{ // X / Y offset
		y = e->parClippingLevel.widget->getBottom() + mssp;

		int w = (sw-margin*2)/3;
		int x = margin*2;

		e->parDC.setBounds(x, y, w+50, smallSliderHeight);

		x = e->parDC.widget->getRight() + margin;

		e->parX.setBounds(x, y, w-25, smallSliderHeight);

		x = e->parX.widget->getRight() + margin;

		e->parY.setBounds(x, y, w-25, smallSliderHeight);
	}
	
	x = margin*2;
	y = e->parY.widget->getBottom() + margin;
	
	oscilloscopeEditor->stateWidgetSet->setBounds(0, 0, 0, 0);
	oscilloscopeEditor->setBounds(margin*2, y, w3 - margin*4, 226-margin);
	oscilloscopeArea.setBounds(oscilloscopeEditor->getBounds());
	y = oscilloscopeEditor->getBottom()-14;

	/* FEEDBACK */
	FeedbackText.setBounds(m+54+16, y+16, sw, th); y+=th;

	FBSourceText.setBounds(x, y, 54, th); x+=54;
	e->parFeedbackSrc.setBounds(x, y, 88, sh); y+=sh+mssp;
	e->parFeedbackHPF.setBounds(margin*2, y, sw, sh); y+=sh-2;
	e->parFeedbackLPF.setBounds(margin*2, y, sw, sh); y+=sh+mssp;
	e->parFeedbackAmp.setBounds(margin*2, y, sw, sh);

	y+=ssp-4;

	{ // Smoothing and oversampling
		SettingsText.setBounds(m, y, sw, th); y+=th;
		e->parSmoothing.setBounds(margin*2, y, sw-108-4, sh);
		e->parOversampling.setBounds(sw-108+8, y, 108, sh); y+=sh+mssp;
	}

	/** COLUMN 2 **/
	m = w3*1+4;
	y = getPresetSectionBottom()+8;

	{ //radar controls
		RadarText.setBounds(m, y, sw, bth); y+=bth;
		x = m;
		RadarResetText.setBounds(x, y, 54, sh);
		x += 46;
		e->parResetMode.setBounds(x, y, 77, sh);
		x += 77+4;
		e->parCoreGain.setBounds(x, y, w-(8+x+w3), sh);
		y+=sh+mssp;

		int width = sw+6;

		e->parPhase.setBounds(m, y, width, sh); y+=sh-2;
		e->parSubPhase.setBounds(m, y, width, sh); y+=sh-2;
		e->parSubPhaseRotation.setBounds(m, y, width, sh);

		y+=sh+mssp;

		e->parMorph.setBounds(m, y, width, bsh);

		y+=bsh+mssp;

		e->parDensity.setBounds(m, y, width, bsh);

		y+=bsh+mssp;

		e->parSharp.setBounds(m, y, width, sh); y+=sh-2;
		e->parSharpCurve.setBounds(m, y, sw-18-18-6-6, sh);
		e->parCurveStart.setBounds(m+(sw-18-18-6), y, 18, sh);
		e->parCurveEnd.setBounds(m+(sw-18), y, 18, sh);

		y+=sh+mssp;

		e->parDirection.setBounds(m, y, width, sh); y+=sh-2;
		e->parShade.setBounds(m, y, width, sh); y+=sh-2;
		e->parLap.setBounds(m, y, width, sh); y+=sh-2;
		e->parPow2Bend.setBounds(m, y, width, sh);

		y+=sh+mssp;

		e->parLength.setBounds(m, y, width, sh); y+=sh-2;
		e->parFrontRing.setBounds(m, y, width, sh); y+=sh-2;
		e->parInner.setBounds(m, y, width, sh); y+=sh-2;
		e->parZDepth.setBounds(m, y, width, sh);

		y+=sh+mssp;

		e->parProtrudeX.setBounds(m, y, width, sh); y+=sh-2;
		e->parProtrudeY.setBounds(m, y, width, sh); y+=sh-2;
		e->parRatio.setBounds(m, y, width, sh);

		y+=sh+mssp;

		e->parRingCut.setBounds(m+12, y, sw/2-24, sh);
		e->parSpiralReturn.setBounds(m+sw/2+12, y, sw/2-24, sh); y+=sh-2;
		e->parPhaseInv.setBounds(m+12, y, sw/2-24, sh);
		e->parTunnelInv.setBounds(m+sw/2+12, y, sw/2-24, sh); y+=sh-2;
	}

	/** COLUMN 3 **/
	m = w3*2+8;
	y = getPresetSectionBottom()+4;
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

	int ysms = 466; // start of modulator section

	bw = 80; // button width
	int bws = 80; // button spacing
	int bx = 0; // button x;
	int bh = 20; // button height

	pageLFO_1_4.setBounds(bx+8*1, ysms, bw, bh); bx+=bws;
	pageLFO_5_8.setBounds(bx+8*2, ysms, bw, bh); bx+=bws;
	pageADSR_1_3.setBounds(bx+8*3, ysms, bw, bh); bx+=bws;
	pageADSR_4_6.setBounds(bx+8*4, ysms, bw, bh); bx+=bws;
	pageBreakpoint1.setBounds(bx+8*5, ysms, bw, bh); bx+=bws;
	pageBreakpoint2.setBounds(bx+8*6, ysms, bw, bh); bx+=bws;
	pageMidiMod.setBounds(bx+8*7, ysms, bw, bh); bx+=bws;
	pageFX.setBounds(bx+8*8, ysms, bw, bh); bx+=bws;
	pageHelp.setBounds(bx+8*9, ysms, bw, bh);

	ysms+=ssp+4;

	{
		int x = margin * 2;
		int y = pageLFO_1_4.getBottom() + margin;

		BP1ResetText.setBounds(x, y, 124, sh);

		x += 124 + margin;

		e->parBP1Reset.setBounds(x, y, 88, sh);

		x += 88 + margin*2;

		BP2ResetText.setBounds(x, y, 124, sh);

		x += 124 + margin;

		e->parBP2Reset.setBounds(x, y, 88, sh);
	}

	y+=sh+mssp;

	{
		const int y =  ysms + smallSliderHeight + margin;
		const int width = w-8*2;
		const int height = h-ysms-16;
		Breakpoint1Editor->setBounds(8, y, width, height);
		Breakpoint2Editor->setBounds(8, y, width, height);
	}

	equalizerEditor->setBounds(8, ysms, (w/2)-8*2, h-ysms-8);

	{
		int x = equalizerEditor->getRight() + 8;
		bandpassEditor->setBounds(x, ysms, 156, 96);

		int y = bandpassEditor->getBottom() + 8;
		limiterEditor->setBounds(x, y, 156, h-ysms-8-104);
	}

	{
		int x = bandpassEditor->getRight() + margin * 2;
		int w = jmin(getWidth() - x - margin*2, 156);
		int h = equalizerEditor->getHeight();
		delayEditor->setBounds(x, ysms, w, h);
	}

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
