#include "VectorSlider.h"

using namespace jura;

ButtonWithCustomEnableColor buttonEnableColorRed{ 255, 147, 147 };
ButtonWithCustomEnableColor buttonEnableColorGreen{ 160, 253, 154 };

AutomatableClickButton::AutomatableClickButton(const juce::String& buttonText)
	: RClickButton(buttonText), rsAutomatableWidget(this)
{

}

void AutomatableClickButton::mouseDown(const MouseEvent& e)
{
	if (e.mods.isRightButtonDown())
	{
		if (!isPopUpOpen())
			openRightClickPopupMenu();
		else
			closePopUp();
	}
	else
		RClickButton::mouseDown(e);
}

void AutomatableClickButton::parameterChanged(Parameter* p)
{
	RWidget::parameterChanged(p);
}

//=================================================================================================

ElanModulatableButton::ElanModulatableButton(const juce::String& buttonText) : rsAutomatableButton(buttonText)
{
}

ElanModulatableButton::~ElanModulatableButton()
{
	ModulatableParameter* mp = dynamic_cast<ModulatableParameter*> (assignedParameter);
	if (mp)
		mp->deRegisterModulationTargetObserver(this);
}

void ElanModulatableButton::modulationsChanged()
{
	repaint();
}

void ElanModulatableButton::assignParameter(Parameter* p)
{
	rsAutomatableButton::assignParameter(p);
	ModulatableParameter* mp = dynamic_cast<ModulatableParameter*> (p);
	if (mp)
		mp->registerModulationTargetObserver(this);
}

void ElanModulatableButton::paint(Graphics& g)
{
	rsAutomatableButton::paint(g);
	ModulatableParameter* mp = dynamic_cast<ModulatableParameter*> (assignedParameter);
	if (mp && mp->hasConnectedSources())
		g.fillAll(Colour::fromFloatRGBA(1.f, 0.f, 0.f, 0.125f)); // preliminary
}

//=================================================================================================

ModulatableClickButton::ModulatableClickButton(const juce::String& buttonText) : AutomatableClickButton(buttonText)
{
}

ModulatableClickButton::~ModulatableClickButton()
{
	ModulatableParameter* mp = dynamic_cast<ModulatableParameter*> (assignedParameter);
	if (mp)
		mp->deRegisterModulationTargetObserver(this);
}

void ModulatableClickButton::modulationsChanged()
{
	repaint();
}

void ModulatableClickButton::assignParameter(Parameter* p)
{
	AutomatableClickButton::assignParameter(p);
	ModulatableParameter* mp = dynamic_cast<ModulatableParameter*> (p);
	if (mp)
		mp->registerModulationTargetObserver(this);
}

void ModulatableClickButton::paint(Graphics& g)
{
	AutomatableClickButton::paint(g);
	ModulatableParameter* mp = dynamic_cast<ModulatableParameter*> (assignedParameter);
	if (mp && mp->hasConnectedSources())
		g.fillAll(Colour::fromFloatRGBA(1.f, 0.f, 0.f, 0.125f)); // preliminary
}

//=================================================================================================

ElanModulatableComboBox::~ElanModulatableComboBox()
{
	ModulatableParameter* mp = dynamic_cast<ModulatableParameter*> (assignedParameter);
	if (mp)
		mp->deRegisterModulationTargetObserver(this);
}

void ElanModulatableComboBox::modulationsChanged()
{
	repaint();
}

void ElanModulatableComboBox::assignParameter(Parameter* p)
{
	rsAutomatableComboBox::assignParameter(p);
	ModulatableParameter* mp = dynamic_cast<ModulatableParameter*> (p);
	if (mp)
		mp->registerModulationTargetObserver(this);
}

void ElanModulatableComboBox::paint(Graphics& g)
{
	rsAutomatableComboBox::paint(g);
	ModulatableParameter* mp = dynamic_cast<ModulatableParameter*> (assignedParameter);
	if (mp && mp->hasConnectedSources())
		g.fillAll(Colour::fromFloatRGBA(1.f, 0.f, 0.f, 0.5f)); // preliminary
}

PopupMenu createRightClickMenu(jura::rsAutomatableWidget * widget)
{
	PopupMenu mainMenu;
	PopupMenu metaParameterLinearMenu;
	PopupMenu metaParameterFlatMenu;

	MetaControlledParameter* mcp = widget->getMetaControlledParameter();
	String miString;
	if (mcp != nullptr)
	{
		// menu item to show attached metaparameter
		int mi = mcp->getMetaParameterIndex();
		if (mi > -1)
			miString = "Attached to: " + mcp->getMetaParameterName();
		else
			miString = "Attached to: none";
		MenuHelper::addGreyedOutItem(mainMenu, miString);

		// seperator
		MenuHelper::addSeperator(mainMenu);

		// submenu for attach to metaparameter linear
		for (int i = 0; i < widget->getMetaParameterManager()->getNumMetaParameters(); ++i)
		{
			MenuHelper::addCallbackItem(metaParameterLinearMenu, widget->getMetaParameterManager()->getMetaParameterName(i), [widget, i]()
			{
				widget->getMetaControlledParameter()->attachToMetaParameter(i, false);
			});
		}

		// submenu for attach to metaparameter flat
		for (int i = 0; i < widget->getMetaParameterManager()->getNumMetaParameters(); ++i)
		{
			MenuHelper::addCallbackItem(metaParameterFlatMenu, widget->getMetaParameterManager()->getMetaParameterName(i), [widget, i]()
			{
				widget->getMetaControlledParameter()->attachToMetaParameter(i, true);
			});
		}

		MenuHelper::addSubMenu(mainMenu, "Attach Meta Parameter (linear)", metaParameterLinearMenu);

		MenuHelper::addSubMenu(mainMenu, "Attach Meta Parameter (flat)", metaParameterFlatMenu);

		// menu item for detach parameter
		MenuHelper::addCallbackItem(mainMenu, "Detach meta parameter", [widget]()
		{
			widget->getMetaControlledParameter()->detachFromMetaParameter();
		});
	}

	MenuHelper::addCallbackItem(mainMenu, "Automation setup", [widget]()
	{
		widget->showAutomationSetup();
	});

	MenuHelper::addCallbackItem(mainMenu, "Modulation setup", [widget]()
	{
		widget->showModulationSetup();
	});

	return mainMenu;
}
