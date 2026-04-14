#pragma once

#include "ChaoticaModule.h"

class ChaoticaEditor : public jura::AudioModuleEditor
{
public:
	ChaoticaEditor(ChaoticaModule *newChaoticaToEdit);

	virtual ~ChaoticaEditor() = default;

  virtual void createWidgets();

  // overriden callbacks
  virtual void resized() override;
  virtual void rButtonClicked(jura::RButton* button) override;
  virtual void updateWidgetsAccordingToState() override;

protected:
	int initWidth = 685;
	int initHeight = 840;

	ChaoticaModule *ChaoticaToEdit;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChaoticaEditor)
};