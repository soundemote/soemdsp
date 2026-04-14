#pragma once

#include "jura_framework/gui/widgets/jura_RButton.h"
#include "jura_framework/gui/widgets/jura_AutomatableWidget.h"

PopupMenu createRightClickMenu(jura::rsAutomatableWidget * widget);

class ButtonWithCustomEnableColor : public jura::RButtonPainter
{
public:
	ButtonWithCustomEnableColor(int r, int g, int b)
	{
		enableColor = Colour{ (uint8)r, (uint8)g, (uint8)b };
	}

	void paint(Graphics& g, jura::RButton* b) override
	{
		if (b->getToggleState())
			g.fillAll(enableColor);
		else
			g.fillAll(b->getBackgroundColour());

		g.setColour(b->getOutlineColour());
		g.drawRect(0, 0, b->getWidth(), b->getHeight(), 2);
		g.setColour(b->getTextColour());

		int x = b->getWidth()/2 - b->getFont()->getTextPixelWidth(b->getButtonText(), b->getFont()->getDefaultKerning()) / 2;
		int y = b->getHeight()/2 - b->getFont()->getFontAscent()/2;
		jura::drawBitmapFontText(g, x, y, b->getButtonText(), b->getFont(), b->getTextColour());
	}

protected:
	Colour enableColor;
};

extern ButtonWithCustomEnableColor buttonEnableColorRed;
extern ButtonWithCustomEnableColor buttonEnableColorGreen;

class ButtonWithCustomColor : public jura::RButtonPainter
{
public:
	ButtonWithCustomColor() = default;
	virtual ~ButtonWithCustomColor() = default;

	void setEnableColor(int r, int g, int b) { enableColor = Colour{ (uint8)r, (uint8)g, (uint8)b }; }
	void setDisableColor(int r, int g, int b) { disableColor = Colour{ (uint8)r, (uint8)g, (uint8)b }; }

	void paint(Graphics& g, jura::RButton* b) override
	{
		if (b->getToggleState())
			g.fillAll(enableColor);
		else
			g.fillAll(disableColor);

		g.setColour(b->getOutlineColour());
		g.drawRect(0, 0, b->getWidth(), b->getHeight(), 2);
		g.setColour(b->getTextColour());

		int x = b->getWidth()/2 - b->getFont()->getTextPixelWidth(b->getButtonText(), b->getFont()->getDefaultKerning()) / 2;
		int y = b->getHeight()/2 - b->getFont()->getFontAscent()/2;
		jura::drawBitmapFontText(g, x, y, b->getButtonText(), b->getFont(), b->getTextColour());
	}

protected:
	Colour enableColor;
	Colour disableColor;
};

class JUCE_API AutomatableClickButton : public jura::RClickButton, public rsAutomatableWidget
{
public:
  AutomatableClickButton(const juce::String& buttonText = {});
	virtual void mouseDown(const MouseEvent& e) override;
	virtual void parameterChanged(jura::Parameter* p) override;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AutomatableClickButton)
};


class JUCE_API ElanModulatableButton
	: public rsAutomatableButton
	, public jura::ModulationTargetObserver
	, public ParameterInfo::InfoSender
{
public:
  ElanModulatableButton(const juce::String& buttonText = {});
	virtual ~ElanModulatableButton();
	void assignParameter(jura::Parameter* parameterToAssign) override;

	void setRightClickMenuEnabled(bool v)
	{
		rightClickMenuIsEnabled = v;
	}

protected:
	bool rightClickMenuIsEnabled = true;

	virtual void paint(Graphics& g) override;

	void modulationsChanged() override;

	virtual void mouseEnter(const MouseEvent&) override
	{
		ParameterInfo::InfoSender::updateParameterInfo();
	}

	virtual void mouseDown(const MouseEvent& e) override
	{
		if (e.mods.isRightButtonDown())
		{
			if (rightClickMenuIsEnabled)
      {
        auto m = createRightClickMenu(this);
        MenuHelper::show(&m, this);
      }
		}
		else
		{
			rsAutomatableButton::mouseDown(e);
		}

		ParameterInfo::InfoSender::updateParameterInfo();
	}

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ElanModulatableButton)
};

class JUCE_API ModulatableClickButton : public AutomatableClickButton, public jura::ModulationTargetObserver
{
public:
  ModulatableClickButton(const juce::String& buttonText = {});
	virtual ~ModulatableClickButton();
	void modulationsChanged() override;
	void assignParameter(jura::Parameter* parameterToAssign) override;
  void paint(Graphics& g) override;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModulatableClickButton)
};

