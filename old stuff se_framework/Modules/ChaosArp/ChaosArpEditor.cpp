#include <map>
#include <vector>

#include "../../PrettyScope/Source/OpenGLOscilloscope.h"
#include "ChaosArpEditor.h"

#include "jura_framework/tools/jura_StringTools.h"

using namespace se;
using namespace lmd;
using namespace jura;
using namespace rosic;
using namespace juce;

/*
https://regex101.com/r/G9hsLr/1/
https://regex101.com/r/Dg8jrl/1/
*/

ChaosArp_EnvelopeEditor::ChaosArp_EnvelopeEditor(ChaosArp_EnvelopeModule * newModuleToEdit)
	: BasicEditor(newModuleToEdit)
	, moduleToEdit(newModuleToEdit)
{
	ScopedLock scopedLock(*lock);

	setWidgetAppearance(jura::ColourScheme::DARK_ON_BRIGHT);
	setPresetSectionPosition(AudioModuleEditor::positions::INVISIBLE);

	addWidget(bypass_button = new ModulatableButton());
	bypass_button->setButtonText("bypass");
	bypass_button->assignParameter(moduleToEdit->getParameterByName("bypass"));
	bypass_button->setName("bypass");
	bypass_button->setDescription("description");
	bypass_button->setDescriptionField(infoField);

	addWidget(amplitude_slider = new ModulatableSlider());
	amplitude_slider->assignParameter(moduleToEdit->getParameterByName("amplitude"));
	amplitude_slider->setName("amplitude");
	amplitude_slider->setDescription("description");
	amplitude_slider->setDescriptionField(infoField);
	amplitude_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(unipolar_button = new ModulatableButton());
	unipolar_button->setButtonText("unipolar");
	unipolar_button->assignParameter(moduleToEdit->getParameterByName("Unipolar_Enable"));
	unipolar_button->setName("unipolar");
	unipolar_button->setDescription("description");
	unipolar_button->setDescriptionField(infoField);

	addWidget(invert_button = new ModulatableButton());
	invert_button->setButtonText("invert");
	invert_button->assignParameter(moduleToEdit->getParameterByName("PhaseInvert"));
	invert_button->setName("invert");
	invert_button->setDescription("description");
	invert_button->setDescriptionField(infoField);

	addWidget(loop_button = new ModulatableButton());
	loop_button->setButtonText("loop");
	loop_button->assignParameter(moduleToEdit->getParameterByName("Loop"));
	loop_button->setName("loop");
	loop_button->setDescription("description");
	loop_button->setDescriptionField(infoField);

	addWidget(temposync_button = new ModulatableButton());
	temposync_button->setButtonText("temposync");
	temposync_button->assignParameter(moduleToEdit->getParameterByName("Beats_Enable"));
	temposync_button->setName("temposync");
	temposync_button->setDescription("description");
	temposync_button->setDescriptionField(infoField);

	addWidget(attack_slider = new ModulatableSlider());
	attack_slider->assignParameter(moduleToEdit->getParameterByName("Attack"));
	attack_slider->setName("attack");
	attack_slider->setDescription("description");
	attack_slider->setDescriptionField(infoField);
	attack_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(decay_slider = new ModulatableSlider());
	decay_slider->assignParameter(moduleToEdit->getParameterByName("Decay"));
	decay_slider->setName("decay");
	decay_slider->setDescription("description");
	decay_slider->setDescriptionField(infoField);
	decay_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(sustain_slider = new ModulatableSlider());
	sustain_slider->assignParameter(moduleToEdit->getParameterByName("Sustain"));
	sustain_slider->setName("sustain");
	sustain_slider->setDescription("description");
	sustain_slider->setDescriptionField(infoField);
	sustain_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(release_slider = new ModulatableSlider());
	release_slider->assignParameter(moduleToEdit->getParameterByName("Release"));
	release_slider->setName("release");
	release_slider->setDescription("description");
	release_slider->setDescriptionField(infoField);
	release_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(attackCurve = new ModulatableSlider());
	attackCurve->assignParameter(moduleToEdit->getParameterByName("AttackCurve"));
	attackCurve->setName("attackCurve");
	attackCurve->setDescription("description");
	attackCurve->setDescriptionField(infoField);
	attackCurve->setStringConversionFunction(&elan::StringFunc5);

	addWidget(decayCurve_slider = new ModulatableSlider());
	decayCurve_slider->assignParameter(moduleToEdit->getParameterByName("DecayCurve"));
	decayCurve_slider->setName("decayCurve");
	decayCurve_slider->setDescription("description");
	decayCurve_slider->setDescriptionField(infoField);
	decayCurve_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(releaseCurve_slider = new ModulatableSlider());
	releaseCurve_slider->assignParameter(moduleToEdit->getParameterByName("ReleaseCurve"));
	releaseCurve_slider->setName("releaseCurve");
	releaseCurve_slider->setDescription("description");
	releaseCurve_slider->setDescriptionField(infoField);
	releaseCurve_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(resetOnUserNote_button = new ModulatableButton());
	resetOnUserNote_button->setButtonText("resetOnUserNote");
	resetOnUserNote_button->assignParameter(moduleToEdit->getParameterByName("resetOnUserNote"));
	resetOnUserNote_button->setName("resetOnUserNote");
	resetOnUserNote_button->setDescription("description");
	resetOnUserNote_button->setDescriptionField(infoField);

	addWidget(resetOnArpNote_button = new ModulatableButton());
	resetOnArpNote_button->setButtonText("resetOnArpNote");
	resetOnArpNote_button->assignParameter(moduleToEdit->getParameterByName("resetOnArpNote"));
	resetOnArpNote_button->setName("resetOnArpNote");
	resetOnArpNote_button->setDescription("description");
	resetOnArpNote_button->setDescriptionField(infoField);

	addWidget(resetIfNotUserLegato_button = new ModulatableButton());
	resetIfNotUserLegato_button->setButtonText("resetIfNotUserLegato");
	resetIfNotUserLegato_button->assignParameter(moduleToEdit->getParameterByName("resetIfNotUserLegato"));
	resetIfNotUserLegato_button->setName("resetIfNotUserLegato");
	resetIfNotUserLegato_button->setDescription("description");
	resetIfNotUserLegato_button->setDescriptionField(infoField);

	addWidget(resetIfNotArpLegato_button = new ModulatableButton());
	resetIfNotArpLegato_button->setButtonText("resetIfNotArpLegato");
	resetIfNotArpLegato_button->assignParameter(moduleToEdit->getParameterByName("resetIfNotArpLegato"));
	resetIfNotArpLegato_button->setName("resetIfNotArpLegato");
	resetIfNotArpLegato_button->setDescription("description");
	resetIfNotArpLegato_button->setDescriptionField(infoField);

	addWidget(resetOnArpStep_button = new ModulatableButton());
	resetOnArpStep_button->setButtonText("resetOnArpStep");
	resetOnArpStep_button->assignParameter(moduleToEdit->getParameterByName("resetOnArpStep"));
	resetOnArpStep_button->setName("resetOnArpStep");
	resetOnArpStep_button->setDescription("description");
	resetOnArpStep_button->setDescriptionField(infoField);

	addWidget(resetOnArpPattern_button = new ModulatableButton());
	resetOnArpPattern_button->setButtonText("resetOnArpPattern");
	resetOnArpPattern_button->assignParameter(moduleToEdit->getParameterByName("resetOnArpPattern"));
	resetOnArpPattern_button->setName("resetOnArpPattern");
	resetOnArpPattern_button->setDescription("description");
	resetOnArpPattern_button->setDescriptionField(infoField);

	addWidget(triggerOnUserNote_button = new ModulatableButton());
	triggerOnUserNote_button->setButtonText("triggerOnUserNote");
	triggerOnUserNote_button->assignParameter(moduleToEdit->getParameterByName("triggerOnUserNote"));
	triggerOnUserNote_button->setName("triggerOnUserNote");
	triggerOnUserNote_button->setDescription("description");
	triggerOnUserNote_button->setDescriptionField(infoField);

	addWidget(triggerOnArpNote_button = new ModulatableButton());
	triggerOnArpNote_button->setButtonText("triggerOnArpNote");
	triggerOnArpNote_button->assignParameter(moduleToEdit->getParameterByName("triggerOnArpNote"));
	triggerOnArpNote_button->setName("triggerOnArpNote");
	triggerOnArpNote_button->setDescription("description");
	triggerOnArpNote_button->setDescriptionField(infoField);

	addWidget(triggerIfNotUserLegato_button = new ModulatableButton());
	triggerIfNotUserLegato_button->setButtonText("triggerIfNotUserLegato");
	triggerIfNotUserLegato_button->assignParameter(moduleToEdit->getParameterByName("triggerIfNotUserLegato"));
	triggerIfNotUserLegato_button->setName("triggerIfNotUserLegato");
	triggerIfNotUserLegato_button->setDescription("description");
	triggerIfNotUserLegato_button->setDescriptionField(infoField);

	addWidget(triggerIfNotArpLegato_button = new ModulatableButton());
	triggerIfNotArpLegato_button->setButtonText("triggerIfNotArpLegato");
	triggerIfNotArpLegato_button->assignParameter(moduleToEdit->getParameterByName("triggerIfNotArpLegato"));
	triggerIfNotArpLegato_button->setName("triggerIfNotArpLegato");
	triggerIfNotArpLegato_button->setDescription("description");
	triggerIfNotArpLegato_button->setDescriptionField(infoField);

	addWidget(triggerOnArpStep_button = new ModulatableButton());
	triggerOnArpStep_button->setButtonText("triggerOnArpStep");
	triggerOnArpStep_button->assignParameter(moduleToEdit->getParameterByName("triggerOnArpStep"));
	triggerOnArpStep_button->setName("triggerOnArpStep");
	triggerOnArpStep_button->setDescription("description");
	triggerOnArpStep_button->setDescriptionField(infoField);

	addWidget(triggerOnArpPattern_button = new ModulatableButton());
	triggerOnArpPattern_button->setButtonText("triggerOnArpPattern");
	triggerOnArpPattern_button->assignParameter(moduleToEdit->getParameterByName("triggerOnArpPattern"));
	triggerOnArpPattern_button->setName("triggerOnArpPattern");
	triggerOnArpPattern_button->setDescription("description");
	triggerOnArpPattern_button->setDescriptionField(infoField);

}

