#pragma once

using jura::RButton;
using jura::RRadioButton;
using jura::RRadioButtonGroup;
using jura::RTextField;

class JerobeamTorusEditor
	: public BasicEditorWithLicensing
	, PluginFileManager::FileMemory
{
	friend JerobeamTorusModule;

public:
	JerobeamTorusEditor(JerobeamTorusModule* newJerobeamTorusToEdit);

	virtual ~JerobeamTorusEditor()
	{
		JerobeamTorusToEdit->removeChangeListener(this);
	}

	virtual void createWidgets();

	// overriden callbacks
	virtual void resized() override;
	virtual void rButtonClicked(jura::RButton* button) override;
	virtual void updateWidgetsAccordingToState() override;

	RTextField TorusResetText{ "Reset:" };
	RTextField ToneModText{ "Tone Mod:" };
	RText TorusText{ "Torus" };

protected:
	int initWidth = 837;
	int initHeight = 764;

	JerobeamTorusModule* JerobeamTorusToEdit = nullptr;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(JerobeamTorusEditor)
};
