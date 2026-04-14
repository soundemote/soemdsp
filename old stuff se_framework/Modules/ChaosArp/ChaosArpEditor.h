#pragma once

#include <map>

#include "utils/lmdsp.hpp"

#include "ChaosArpModule.h"

class JUCE_API ChaosArp_EnvelopeEditor : public BasicEditor
{
public:

	ChaosArp_EnvelopeEditor(ChaosArp_EnvelopeModule * newModuleToEdit);
	virtual ~ChaosArp_EnvelopeEditor() = default;

	void resized();
	void paint(Graphics& g);

	ChaosArp_EnvelopeModule * moduleToEdit;

protected:

	ModulatableButton * bypass_button;
	ModulatableSlider * amplitude_slider;
	ModulatableButton * unipolar_button;
	ModulatableButton * invert_button;

	ModulatableButton * loop_button;
	ModulatableButton * temposync_button;

	ModulatableSlider * attack_slider;
	ModulatableSlider * decay_slider;
	ModulatableSlider * sustain_slider;
	ModulatableSlider * release_slider;

	ModulatableSlider * attackCurve;
	ModulatableSlider * decayCurve_slider;
	ModulatableSlider * releaseCurve_slider;

	ModulatableButton * resetOnUserNote_button;
	ModulatableButton * resetOnArpNote_button;
	ModulatableButton * resetIfNotUserLegato_button;
	ModulatableButton * resetIfNotArpLegato_button;
	ModulatableButton * resetOnArpStep_button;
	ModulatableButton * resetOnArpPattern_button;

	ModulatableButton * triggerOnUserNote_button;
	ModulatableButton * triggerOnArpNote_button;
	ModulatableButton * triggerIfNotUserLegato_button;
	ModulatableButton * triggerIfNotArpLegato_button;
	ModulatableButton * triggerOnArpStep_button;
	ModulatableButton * triggerOnArpPattern_button;
};

class JUCE_API ChaosArp_LFOEditor : public BasicEditor
{
public:

	ChaosArp_LFOEditor(ChaosArp_LFOModule * newModuleToEdit);
	virtual ~ChaosArp_LFOEditor() = default;

	void resized() override;
	void paint(Graphics& g) override;

	ChaosArp_LFOModule * moduleToEdit;

protected:

	ModulatableButton * bypass_button;
	ModulatableSlider * amplitude_slider;
	ModulatableSlider * phase_slider;
	ModulatableButton * unipolar_button;
	ModulatableButton * invert_button;

	ModulatableSlider * division_slider;
	ModulatableSlider * frequency_slider;
	ModulatableSlider * pitch_slider;
	ModulatableSlider * octave_slider;
	ModulatableButton * pitchEnable_button;

	ModulatableSlider * randomFreq_slider;
	ModulatableSlider * randomAmp_slider;

	ModulatableSlider * samples_slider;
	ModulatableSlider * bits_slider;

	ModulatableSlider * waveBend_asym_slider;
	ModulatableSlider * waveBend_attackBend_slider;
	ModulatableSlider * waveBend_decayBend_slider;
	ModulatableSlider * waveBend_attackSigmoid_slider;
	ModulatableSlider * waveBend_decaySigmoid_slider;

	ModulatableButton * resetOnUserNote_button;
	ModulatableButton * resetOnArpNote_button;
	ModulatableButton * resetIfNotUserLegato_button;
	ModulatableButton * resetIfNotArpLegato_button;
	ModulatableButton * resetOnArpStep_button;
	ModulatableButton * resetOnArpPattern_button;
};

class JUCE_API ChaosArp_ChaosOscEditor : public BasicEditor
{
public:

	ChaosArp_ChaosOscEditor(ChaosArp_ChaosOscModule * newModuletoEdit);
	virtual ~ChaosArp_ChaosOscEditor() = default;

	void resized() override;
	void paint(Graphics& g) override;

	void mouseEnter(const MouseEvent& event) override
	{
		if (oscilloscopeEditor->isMouseOverOrDragging(true))
			oscilloscopeEditor->setWidgetsToHideForMouseover(true);
		else
			oscilloscopeEditor->setWidgetsToHideForMouseover(false);
	}

	void mouseExit(const MouseEvent& event) override
	{
		if (!oscilloscopeEditor->isMouseOverOrDragging(true))
			oscilloscopeEditor->setWidgetsToHideForMouseover(false);
	}

	void mouseUp(const MouseEvent &event) override
	{
		if (!oscilloscopeEditor->isMouseOverOrDragging(true))
			oscilloscopeEditor->setWidgetsToHideForMouseover(false);
	}

	void mouseDrag(const MouseEvent &event) override
	{
		if (!oscilloscopeEditor->isMouseOverOrDragging(true))
			oscilloscopeEditor->setWidgetsToHideForMouseover(false);
	}

	ChaosArp_ChaosOscModule * moduleToEdit;
	OscilloscopeEditor * oscilloscopeEditor;
	Component oscilloscopeArea;

protected:

	ModulatableButton * bypass_button;
	ModulatableSlider * amplitude_slider;
	ModulatableButton * unipolar_button;
	ModulatableButton * invert_button;

	ModulatableSlider * xRotate_slider;
	ModulatableSlider * yRotate_slider;
	ModulatableSlider * zRotate_slider;

	ModulatableSlider * xOffset_slider;
	ModulatableSlider * yOffset_slider;
	ModulatableSlider * zOffset_slider;

	ModulatableSlider * xyzOut_slider;

	ModulatableSlider * rho_slider;
	ModulatableSlider * sigma_slider;
	ModulatableSlider * beta_slider;

	ModulatableSlider * lpcut_slider;
	ModulatableSlider * hpcut_slider;
	ModulatableButton * filterEnable_button;