AudioModuleEditor * ChaosArp_EnvelopeModule::createEditor(int type)
{
	auto editor = new ChaosArp_EnvelopeEditor(this);
	return editor;
}

void ChaosArp_EnvelopeEditor::resized()
{
	BasicEditor::resized();

	int error = 48;

	{
		int x = 8;
		int y = getHeadlineBottom() + 8 - error;
		int w = getWidth() - 8*2;
		int h = 16;

		for (auto & widget : widgets)
		{
			widget->setBounds(x, y, w, h);

			y+=h;
		}
	}

}

void ChaosArp_EnvelopeEditor::paint(Graphics &g)
{
	AudioModuleEditor::paint(g);
}

//==================================================================================//

ChaosArp_LFOEditor::ChaosArp_LFOEditor(ChaosArp_LFOModule * newModuleToEdit)
	: BasicEditor(newModuleToEdit)
	, moduleToEdit(newModuleToEdit)
{
	ScopedLock scopedLock(*lock);

	setWidgetAppearance(jura::ColourScheme::DARK_ON_BRIGHT);
	setPresetSectionPosition(AudioModuleEditor::positions::INVISIBLE);

	addWidget(bypass_button = new ModulatableButton());
	bypass_button->setButtonText("bypass");
	bypass_button->assignParameter(moduleToEdit->getParameterByName("bypass"));
	bypass_button->setName("bypass");
	bypass_button->setDescription("description");
	bypass_button->setDescriptionField(infoField);

	addWidget(amplitude_slider = new ModulatableSlider());
	amplitude_slider->assignParameter(moduleToEdit->getParameterByName("amplitude"));
	amplitude_slider->setName("amplitude");
	amplitude_slider->setDescription("description");
	amplitude_slider->setDescriptionField(infoField);
	amplitude_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(phase_slider = new ModulatableSlider());
	phase_slider->assignParameter(moduleToEdit->getParameterByName("Phase"));
	phase_slider->setName("phase");
	phase_slider->setDescription("description");
	phase_slider->setDescriptionField(infoField);
	phase_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(unipolar_button = new ModulatableButton());
	unipolar_button->setButtonText("unipolar");
	unipolar_button->assignParameter(moduleToEdit->getParameterByName("Unipolar_Enable"));
	unipolar_button->setName("unipolar");
	unipolar_button->setDescription("description");
	unipolar_button->setDescriptionField(infoField);

	addWidget(invert_button = new ModulatableButton());
	invert_button->setButtonText("invert");
	invert_button->assignParameter(moduleToEdit->getParameterByName("PhaseInvert"));
	invert_button->setName("invert");
	invert_button->setDescription("description");
	invert_button->setDescriptionField(infoField);

	addWidget(division_slider = new ModulatableSlider());
	division_slider->assignParameter(moduleToEdit->getParameterByName("Tempo"));
	division_slider->setName("division");
	division_slider->setDescription("description");
	division_slider->setDescriptionField(infoField);
	division_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(frequency_slider = new ModulatableSlider());
	frequency_slider->assignParameter(moduleToEdit->getParameterByName("frequency"));
	frequency_slider->setName("frequency");
	frequency_slider->setDescription("description");
	frequency_slider->setDescriptionField(infoField);
	frequency_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(pitch_slider = new ModulatableSlider());
	pitch_slider->assignParameter(moduleToEdit->getParameterByName("Pitch"));
	pitch_slider->setName("pitch");
	pitch_slider->setDescription("description");
	pitch_slider->setDescriptionField(infoField);
	pitch_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(octave_slider = new ModulatableSlider());
	octave_slider->assignParameter(moduleToEdit->getParameterByName("Octave"));
	octave_slider->setName("octave");
	octave_slider->setDescription("description");
	octave_slider->setDescriptionField(infoField);
	octave_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(pitchEnable_button = new ModulatableButton());
	pitchEnable_button->setButtonText("pitchEnable");
	pitchEnable_button->assignParameter(moduleToEdit->getParameterByName("PitchEnable"));
	pitchEnable_button->setName("pitchEnable");
	pitchEnable_button->setDescription("description");
	pitchEnable_button->setDescriptionField(infoField);

	addWidget(randomFreq_slider = new ModulatableSlider());
	randomFreq_slider->assignParameter(moduleToEdit->getParameterByName("FreqRandom"));
	randomFreq_slider->setName("randomFreq");
	randomFreq_slider->setDescription("description");
	randomFreq_slider->setDescriptionField(infoField);
	randomFreq_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(randomAmp_slider = new ModulatableSlider());
	randomAmp_slider->assignParameter(moduleToEdit->getParameterByName("AmpRandom"));
	randomAmp_slider->setName("randomAmp");
	randomAmp_slider->setDescription("description");
	randomAmp_slider->setDescriptionField(infoField);
	randomAmp_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(waveBend_asym_slider = new ModulatableSlider());
	waveBend_asym_slider->assignParameter(moduleToEdit->getParameterByName("WaveBend_Asym"));
	waveBend_asym_slider->setName("waveBend_asym");
	waveBend_asym_slider->setSliderName("asym");
	waveBend_asym_slider->setDescription("description");
	waveBend_asym_slider->setDescriptionField(infoField);
	waveBend_asym_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(waveBend_attackBend_slider = new ModulatableSlider());
	waveBend_attackBend_slider->assignParameter(moduleToEdit->getParameterByName("WaveBend_AttackBend"));
	waveBend_attackBend_slider->setName("waveBend_attackBend");
	waveBend_attackBend_slider->setSliderName("attackBend");
	waveBend_attackBend_slider->setDescription("description");
	waveBend_attackBend_slider->setDescriptionField(infoField);
	waveBend_attackBend_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(waveBend_decayBend_slider = new ModulatableSlider());
	waveBend_decayBend_slider->assignParameter(moduleToEdit->getParameterByName("WaveBend_DecayBend"));
	waveBend_decayBend_slider->setName("waveBend_decayBend");
	waveBend_decayBend_slider->setSliderName("decayBend");
	waveBend_decayBend_slider->setDescription("description");
	waveBend_decayBend_slider->setDescriptionField(infoField);
	waveBend_decayBend_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(waveBend_attackSigmoid_slider = new ModulatableSlider());
	waveBend_attackSigmoid_slider->assignParameter(moduleToEdit->getParameterByName("WaveBend_AttackSgimoid"));
	waveBend_attackSigmoid_slider->setName("waveBend_attackSigmoid");
	waveBend_attackSigmoid_slider->setSliderName("attackSigmoid");
	waveBend_attackSigmoid_slider->setDescription("description");
	waveBend_attackSigmoid_slider->setDescriptionField(infoField);
	waveBend_attackSigmoid_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(waveBend_decaySigmoid_slider = new ModulatableSlider());
	waveBend_decaySigmoid_slider->assignParameter(moduleToEdit->getParameterByName("WaveBend_DecaySgimoid"));
	waveBend_decaySigmoid_slider->setName("waveBend_decaySigmoid");
	waveBend_decaySigmoid_slider->setSliderName("decaySigmoid");
	waveBend_decaySigmoid_slider->setDescription("description");
	waveBend_decaySigmoid_slider->setDescriptionField(infoField);
	waveBend_decaySigmoid_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(resetOnUserNote_button = new ModulatableButton());
	resetOnUserNote_button->setButtonText("resetOnUserNote");
	resetOnUserNote_button->assignParameter(moduleToEdit->getParameterByName("resetOnUserNote"));
	resetOnUserNote_button->setName("resetOnUserNote");
	resetOnUserNote_button->setDescription("description");
	resetOnUserNote_button->setDescriptionField(infoField);

	addWidget(resetOnArpNote_button = new ModulatableButton());
	resetOnArpNote_button->setButtonText("resetOnArpNote");
	resetOnArpNote_button->assignParameter(moduleToEdit->getParameterByName("resetOnArpNote"));
	resetOnArpNote_button->setName("resetOnArpNote");
	resetOnArpNote_button->setDescription("description");
	resetOnArpNote_button->setDescriptionField(infoField);

	addWidget(resetIfNotUserLegato_button = new ModulatableButton());
	resetIfNotUserLegato_button->setButtonText("resetIfNotUserLegato");
	resetIfNotUserLegato_button->assignParameter(moduleToEdit->getParameterByName("resetIfNotUserLegato"));
	resetIfNotUserLegato_button->setName("resetIfNotUserLegato");
	resetIfNotUserLegato_button->setDescription("description");
	resetIfNotUserLegato_button->setDescriptionField(infoField);

	addWidget(resetIfNotArpLegato_button = new ModulatableButton());
	resetIfNotArpLegato_button->setButtonText("resetIfNotArpLegato");
	resetIfNotArpLegato_button->assignParameter(moduleToEdit->getParameterByName("resetIfNotArpLegato"));
	resetIfNotArpLegato_button->setName("resetIfNotArpLegato");
	resetIfNotArpLegato_button->setDescription("description");
	resetIfNotArpLegato_button->setDescriptionField(infoField);

	addWidget(resetOnArpStep_button = new ModulatableButton());
	resetOnArpStep_button->setButtonText("resetOnArpStep");
	resetOnArpStep_button->assignParameter(moduleToEdit->getParameterByName("resetOnArpStep"));
	resetOnArpStep_button->setName("resetOnArpStep");
	resetOnArpStep_button->setDescription("description");
	resetOnArpStep_button->setDescriptionField(infoField);

	addWidget(resetOnArpPattern_button = new ModulatableButton());
	resetOnArpPattern_button->setButtonText("resetOnArpPattern");
	resetOnArpPattern_button->assignParameter(moduleToEdit->getParameterByName("resetOnArpPattern"));
	resetOnArpPattern_button->setName("resetOnArpPattern");
	resetOnArpPattern_button->setDescription("description");
	resetOnArpPattern_button->setDescriptionField(infoField);
}

