#pragma once

#include "JerobeamSphereModule.h"
#include "../../Source/BasicEnvelopePlugin/BasicEnvelopeEditor.h"
#include "../../Source/BasicOscillatorPlugin/BasicOscillatorEditor.h"
#include "../../Source/MidiModSourcePlugin/MidiModSourceEditor.h"

using jura::RButton;
using jura::RRadioButton;
using jura::RRadioButtonGroup;
using jura::RTextField;

class JerobeamSphereEditor : public BasicEditor
{
	friend JerobeamSphereModule;

public:
	JerobeamSphereEditor(JerobeamSphereModule *newJerobeamSphereToEdit);

	virtual ~JerobeamSphereEditor() = default;

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

	RTextField SphereResetText{ "Reset:" };
	RTextField ToneModText{ "Tone Mod:" };

	RText StereoText{ "Stereo & Gain" };
	RText FilterText{ "Filter" };
	RText SettingsText{ "Settings" };

	RText SphereText{ "Sphere" };

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

	/** Returns true if the passed button is one of our GUI page switch buttons. */
	bool isGuiPageButton(RButton* button) { return pageButtonGroup.isButtonMemberOfGroup(button); }

	// buttons for switching the GUI page:
	RRadioButton
		*pageLFO_1_4,
		*pageLFO_5_8,
		*pageADSR_1_3,
		*pageADSR_4_6,
		*pageBreakpoint1,
		*pageBreakpoint2,
		*pageMidiMod,
		*pageHelp;

	RRadioButtonGroup pageButtonGroup;

	BasicEnvelopeEditor * AmpEnvEditor = nullptr;
	vector<BasicEnvelopeEditor *> BasicEnvelopeEditorArray{ 6 };
	vector<BasicOscillatorEditor *> BasicOscillatorEditorArray{ 8 };
	vector<MidiModSourceEditor *> MidiModSourceEditorArray{ 12 };
	jura::BreakpointModulatorEditor *Breakpoint1Editor, *Breakpoint2Editor;

	JerobeamSphereModule * JerobeamSphereToEdit = nullptr;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(JerobeamSphereEditor)
};