#pragma once

#include "JerobeamNyquistShannonModule.h"
#include "../../Source/BasicEnvelopePlugin/BasicEnvelopeEditor.h"
#include "../../Source/BasicOscillatorPlugin/BasicOscillatorEditor.h"
#include "../../Source/MidiModSourcePlugin/MidiModSourceEditor.h"

using jura::RButton;
using jura::RRadioButton;
using jura::RRadioButtonGroup;
using jura::RTextField;

class JerobeamNyquistShannonEditor : public BasicEditorWithLicensing
{
	friend JerobeamNyquistShannonModule;

public:
	JerobeamNyquistShannonEditor(JerobeamNyquistShannonModule *newJerobeamNyquistShannonToEdit);

	virtual ~JerobeamNyquistShannonEditor()
	{
		JerobeamNyquistShannonToEdit->removeChangeListener(this);
	}

	virtual void createWidgets();

	// overriden callbacks
	virtual void resized() override;
	virtual void rButtonClicked(jura::RButton* button) override;
	virtual void updateWidgetsAccordingToState() override;

	RTextField EnvResetText{ "Reset:" };
	RTextField FBSourceText{ "Source:" };
	RTextField BP1ResetText{ "Breakpoint 1 Reset:" };
	RTextField BP2ResetText{ "Breakpoint 2 Reset:" };
	RTextField StereoModeText{ "Mode:" };
	RTextField ClippingModeText{ "Clipping:" };

	RTextField NyquistShannonResetText{ "Reset:" };
	RTextField ToneModText{ "Tone Mod:" };

	RText StereoText{ "Stereo & Gain" };
	RText FilterText{ "Filter" };
	RText SettingsText{ "Settings" };

	RText NyquistShannonText{ "NyquistShannon" };

	RText FeedbackText{ "Feedback" };

	RText KeytrackingText{ "Keytracking" };
	RText TempoText{ "Tempo" };
	RText PitchText{ "Pitch" };
	RText AmpEnvText{ "Amp Env" };

	RText HelpText
	{ R"V0G0N(Modulation Routing: Right click on slider > Modulation setup > Add modulation source.

Host Automation: Right click on slider > Meta-attach > use number from 0 to 50. Parameters in your host automation will be labeled "Meta 01" or will be renamed to indicate which parameter is being controlled.

Double-click: Allows typing in a value
Alt + Click: JUMP slider to mouse
Ctrl/Cmd + Click: RESET
Mousewheel: TUNE /
Shift + Wheel: ULTRA-FINE tune
Shift + Drag: FINE tune

A note on Feedback: Feel free to explore the most nonsensical feedback routings you can think of. An LFO may modulate its own parameters. Setup a chain of multiple LFOs modulating eachother. Send one LFO to X Offset and another to Y Offset to invent your own 2D chaos generator. In addition to this, there is a dedicated feedback module on the main synth page. The best growls and screams will be had by modulating 'Frequency', 'Density', or 'Tune' with feedback. Then, create timbrally-dependent vibrato by modulating feedback amount with a sine LFO.\
)V0G0N" };

protected:
	int initWidth = 837;
	int initHeight = 764;

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

	/** Returns true if the passed button is one of our GUI page switch buttons. */
	bool isGuiPageButton(RButton* button) { return pageButtonGroup.isButtonMemberOfGroup(button); }

	// buttons for switching the GUI page:
	RRadioButton
		pageLFO_1_4{ "LFO 1 - 4" },
		pageLFO_5_8{"LFO 5 - 8"},
		pageADSR_1_3{ "ADSR 1 - 3" },
		pageADSR_4_6{ "ADSR 4 - 6" },
		pageBreakpoint1{ "Breakpoint 1" },
		pageBreakpoint2{ "Breakpoint 2" },
		pageMidiMod{ "Midi Source" },
		pageHelp{ "Help" },
		pageFX{ "FX" };

	RRadioButtonGroup pageButtonGroup;

	BasicEnvelopeEditor * AmpEnvEditor = nullptr;
	vector<BasicEnvelopeEditor *> BasicEnvelopeEditorArray{ 6 };
	vector<BasicOscillatorEditor *> BasicOscillatorEditorArray{ 8 };
	vector<MidiModSourceEditor *> MidiModSourceEditorArray{ 12 };
	jura::BreakpointModulatorEditor *Breakpoint1Editor, *Breakpoint2Editor;

	JerobeamNyquistShannonModule * JerobeamNyquistShannonToEdit = nullptr;

	EqualizerEditor * equalizerEditor = nullptr;
	LimiterEditor * limiterEditor = nullptr;
	BandpassEditor * bandpassEditor = nullptr;
	DelayEditor * delayEditor = nullptr;

	OscilloscopeEditor * oscilloscopeEditor = nullptr;
	Component oscilloscopeArea;

	ColourGradient gradientBlack = ColourGradient{ Colour(0, 0, 0), 0, 0, Colour(0, 255, 0), 1, 1, false };
	ColourGradient gradientWhite = ColourGradient{ Colour(255,255,255), 0, 0, Colour(51, 204, 51), 1, 1, false };

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(JerobeamNyquistShannonEditor)
};