AudioModuleEditor * ChaosArp_LFOModule::createEditor(int type)
{
	auto editor = new ChaosArp_LFOEditor(this);
	return editor;
}

void ChaosArp_LFOEditor::resized()
{
	BasicEditor::resized();

	int error = 48;

	{
		int x = 8;
		int y = getHeadlineBottom() + 8 - error;
		int w = getWidth()-8*2;
		int h = 16;

		for (auto & widget : widgets)
		{
			widget->setBounds(x, y, w, h);

			y+=h;
		}
	}

}

void ChaosArp_LFOEditor::paint(Graphics &g)
{
	AudioModuleEditor::paint(g);
}

//==================================================================================//

ChaosArp_ChaosOscEditor::ChaosArp_ChaosOscEditor(ChaosArp_ChaosOscModule * newModuleToEdit)
	: BasicEditor(newModuleToEdit)
	, moduleToEdit(newModuleToEdit)
{
	ScopedLock scopedLock(*lock);
	setWidgetAppearance(jura::ColourScheme::DARK_ON_BRIGHT);
	setPresetSectionPosition(AudioModuleEditor::positions::INVISIBLE);

	oscilloscopeEditor = new OscilloscopeEditor(moduleToEdit->oscilloscopeModule);
	addChildEditor(oscilloscopeEditor);
	oscilloscopeEditor->changeListenerCallback(nullptr);
	oscilloscopeEditor->addMouseListener(this, true);

	//addWidget(bypass_button = new ModulatableButton());
	//bypass_button->setButtonText("bypass");
	//bypass_button->assignParameter(moduleToEdit->getParameterByName("bypass"));
	//bypass_button->setName("bypass");
	//bypass_button->setDescription("description");
	//bypass_button->setDescriptionField(infoField);

	addWidget(amplitude_slider = new ModulatableSlider());
	amplitude_slider->assignParameter(moduleToEdit->getParameterByName("ArpSend"));
	amplitude_slider->setName("amplitude");
	amplitude_slider->setDescription("description");
	amplitude_slider->setDescriptionField(infoField);
	amplitude_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(unipolar_button = new ModulatableButton());
	unipolar_button->setButtonText("unipolar");
	unipolar_button->assignParameter(moduleToEdit->getParameterByName("ArpSend_isUnipolar"));
	unipolar_button->setName("unipolar");
	unipolar_button->setDescription("description");
	unipolar_button->setDescriptionField(infoField);

	addWidget(invert_button = new ModulatableButton());
	invert_button->setButtonText("invert");
	invert_button->assignParameter(moduleToEdit->getParameterByName("PhaseInvert"));
	invert_button->setName("invert");
	invert_button->setDescription("description");
	invert_button->setDescriptionField(infoField);

	addWidget(xRotate_slider = new ModulatableSlider());
	xRotate_slider->assignParameter(moduleToEdit->getParameterByName("rotateX"));
	xRotate_slider->setName("xRotate");
	xRotate_slider->setDescription("description");
	xRotate_slider->setDescriptionField(infoField);
	xRotate_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(yRotate_slider = new ModulatableSlider());
	yRotate_slider->assignParameter(moduleToEdit->getParameterByName("rotateY"));
	yRotate_slider->setName("yRotate");
	yRotate_slider->setDescription("description");
	yRotate_slider->setDescriptionField(infoField);
	yRotate_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(zRotate_slider = new ModulatableSlider());
	zRotate_slider->assignParameter(moduleToEdit->getParameterByName("rotateZ"));
	zRotate_slider->setName("zRotate");
	zRotate_slider->setDescription("description");
	zRotate_slider->setDescriptionField(infoField);
	zRotate_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(xOffset_slider = new ModulatableSlider());
	xOffset_slider->assignParameter(moduleToEdit->getParameterByName("offsetX"));
	xOffset_slider->setName("xOffset");
	xOffset_slider->setDescription("description");
	xOffset_slider->setDescriptionField(infoField);
	xOffset_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(yOffset_slider = new ModulatableSlider());
	yOffset_slider->assignParameter(moduleToEdit->getParameterByName("offsetY"));
	yOffset_slider->setName("yOffset");
	yOffset_slider->setDescription("description");
	yOffset_slider->setDescriptionField(infoField);
	yOffset_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(zOffset_slider = new ModulatableSlider());
	zOffset_slider->assignParameter(moduleToEdit->getParameterByName("offsetZ"));
	zOffset_slider->setName("zOffset");
	zOffset_slider->setDescription("description");
	zOffset_slider->setDescriptionField(infoField);
	zOffset_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(xyzOut_slider = new ModulatableSlider());
	//xyzOut_slider->assignParameter(moduleToEdit->getParameterByName("xyzOut"));
	xyzOut_slider->setName("xyzOut");
	xyzOut_slider->setDescription("description");
	xyzOut_slider->setDescriptionField(infoField);
	xyzOut_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(rho_slider = new ModulatableSlider());
	rho_slider->assignParameter(moduleToEdit->getParameterByName("setRho"));
	rho_slider->setName("rho");
	rho_slider->setDescription("description");
	rho_slider->setDescriptionField(infoField);
	rho_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(sigma_slider = new ModulatableSlider());
	sigma_slider->assignParameter(moduleToEdit->getParameterByName("setSigma"));
	sigma_slider->setName("sigma");
	sigma_slider->setDescription("description");
	sigma_slider->setDescriptionField(infoField);
	sigma_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(beta_slider = new ModulatableSlider());
	beta_slider->assignParameter(moduleToEdit->getParameterByName("setBeta"));
	beta_slider->setName("beta");
	beta_slider->setDescription("description");
	beta_slider->setDescriptionField(infoField);
	beta_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(lpcut_slider = new ModulatableSlider());
	lpcut_slider->assignParameter(moduleToEdit->getParameterByName("Filter_LP"));
	lpcut_slider->setName("lpcut");
	lpcut_slider->setDescription("description");
	lpcut_slider->setDescriptionField(infoField);
	lpcut_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(hpcut_slider = new ModulatableSlider());
	hpcut_slider->assignParameter(moduleToEdit->getParameterByName("Filter_HP"));
	hpcut_slider->setName("hpcut");
	hpcut_slider->setDescription("description");
	hpcut_slider->setDescriptionField(infoField);
	hpcut_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(filterEnable_button = new ModulatableButton());
	filterEnable_button->setButtonText("filter:on/off");
	filterEnable_button->assignParameter(moduleToEdit->getParameterByName("ArpSend_PostFilterEnable"));
	filterEnable_button->setName("invert");
	filterEnable_button->setDescription("description");
	filterEnable_button->setDescriptionField(infoField);

	addWidget(division_slider = new ModulatableSlider());
	division_slider->assignParameter(moduleToEdit->getParameterByName("Freq"));
	division_slider->setName("division");
	division_slider->setDescription("description");
	division_slider->setDescriptionField(infoField);
	division_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(resetOnUserNote_button = new ModulatableButton());
	resetOnUserNote_button->setButtonText("resetOnUserNote");
	resetOnUserNote_button->assignParameter(moduleToEdit->getParameterByName("resetOnUserNote"));
	resetOnUserNote_button->setName("resetOnUserNote");
	resetOnUserNote_button->setDescription("description");
	resetOnUserNote_button->setDescriptionField(infoField);

	addWidget(resetOnArpNote_button = new ModulatableButton());
	resetOnArpNote_button->setButtonText("resetOnArpNote");
	resetOnArpNote_button->assignParameter(moduleToEdit->getParameterByName("resetOnArpNote"));
	resetOnArpNote_button->setName("resetOnArpNote");
	resetOnArpNote_button->setDescription("description");
	resetOnArpNote_button->setDescriptionField(infoField);

	addWidget(resetIfNotUserLegato_button = new ModulatableButton());
	resetIfNotUserLegato_button->setButtonText("resetIfNotUserLegato");
	resetIfNotUserLegato_button->assignParameter(moduleToEdit->getParameterByName("resetIfNotUserLegato"));
	resetIfNotUserLegato_button->setName("resetIfNotUserLegato");
	resetIfNotUserLegato_button->setDescription("description");
	resetIfNotUserLegato_button->setDescriptionField(infoField);

	addWidget(resetIfNotArpLegato_button = new ModulatableButton());
	resetIfNotArpLegato_button->setButtonText("resetIfNotArpLegato");
	resetIfNotArpLegato_button->assignParameter(moduleToEdit->getParameterByName("resetIfNotArpLegato"));
	resetIfNotArpLegato_button->setName("resetIfNotArpLegato");
	resetIfNotArpLegato_button->setDescription("description");
	resetIfNotArpLegato_button->setDescriptionField(infoField);

	addWidget(resetOnArpStep_button = new ModulatableButton());
	resetOnArpStep_button->setButtonText("resetOnArpStep");
	resetOnArpStep_button->assignParameter(moduleToEdit->getParameterByName("resetOnArpStep"));
	resetOnArpStep_button->setName("resetOnArpStep");
	resetOnArpStep_button->setDescription("description");
	resetOnArpStep_button->setDescriptionField(infoField);

	addWidget(resetOnArpPattern_button = new ModulatableButton());
	resetOnArpPattern_button->setButtonText("resetOnArpPattern");
	resetOnArpPattern_button->assignParameter(moduleToEdit->getParameterByName("resetOnArpPattern"));
	resetOnArpPattern_button->setName("resetOnArpPattern");
	resetOnArpPattern_button->setDescription("description");
	resetOnArpPattern_button->setDescriptionField(infoField);

}