	ModulatableSlider * division_slider;

	ModulatableButton * resetOnUserNote_button;
	ModulatableButton * resetOnArpNote_button;
	ModulatableButton * resetIfNotUserLegato_button;
	ModulatableButton * resetIfNotArpLegato_button;
	ModulatableButton * resetOnArpStep_button;
	ModulatableButton * resetOnArpPattern_button;
};

class JUCE_API ChaosArp_MainOscEditor : public BasicEditor
{
public:

	ChaosArp_MainOscEditor(ChaosArp_MainOscModule * newModuletoEdit);
	virtual ~ChaosArp_MainOscEditor() = default;

	void resized() override;
	void paint(Graphics& g) override;

	ChaosArp_MainOscModule * moduleToEdit;

protected:

	ModulatableButton * bypass_button;
	ModulatableSlider * amplitude_slider;
	ModulatableSlider * phase_slider;

	ModulatableSlider * frequency_slider;
	ModulatableSlider * pitch_slider;
	ModulatableSlider * portamento_slider;
	ModulatableSlider * octave_slider;

	ModulatableSlider * waveBend_asym_slider;
	ModulatableSlider * waveBend_bend_slider;
	ModulatableSlider * waveBend_bendAsym_slider;
	ModulatableSlider * waveBend_sigmoid_slider;
	ModulatableSlider * waveBend_sigmoidAsym_slider;

	ModulatableButton * resetOnUserNote_button;
	ModulatableButton * resetOnArpNote_button;
	ModulatableButton * resetIfNotUserLegato_button;
	ModulatableButton * resetIfNotArpLegato_button;
	ModulatableButton * resetOnArpStep_button;
	ModulatableButton * resetOnArpPattern_button;
};

class JUCE_API ChaosArp_ArpeggiatorEditor : public BasicEditor
{
public:

	ChaosArp_ArpeggiatorEditor(ChaosArp_ArpeggiatorModule * newModuletoEdit);
	virtual ~ChaosArp_ArpeggiatorEditor() = default;

	void resized() override;
	void paint(Graphics& g) override;

	ChaosArp_ArpeggiatorModule * moduleToEdit;

protected:

	ModulatableButton * bypass_button;

	ModulatableSlider * stepDivision_slider;
	ModulatableSlider * noteDivision_slider;
	ModulatableSlider * lengthDivision_slider;
	ModulatableSlider * lengthPercentage_slider;

	ModulatableSlider * patternBeats_slider;
	ModulatableSlider * patternResetSpeedMultiplier_slider;

	ModulatableSlider * pitchOffset_slider;

	ModulatableSlider * octavesAmplitude_slider;
	ModulatableSlider * octavesOffset_slider;

	ModulatableSlider * velocity_slider;
	ModulatableButton * velocityIsFixed_button;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChaosArp_ArpeggiatorEditor);
};


class JUCE_API ChaosArp_FilterEditor : public BasicEditor
{
public:

	ChaosArp_FilterEditor(ChaosArp_FilterModule * newModuletoEdit);
	virtual ~ChaosArp_FilterEditor() = default;

	void resized() override;
	void paint(Graphics& g) override;

	ChaosArp_FilterModule * moduleToEdit;

protected:

	// Midi Generator
	ModulatableButton * bypass_button;

	ModulatableSlider * inAmp_slider;
	ModulatableSlider * outAmp_slider;

	ModulatableSlider * hpFrequency_slider;
	ModulatableSlider * hpResonance_slider;
	ModulatableSlider * hpChaos_slider;

	ModulatableSlider * lpFrequency_slider;
	ModulatableSlider * lpResonance_slider;
	ModulatableSlider * lpChaos_slider;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChaosArp_FilterEditor);
};

enum brush { line1, dot1, line2, dot2 };

class JUCE_API ChaosArpEditor : public BasicEditor
{
public:

	ChaosArpEditor(ChaosArpModule * moduleToEdit);
	virtual ~ChaosArpEditor();

	void createWidgets();

	void resized() override;
	void paint(Graphics& g) override;

	//RImage background{ BinaryData::background_png, BinaryData::background_pngSize };

	RText GlobalParameterText;

	bool userClickedClear = false;


	void mouseDown(const MouseEvent& event) override
	{
		GlobalParameterText.setText(getDisplayTextForParameterFromMouseEvent(event));
	}

	void mouseUp(const MouseEvent& event) override
	{
		GlobalParameterText.setText("");
	}

	void mouseDrag(const MouseEvent& event) override
	{
		GlobalParameterText.setText(getDisplayTextForParameterFromMouseEvent(event));
	}

	String getDisplayTextForParameterFromMouseEvent(const MouseEvent& event)
	{
		auto widget = dynamic_cast<jura::RWidget *>(event.eventComponent);

		if (widget == nullptr)
			return "";

		return widget->getName() + " : " + widget->getValueDisplayString();
	}

	OpenGLCanvas openGLCanvas;

	ChaosArpModule * moduleToEdit;

	ChaosArp_MainOscEditor * mainOscEditor;
	ChaosArp_FilterEditor * filterEditor;
	ChaosArp_ArpeggiatorEditor * arpeggiatorEditor;
	ChaosArp_ChaosOscEditor * chaosOsc1Editor;
	ChaosArp_ChaosOscEditor * chaosOsc2Editor;
	ChaosArp_EnvelopeEditor * ampEnvEditor;
	ChaosArp_EnvelopeEditor * filterEnvEditor;
	ChaosArp_LFOEditor * lfoEditor;

protected:

	int initialWidth = 1280;
	int initialHeight = 890;

	// Main
	ModulatableSlider * paramSmooth_slider;
	RSlider * oversample_slider;

	RButton * midiOut_button;
	RButton * allowInputSignal_button;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChaosArpEditor);
};
