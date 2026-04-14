#include "JerobeamTorusEditor.h"

JerobeamTorusEditor::JerobeamTorusEditor(JerobeamTorusModule* newJerobeamTorusToEdit)
	: BasicEditorWithLicensing(newJerobeamTorusToEdit, this, "Torus Generator", "TorusGenerator", "https://www.soundemote.com/products/Torus-generator", "https://www.soundemote.com/products/Torus-generator")
{
	ScopedLock scopedLock(*lock);

	JerobeamTorusToEdit = newJerobeamTorusToEdit;
	JerobeamTorusToEdit->addChangeListener(this);
}

void JerobeamTorusEditor::createWidgets()
{
	addWidget(&TorusText);
	addWidget(&TorusResetText);
	addWidget(&ToneModText);

	auto & e = JerobeamTorusToEdit;
	//e->parWanderSpd.widget->setColourScheme(pinkColorScheme);
	//e->parDarkAngleSpd.widget->setColourScheme(pinkColorScheme);
	//e->parRotXSpd.widget->setColourScheme(pinkColorScheme);
	//e->parRotYSpd.widget->setColourScheme(pinkColorScheme);
	//e->parRotZSpd.widget->setColourScheme(pinkColorScheme);

	//e->parPhase.widget->setColourScheme(tealColorScheme);
	//e->parSubPhase.widget->setColourScheme(tealColorScheme);
	//e->parDensity.widget->setColourScheme(tealColorScheme);
	//e->parSDensity.widget->setColourScheme(tealColorScheme);

	//e->parSize.widget->setColourScheme(yellowColorScheme);
	//e->parLength.widget->setColourScheme(yellowColorScheme);
	//e->parSharp.widget->setColourScheme(yellowColorScheme);

	//e->parWander.widget->setColourScheme(tealColorScheme);
	//e->parDarkIntensity.widget->setColourScheme(tealColorScheme);
	//e->parDarkAngle.widget->setColourScheme(tealColorScheme);
	//e->parRotX.widget->setColourScheme(tealColorScheme);
	//e->parRotY.widget->setColourScheme(tealColorScheme);
	//e->parRotZ.widget->setColourScheme(tealColorScheme);
}

void JerobeamTorusEditor::rButtonClicked(jura::RButton* button)
{
	BasicEditorWithLicensing::rButtonClicked(button);
}

void JerobeamTorusEditor::updateWidgetsAccordingToState()
{
	AudioModuleEditor::updateWidgetsAccordingToState();
}

jura::AudioModuleEditor * JerobeamTorusModule::createEditor(int type)
{
	auto ptr = new JerobeamTorusEditor(this);

	ptr->setSize(ptr->initWidth, ptr->initHeight);

	ptr->updateWidgetsAccordingToState();

	ptr->BasicEditorWithLicensing::keyValidator.setProductIndex(se::KeyGenerator::productIndices::TORUSGENERATOR);
	ptr->BasicEditorWithLicensing::initializePlugIn();

	return ptr;
}

void JerobeamTorusEditor::resized()
{
	ScopedLock scopedLock(*lock);
	BasicEditorWithLicensing::resized();

	const int margin = 4;
	const int smallSliderHeight = 16;

	int w = int(round(getWidth()/6.0)*6 - 1);
	int h  = int(round(getHeight()/2.0)*2);
	auto & e = JerobeamTorusToEdit;

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

	// int bw; // button width

	// int x;

	infoField->setBounds(m, 442, w-12, sh);
	int webLinkWidth = jura::BitmapFontRoundedBoldA10D0::instance.getTextPixelWidth(webLink->getButtonText(), 1);
	webLink->setBounds(m, getHeight()-16+3, webLinkWidth, 16);

	///** COLUMN 1 **/
	//m = w3*0+4;
	//int y = getPresetSectionBottom()+8;

	///** COLUMN 2 **/
	//m = w3*1+4;
	//y = getPresetSectionBottom()+8;

	//{ //Torus controls
	//	TorusText.setBounds(m, y, sw, bth);
	//	y+=bth;
	//	x = m;
	//	TorusResetText.setBounds(x, y, 54, sh); x += 46;

	//	e->parResetMode.setBounds(x, y, 77, sh); x += 77+4;
	//	e->parCoreGain.setBounds(x, y, w-(8+x+w3), sh); y+=sh+mssp;

	//	int width = sw+6;

	//	e->parPhase.setBounds(m, y, width, sh); y+=sh+mssp;

	//	e->parSubPhase.setBounds(m, y, width, sh); y+=sh-2;
	//	e->parWanderSpd.setBounds(m, y, width, sh);

	//	y+=sh+mssp;

	//	e->parDensityQuantize.setBounds(m, y, 28, bsh);
	//	e->parDensity.setBounds(m+30, y, width-30, bsh); y+=bsh-2;

	//	e->parSubDensityQuantize.setBounds(m, y, 28, sh);
	//	e->parSDensity.setBounds(m+30, y, width-30, sh);

	//	y+=sh+mssp;

	//	e->parBalance.setBounds(m, y, width, sh);

	//	y+=sh+mssp;

	//	e->parSize.setBounds(m, y, width, sh); y+=sh-2;
	//	e->parLength.setBounds(m, y, width, sh); y+=sh-2;
	//	e->parSharp.setBounds(m, y, width, sh);

	//	y+=sh+mssp;

	//	e->parDarkIntensity.setBounds(m, y, width, sh); y+=sh-2;
	//	e->parDarkAngle.setBounds(m, y, width, sh); y+=sh-2;
	//	e->parDarkAngleSpd.setBounds(m, y, width, sh);

	//	y+=sh+mssp;

	//	e->parRotX.setBounds(m, y, width, sh); y+=sh-2;
	//	e->parRotXSpd.setBounds(m, y, width, sh);

	//	y+=sh+mssp;

	//	e->parRotY.setBounds(m, y, width, sh); y+=sh-2;
	//	e->parRotYSpd.setBounds(m, y, width, sh);

	//	y+=sh+mssp;

	//	e->parRotZ.setBounds(m, y, width, sh); y+=sh-2;
	//	e->parRotZSpd.setBounds(m, y, width, sh);

	//	y+=sh+mssp;

	//	e->parZAspectX.setBounds(m, y, width, sh); y+=sh-2;
	//	e->parZAspectY.setBounds(m, y, width, sh); y+=sh-2;
	//	e->parZDepth.setBounds(m, y, width, sh);
	//}
}