AudioModuleEditor * ChaosArp_ChaosOscModule::createEditor(int type)
{
	auto editor = new ChaosArp_ChaosOscEditor(this);
	return editor;
}

void ChaosArp_ChaosOscEditor::resized()
{
	BasicEditor::resized();

	int error = 48;
	
	int x = 8;
	int y = getHeadlineBottom() + 8 - error;
	int w = getWidth()-8*2;
	int h = 16;

	for (auto & widget : widgets)
	{
		widget->setBounds(x, y, w, h);

		y+=h;
	}

	oscilloscopeEditor->setBounds
	(
		widgets.back()->getX(),
		widgets.back()->getBottom() + 8,
		widgets.back()->getWidth(),
		widgets.back()->getWidth()
	);
}

void ChaosArp_ChaosOscEditor::paint(Graphics &g)
{
	AudioModuleEditor::paint(g);
}

ChaosArp_MainOscEditor::ChaosArp_MainOscEditor(ChaosArp_MainOscModule * newModuleToEdit)
	: BasicEditor(newModuleToEdit)
	, moduleToEdit(newModuleToEdit)
{
	ScopedLock scopedLock(*lock);

	setWidgetAppearance(jura::ColourScheme::DARK_ON_BRIGHT);
	setPresetSectionPosition(AudioModuleEditor::positions::INVISIBLE);

	addWidget(bypass_button = new ModulatableButton());
	bypass_button->setButtonText("bypass");
	bypass_button->assignParameter(moduleToEdit->getParameterByName("bypass"));
	bypass_button->setName("bypass");
	bypass_button->setDescription("description");
	bypass_button->setDescriptionField(infoField);

	addWidget(amplitude_slider = new ModulatableSlider());
	amplitude_slider->assignParameter(moduleToEdit->getParameterByName("amplitude"));
	amplitude_slider->setName("amplitude");
	amplitude_slider->setDescription("description");
	amplitude_slider->setDescriptionField(infoField);
	amplitude_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(phase_slider = new ModulatableSlider());
	phase_slider->assignParameter(moduleToEdit->getParameterByName("Phase"));
	phase_slider->setName("phase");
	phase_slider->setDescription("description");
	phase_slider->setDescriptionField(infoField);
	phase_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(frequency_slider = new ModulatableSlider());
	frequency_slider->assignParameter(moduleToEdit->getParameterByName("frequency"));
	frequency_slider->setName("frequency");
	frequency_slider->setDescription("description");
	frequency_slider->setDescriptionField(infoField);
	frequency_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(pitch_slider = new ModulatableSlider());
	pitch_slider->assignParameter(moduleToEdit->getParameterByName("Pitch"));
	pitch_slider->setName("pitch");
	pitch_slider->setDescription("description");
	pitch_slider->setDescriptionField(infoField);
	pitch_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(portamento_slider = new ModulatableSlider());
	//portamento_slider->assignParameter(moduleToEdit->getParameterByName("portamento"));
	portamento_slider->setName("portamento");
	portamento_slider->setDescription("description");
	portamento_slider->setDescriptionField(infoField);
	portamento_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(octave_slider = new ModulatableSlider());
	octave_slider->assignParameter(moduleToEdit->getParameterByName("Octave"));
	octave_slider->setName("octave");
	octave_slider->setDescription("description");
	octave_slider->setDescriptionField(infoField);
	octave_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(waveBend_asym_slider = new ModulatableSlider());
	waveBend_asym_slider->assignParameter(moduleToEdit->getParameterByName("WaveBend_Asym"));
	waveBend_asym_slider->setName("waveBend_asym");
	waveBend_asym_slider->setSliderName("saw -/+");
	waveBend_asym_slider->setDescription("description");
	waveBend_asym_slider->setDescriptionField(infoField);
	waveBend_asym_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(waveBend_bend_slider = new ModulatableSlider());
	waveBend_bend_slider->assignParameter(moduleToEdit->getParameterByName("WaveBend_Bend"));
	waveBend_bend_slider->setName("waveBend_asym_slider");
	waveBend_bend_slider->setSliderName("spike -/+");
	waveBend_bend_slider->setDescription("description");
	waveBend_bend_slider->setDescriptionField(infoField);
	waveBend_bend_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(waveBend_bendAsym_slider = new ModulatableSlider());
	waveBend_bendAsym_slider->assignParameter(moduleToEdit->getParameterByName("WaveBend_BendAsym"));
	waveBend_bendAsym_slider->setName("waveBend_bendAsym");
	waveBend_bendAsym_slider->setSliderName("square -/+");
	waveBend_bendAsym_slider->setDescription("description");
	waveBend_bendAsym_slider->setDescriptionField(infoField);
	waveBend_bendAsym_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(waveBend_sigmoid_slider = new ModulatableSlider());
	waveBend_sigmoid_slider->assignParameter(moduleToEdit->getParameterByName("WaveBend_Sigmoid"));
	waveBend_sigmoid_slider->setName("waveBend_sigmoid");
	waveBend_sigmoid_slider->setSliderName("sine -/+");
	waveBend_sigmoid_slider->setDescription("description");
	waveBend_sigmoid_slider->setDescriptionField(infoField);
	waveBend_sigmoid_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(waveBend_sigmoidAsym_slider = new ModulatableSlider());
	waveBend_sigmoidAsym_slider->assignParameter(moduleToEdit->getParameterByName("WaveBend_SigmoidAsym"));
	waveBend_sigmoidAsym_slider->setName("waveBend_sigmoidAsym");
	waveBend_sigmoidAsym_slider->setSliderName("knee -/+");
	waveBend_sigmoidAsym_slider->setDescription("description");
	waveBend_sigmoidAsym_slider->setDescriptionField(infoField);
	waveBend_sigmoidAsym_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(resetOnUserNote_button = new ModulatableButton());
	resetOnUserNote_button->setButtonText("resetOnUserNote");
	resetOnUserNote_button->assignParameter(moduleToEdit->getParameterByName("resetOnUserNote"));
	resetOnUserNote_button->setName("resetOnUserNote");
	resetOnUserNote_button->setDescription("description");
	resetOnUserNote_button->setDescriptionField(infoField);

	addWidget(resetOnArpNote_button = new ModulatableButton());
	resetOnArpNote_button->setButtonText("resetOnArpNote");
	resetOnArpNote_button->assignParameter(moduleToEdit->getParameterByName("resetOnArpNote"));
	resetOnArpNote_button->setName("resetOnArpNote");
	resetOnArpNote_button->setDescription("description");
	resetOnArpNote_button->setDescriptionField(infoField);

	addWidget(resetIfNotUserLegato_button = new ModulatableButton());
	resetIfNotUserLegato_button->setButtonText("resetIfNotUserLegato");
	resetIfNotUserLegato_button->assignParameter(moduleToEdit->getParameterByName("resetIfNotUserLegato"));
	resetIfNotUserLegato_button->setName("resetIfNotUserLegato");
	resetIfNotUserLegato_button->setDescription("description");
	resetIfNotUserLegato_button->setDescriptionField(infoField);

	addWidget(resetIfNotArpLegato_button = new ModulatableButton());
	resetIfNotArpLegato_button->setButtonText("resetIfNotArpLegato");
	resetIfNotArpLegato_button->assignParameter(moduleToEdit->getParameterByName("resetIfNotArpLegato"));
	resetIfNotArpLegato_button->setName("resetIfNotArpLegato");
	resetIfNotArpLegato_button->setDescription("description");
	resetIfNotArpLegato_button->setDescriptionField(infoField);

	addWidget(resetOnArpStep_button = new ModulatableButton());
	resetOnArpStep_button->setButtonText("resetOnArpStep");
	resetOnArpStep_button->assignParameter(moduleToEdit->getParameterByName("resetOnArpStep"));
	resetOnArpStep_button->setName("resetOnArpStep");
	resetOnArpStep_button->setDescription("description");
	resetOnArpStep_button->setDescriptionField(infoField);

	addWidget(resetOnArpPattern_button = new ModulatableButton());
	resetOnArpPattern_button->setButtonText("resetOnArpPattern");
	resetOnArpPattern_button->assignParameter(moduleToEdit->getParameterByName("resetOnArpPattern"));
	resetOnArpPattern_button->setName("resetOnArpPattern");
	resetOnArpPattern_button->setDescription("description");
	resetOnArpPattern_button->setDescriptionField(infoField);

}