class JUCE_API ElanModulatableComboBox
	: public rsAutomatableComboBox
	, public jura::ModulationTargetObserver
	, public ParameterInfo::InfoSender
{
public:
	ElanModulatableComboBox()
	{
		// setup defualt function for sender
		ParameterInfo::InfoSender::valueStringFunction = [this]()
		{
			return String(getAssignedParameter()->getValue(), 0) + " : " + getSelectedItemText();
		};
	}
	virtual ~ElanModulatableComboBox();

	void modulationsChanged() override;

	void assignParameter(jura::Parameter* parameterToAssign) override;

	void setRightClickMenuEnabled(bool v)
	{
		rightClickMenuIsEnabled = v;
	}

	void setSkin(LookAndFeel * skin_)
	{
		skin = skin_;
	}

protected:
	bool rightClickMenuIsEnabled = true;
	LookAndFeel * skin = nullptr;	

	PopupMenu createLeftClickMenu()
	{
		PopupMenu menu;
		for (int i = 0; i < getNumSelectableItems(); ++i)
		{
			MenuHelper::addCallbackItem(menu, getAssignedParameter()->getOptionStringAtIndex(i), [this, i]()
			{
				getAssignedParameter()->setValue(i, true, true);
			});
		}

		if (skin != nullptr)
			menu.setLookAndFeel(skin);

		return menu;
	}

	virtual void paint(Graphics& g) override;

	virtual void mouseEnter(const MouseEvent&) override
	{
		ParameterInfo::InfoSender::updateParameterInfo();
	}

	virtual void mouseDown(const MouseEvent& e) override
	{
		if (e.mods.isRightButtonDown())
		{
			if (rightClickMenuIsEnabled)
      {
        auto m = createRightClickMenu(this);
				MenuHelper::show(&m, this);
      }
		}
		else
		{
      auto m = createLeftClickMenu();
			MenuHelper::show(&m, this);
		}

		ParameterInfo::InfoSender::updateParameterInfo();
	}

	void parameterChanged(Parameter* /*p*/) override
	{
		//rsAutomatableComboBox::parameterChanged(p);

		if (assignedParameter == nullptr)
			return;

		setLocalAutomationSwitch(false); // to avoid circular notifications and updates
		updateWidgetFromAssignedParameter(true);
		setLocalAutomationSwitch(true);

		ParameterInfo::InfoSender::updateParameterInfo();
	}

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ElanModulatableComboBox)
};

class ElanRSlider
	: public RSlider
	, public ParameterInfo::InfoSender
{
public:
	ElanRSlider() = default;
	~ElanRSlider() = default;

protected:
	virtual void mouseDown(const MouseEvent & e) override
	{
		RSlider::mouseDown(e);
		ParameterInfo::InfoSender::updateParameterInfo();
	}

	virtual void mouseMove(const MouseEvent & e) override
	{
		RSlider::mouseMove(e);
		ParameterInfo::InfoSender::updateParameterInfo();
	}

	virtual void mouseWheelMove(const MouseEvent& e, const MouseWheelDetails& wheel) override
	{
		RSlider::mouseWheelMove(e, wheel);
		ParameterInfo::InfoSender::updateParameterInfo();
	}
};

/* TODO: ParameterInfo::InfoSender should be named Sender but it clashes with ParameterValueInput::Sender inside VectorSlider.h ElanModulatableSlider class*/
class ElanModulatableSlider
	: public rsModulatableSlider
	, public ParameterInfo::InfoSender
	, public ParameterValueInput::InputSender
{
public:
	ElanModulatableSlider() = default;
	~ElanModulatableSlider() = default;

	void setRightClickMenuEnabled(bool v)
	{
		rightClickMenuIsEnabled = v;
	}

	void mouseDoubleClick(const MouseEvent&) override {}

	virtual void mouseDown(const MouseEvent& e) override
	{
		if (e.mods.isRightButtonDown())
		{
			if (rightClickMenuIsEnabled)
      {
        auto m = createRightClickMenu(this);
        MenuHelper::show(&m, this);
      }
				
		}
		else
		{
			rsModulatableSlider::mouseDown(e);
		}
	}

	virtual void mouseMove(const MouseEvent& e) override
	{
		rsModulatableSlider::mouseMove(e);
		ParameterInfo::InfoSender::updateParameterInfo();
	}

	virtual void mouseDrag(const MouseEvent& e) override
	{
		double scale = .003;

		if (ModifierKeys::getCurrentModifiers().isShiftDown())
			scale *= 0.0625;

		if (!isEnabled())
			return;

		if (!e.mods.isRightButtonDown() && !e.mods.isCommandDown())
		{
			int newDragDistance = e.getDistanceFromDragStartY() - e.getDistanceFromDragStartX();

			int dragDelta = newDragDistance - oldDragDistance;
			oldDragDistance = newDragDistance;
			dragValue += scale * dragDelta;

			double y = normalizedValueOnMouseDown;
			y -= dragValue;
			y = clamp(y, 0.0, 1.0);

			setNormalizedValue(y);
		}

		ParameterInfo::InfoSender::updateParameterInfo();
	}

	virtual void mouseWheelMove(const MouseEvent& e, const MouseWheelDetails& wheel) override
	{
		rsModulatableSlider::mouseWheelMove(e, wheel);
		ParameterInfo::InfoSender::updateParameterInfo();
	}

	virtual void paint(Graphics& g) override
	{
		rsAutomatableSlider::paint(g);
		jura::ModulatableParameter* mp = dynamic_cast<jura::ModulatableParameter*> (assignedParameter);
		if (mp && mp->hasConnectedSources())
			g.fillAll(Colour::fromFloatRGBA(1.f, 0.f, 0.f, 0.125f)); // preliminary
	}

protected:
	void setStringValue(String v) override
	{
		getAssignedParameter()->setValue(v.getDoubleValue(), true, true);
		ParameterInfo::InfoSender::updateParameterInfo();
	}
	String getStringValue() override
	{
		auto param = getAssignedParameter();

		jassert(param != nullptr);

		if (param)
		{
			originalValue = getAssignedParameter()->getValue();
			return String(getAssignedParameter()->getValue(), 8).substring(0, 10);
		}

		return "";
	}
	double originalValue = 0;

	bool rightClickMenuIsEnabled = true;
};