AudioModuleEditor * ChaosArp_MainOscModule::createEditor(int type)
{
	auto editor = new ChaosArp_MainOscEditor(this);
	return editor;
}

void ChaosArp_MainOscEditor::resized()
{
	BasicEditor::resized();

	int error = 48;

	{
		int x = 8;
		int y = getHeadlineBottom() + 8 - error;
		int w = getWidth() - 8 * 2;
		int h = 16;

		for (auto & widget : widgets)
		{
			widget->setBounds(x, y, w, h);

			y += h;
		}
	}
}

void ChaosArp_MainOscEditor::paint(Graphics &g)
{
	AudioModuleEditor::paint(g);
}

//==================================================================================//

ChaosArp_ArpeggiatorEditor::ChaosArp_ArpeggiatorEditor(ChaosArp_ArpeggiatorModule * newModuleToEdit)
	: BasicEditor(newModuleToEdit)
	, moduleToEdit(newModuleToEdit)
{
	ScopedLock scopedLock(*lock);

	setWidgetAppearance(jura::ColourScheme::DARK_ON_BRIGHT);
	setPresetSectionPosition(AudioModuleEditor::positions::INVISIBLE);

	addWidget(bypass_button = new ModulatableButton());
	bypass_button->setButtonText("bypass");
	bypass_button->assignParameter(moduleToEdit->getParameterByName("Arp_Enable"));
	bypass_button->setName("bypass");
	bypass_button->setDescription("description");
	bypass_button->setDescriptionField(infoField);

	addWidget(stepDivision_slider = new ModulatableSlider());
	stepDivision_slider->assignParameter(moduleToEdit->getParameterByName("StepSize"));
	stepDivision_slider->setName("stepDivision");
	stepDivision_slider->setDescription("description");
	stepDivision_slider->setDescriptionField(infoField);
	stepDivision_slider->setStringConversionFunction(&elan::indexToTimeSigName);

	addWidget(noteDivision_slider = new ModulatableSlider());
	noteDivision_slider->assignParameter(moduleToEdit->getParameterByName("NoteQuantize"));
	noteDivision_slider->setName("noteLengthDivision");
	noteDivision_slider->setDescription("description");
	noteDivision_slider->setDescriptionField(infoField);
	noteDivision_slider->setStringConversionFunction(&elan::indexToTimeSigName);

	addWidget(lengthDivision_slider = new ModulatableSlider());
	lengthDivision_slider->assignParameter(moduleToEdit->getParameterByName("NoteLength"));
	lengthDivision_slider->setName("noteLengthDivision");
	lengthDivision_slider->setDescription("description");
	lengthDivision_slider->setDescriptionField(infoField);
	lengthDivision_slider->setStringConversionFunction(&elan::indexToTimeSigName);

	addWidget(lengthPercentage_slider = new ModulatableSlider());
	lengthPercentage_slider->assignParameter(moduleToEdit->getParameterByName("NoteLengthPercent"));
	lengthPercentage_slider->setName("noteLengthPercentage");
	lengthPercentage_slider->setDescription("description");
	lengthPercentage_slider->setDescriptionField(infoField);
	lengthPercentage_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(patternBeats_slider = new ModulatableSlider());
	patternBeats_slider->assignParameter(moduleToEdit->getParameterByName("PatternResetBeats"));
	patternBeats_slider->setName("patternBeats");
	patternBeats_slider->setDescription("description");
	patternBeats_slider->setDescriptionField(infoField);
	patternBeats_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(patternResetSpeedMultiplier_slider = new ModulatableSlider());
	patternResetSpeedMultiplier_slider->assignParameter(moduleToEdit->getParameterByName("patternResetSpeedMultiplier"));
	patternResetSpeedMultiplier_slider->setName("patternBeatsIntegerOnly");
	patternResetSpeedMultiplier_slider->setSliderName("^- speed multiply");
	patternResetSpeedMultiplier_slider->setDescription("description");
	patternResetSpeedMultiplier_slider->setDescriptionField(infoField);

	addWidget(pitchOffset_slider = new ModulatableSlider());
	pitchOffset_slider->assignParameter(moduleToEdit->getParameterByName("PitchOffset"));
	pitchOffset_slider->setName("pitchOffset");
	pitchOffset_slider->setDescription("description");
	pitchOffset_slider->setDescriptionField(infoField);
	pitchOffset_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(octavesAmplitude_slider = new ModulatableSlider());
	octavesAmplitude_slider->assignParameter(moduleToEdit->getParameterByName("OctavesAmplitude"));
	octavesAmplitude_slider->setName("octavesAmplitude");
	octavesAmplitude_slider->setDescription("description");
	octavesAmplitude_slider->setDescriptionField(infoField);
	octavesAmplitude_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(octavesOffset_slider = new ModulatableSlider());
	//octavesOffset_slider->assignParameter(moduleToEdit->getParameterByName("octavesOffset"));
	octavesOffset_slider->setName("octavesOffset");
	octavesOffset_slider->setDescription("description");
	octavesOffset_slider->setDescriptionField(infoField);
	octavesOffset_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(velocity_slider = new ModulatableSlider());
	velocity_slider->assignParameter(moduleToEdit->getParameterByName("Velocity"));
	velocity_slider->setName("velocity");
	velocity_slider->setDescription("description");
	velocity_slider->setDescriptionField(infoField);
	velocity_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(velocityIsFixed_button = new ModulatableButton());
	velocityIsFixed_button->setButtonText("velocityIsFixed");
	//velocityIsFixed_button->assignParameter(moduleToEdit->getParameterByName("velocityIsFixed"));
	velocityIsFixed_button->setName("velocityIsFixed");
	velocityIsFixed_button->setDescription("description");
	velocityIsFixed_button->setDescriptionField(infoField);
}

AudioModuleEditor * ChaosArp_ArpeggiatorModule::createEditor(int type)
{
	auto editor = new ChaosArp_ArpeggiatorEditor(this);
	return editor;
}

void ChaosArp_ArpeggiatorEditor::resized()
{
	BasicEditor::resized();

	int error = 48;

	{
		int x = 8;
		int y = getHeadlineBottom() + 8 - error;
		int w = getWidth() - 8 * 2;
		int h = 16;

		for (auto & widget : widgets)
		{
			widget->setBounds(x, y, w, h);

			y += h;
		}
	}

}

void ChaosArp_ArpeggiatorEditor::paint(Graphics &g)
{
	AudioModuleEditor::paint(g);
}

//==================================================================================//

ChaosArp_FilterEditor::ChaosArp_FilterEditor(ChaosArp_FilterModule * newModuleToEdit)
	: BasicEditor(newModuleToEdit)
	, moduleToEdit(newModuleToEdit)
{
	ScopedLock scopedLock(*lock);
	
	setWidgetAppearance(jura::ColourScheme::DARK_ON_BRIGHT);
	setPresetSectionPosition(AudioModuleEditor::positions::INVISIBLE);

	addWidget(bypass_button = new ModulatableButton());
	bypass_button->setButtonText("bypass");
	//bypass_button->assignParameter(moduleToEdit->getParameterByName("Filter_Bypass"));
	bypass_button->setName("bypass");
	bypass_button->setDescription("description");
	bypass_button->setDescriptionField(infoField);

	addWidget(inAmp_slider = new ModulatableSlider());
	//inAmp_slider->assignParameter(moduleToEdit->getParameterByName("InAmp"));
	inAmp_slider->setName("inAmp");
	inAmp_slider->setDescription("description");
	inAmp_slider->setDescriptionField(infoField);
	inAmp_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(outAmp_slider = new ModulatableSlider());
	outAmp_slider->assignParameter(moduleToEdit->getParameterByName("outAmp"));
	outAmp_slider->setName("outAmp");
	outAmp_slider->setDescription("description");
	outAmp_slider->setDescriptionField(infoField);
	outAmp_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(hpFrequency_slider = new ModulatableSlider());
	hpFrequency_slider->assignParameter(moduleToEdit->getParameterByName("hpFrequency"));
	hpFrequency_slider->setName("hpFrequency");
	hpFrequency_slider->setDescription("description");
	hpFrequency_slider->setDescriptionField(infoField);
	hpFrequency_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(hpResonance_slider = new ModulatableSlider());
	hpResonance_slider->assignParameter(moduleToEdit->getParameterByName("hpResonance"));
	hpResonance_slider->setName("hpResonance");
	hpResonance_slider->setDescription("description");
	hpResonance_slider->setDescriptionField(infoField);
	hpResonance_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(hpChaos_slider = new ModulatableSlider());
	hpChaos_slider->assignParameter(moduleToEdit->getParameterByName("hpChaos"));
	hpChaos_slider->setName("hpChaos");
	hpChaos_slider->setDescription("description");
	hpChaos_slider->setDescriptionField(infoField);
	hpChaos_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(lpFrequency_slider = new ModulatableSlider());
	lpFrequency_slider->assignParameter(moduleToEdit->getParameterByName("lpFrequency"));
	lpFrequency_slider->setName("lpFrequency");
	lpFrequency_slider->setDescription("description");
	lpFrequency_slider->setDescriptionField(infoField);
	lpFrequency_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(lpResonance_slider = new ModulatableSlider());
	lpResonance_slider->assignParameter(moduleToEdit->getParameterByName("lpResonance"));
	lpResonance_slider->setName("lpResonance");
	lpResonance_slider->setDescription("description");
	lpResonance_slider->setDescriptionField(infoField);
	lpResonance_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(lpChaos_slider = new ModulatableSlider());
	lpChaos_slider->assignParameter(moduleToEdit->getParameterByName("lpChaos"));
	lpChaos_slider->setName("lpChaos");
	lpChaos_slider->setDescription("description");
	lpChaos_slider->setDescriptionField(infoField);
	lpChaos_slider->setStringConversionFunction(&elan::StringFunc5);
}

AudioModuleEditor * ChaosArp_FilterModule::createEditor(int type)
{
	auto editor = new ChaosArp_FilterEditor(this);
	return editor;
}

void ChaosArp_FilterEditor::resized()
{
	BasicEditor::resized();

	int error = 48;

	{
		int x = 8;
		int y = getHeadlineBottom() + 8 - error;
		int w = getWidth() - 8 * 2;
		int h = 16;

		for (auto & widget : widgets)
		{
			widget->setBounds(x, y, w, h);

			y += h;
		}
	}
}

void ChaosArp_FilterEditor::paint(Graphics &g)
{
	AudioModuleEditor::paint(g);
}

//==================================================================================//

ChaosArpEditor::ChaosArpEditor(ChaosArpModule * newModuleToEdit)
	: BasicEditor(newModuleToEdit)
	, openGLCanvas(newModuleToEdit, &newModuleToEdit->audioBuffer, &newModuleToEdit->brushes)
	, moduleToEdit(newModuleToEdit)
{
	ScopedLock scopedLock(*lock);

	setPresetSectionPosition(AudioModuleEditor::positions::RIGHT_TO_HEADLINE);

	openGLCanvas.setScreenshotPath(moduleToEdit->screenshotPath);
	addAndMakeVisible(openGLCanvas);

	mainOscEditor = new ChaosArp_MainOscEditor(moduleToEdit->mainOscModule);
	mainOscEditor->setHeadlineText("Main Osc");
	addChildEditor(mainOscEditor);

	filterEditor = new ChaosArp_FilterEditor(moduleToEdit->filterModule);
	filterEditor->setHeadlineText("Filter");
	addChildEditor(filterEditor);

	arpeggiatorEditor = new ChaosArp_ArpeggiatorEditor(moduleToEdit->arpModule);
	arpeggiatorEditor->setHeadlineText("Arp");
	addChildEditor(arpeggiatorEditor);

	chaosOsc1Editor = new ChaosArp_ChaosOscEditor(moduleToEdit->chaosOsc1Module);
	chaosOsc1Editor->setHeadlineText("Chaos1");
	addChildEditor(chaosOsc1Editor);

	chaosOsc2Editor = new ChaosArp_ChaosOscEditor(moduleToEdit->chaosOsc2Module);
	chaosOsc2Editor->setHeadlineText("Chaos2");
	addChildEditor(chaosOsc2Editor);

	ampEnvEditor = new ChaosArp_EnvelopeEditor(moduleToEdit->ampEnvModule);
	ampEnvEditor->setHeadlineText("AmpEnv");
	addChildEditor(ampEnvEditor);

	filterEnvEditor = new ChaosArp_EnvelopeEditor(moduleToEdit->filterEnvModule);
	filterEnvEditor->setHeadlineText("FilterEnv");
	addChildEditor(filterEnvEditor);

	lfoEditor = new ChaosArp_LFOEditor(moduleToEdit->lfoModule);
	lfoEditor->setHeadlineText("LFO");
	addChildEditor(lfoEditor);

	addWidget(paramSmooth_slider = new ModulatableSlider());
	//paramSmooth_slider->assignParameter(moduleToEdit->getParameterByName("paramSmooth"));
	paramSmooth_slider->setName("paramSmooth");
	paramSmooth_slider->setDescription("description");
	paramSmooth_slider->setDescriptionField(infoField);
	paramSmooth_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(oversample_slider = new RSlider());
	//oversample_slider->assignParameter(moduleToEdit->getParameterByName("oversample"));
	oversample_slider->setName("oversample");
	oversample_slider->setDescription("description");
	oversample_slider->setDescriptionField(infoField);
	oversample_slider->setStringConversionFunction(&elan::StringFunc5);

	addWidget(midiOut_button = new RButton());
	midiOut_button->setButtonText("midiOut");
	//midiOut_button->assignParameter(moduleToEdit->getParameterByName("midiOut"));
	midiOut_button->setName("midiOut");
	midiOut_button->setDescription("description");
	midiOut_button->setDescriptionField(infoField);

	addWidget(allowInputSignal_button = new RButton());
	allowInputSignal_button->setButtonText("allowInputSignal");
	allowInputSignal_button->assignParameter(moduleToEdit->getParameterByName("allowInputSignal"));
	allowInputSignal_button->setName("allowInputSignal");
	allowInputSignal_button->setDescription("description");
	allowInputSignal_button->setDescriptionField(infoField);

	setSize(initialWidth, initialHeight);
}

ChaosArpEditor::~ChaosArpEditor()
{
}

void ChaosArpEditor::createWidgets()
{
}

AudioModuleEditor * ChaosArpModule::createEditor(int type)
{
	auto editor = new ChaosArpEditor(this);

	return editor;
}

void ChaosArpEditor::resized()
{
	AudioModuleEditor::resized();

	int w = std::max<float>(10, getWidth());
	int h = std::max<float>(10, getHeight());

	if (mainOscEditor)
	{
		if (h > 0)
		{
			const double screenAR = w / static_cast<double>(h);

			ignore_unused(screenAR);

			// We shouldn't call directly into audio processor
			//moduleToEdit->setScreenAspectRatio(screenAR);
		}
	}

	GlobalParameterText.setBounds(760+5, 12+10, 435-10, 46-20);

	int error = 48;
	int border = 2;
	int moduleWidth = 176;
	int margin = 8;
	
	int x = 8;
	int y = getHeadlineBottom() + 8 - error;
	w = moduleWidth;
	h = 16;

	for (auto & widget : widgets)
	{
		widget->setBounds(x, y, w, h);

		y+=h;
	}

	y = widgets.back()->getBottom() - border;
	mainOscEditor->setBounds(x, y, w, 316 + margin);

	y = mainOscEditor->getBottom() - border;
	arpeggiatorEditor->setBounds(x, y, w, 220 + margin);

	x = widgets[0]->getRight() - border;
	y = getHeadlineBottom() + 8;
	chaosOsc1Editor->setBounds(x, y, w, 548 + margin + 16*3);

	x = chaosOsc1Editor->getRight() - border;
	chaosOsc2Editor->setBounds(x, y, w, 548 + margin + 16 * 3);

	x = chaosOsc2Editor->getRight() - border;
	lfoEditor->setBounds(x, y, w, 460 + margin);

	x = lfoEditor->getRight() - border;
	ampEnvEditor->setBounds(x, y, w, 428 + margin);

	x = ampEnvEditor->getRight() - border;
	h = ampEnvEditor->getHeight();
	filterEnvEditor->setBounds(x, y, w, h);

	x = ampEnvEditor->getRight() - border;
	y = ampEnvEditor->getY();
	w = std::min(getWidth() - x - margin, 352);
	h = w;
	openGLCanvas.setSizeAndBounds(x, y, w, h);
	
}

void ChaosArpEditor::paint(Graphics &g)
{
	AudioModuleEditor::paint(g);
}
