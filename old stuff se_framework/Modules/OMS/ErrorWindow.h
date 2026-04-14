#pragma once

class ErrorMessageButton : public TextButton
{
public:
	ErrorMessageButton() = default;
	~ErrorMessageButton() = default;

	void setIsSelected(bool v)
	{
		isSelected = v;
		repaint();
	}

	bool getIsSelected()
	{
		return isSelected;
	}

protected:
	bool isSelected = false;
};

/*
Suggested minimum error message size is 216 x 92 or ratio of 2.3 x 1.0.
Use "addChildComponent" rather than "addAndMakeVisible" so that "showErrorMessage" functions properly.
ErrorWindow uses full size bounds of parent component to properly draw blackout effect contain clicks to window buttons.
*/
class ErrorWindow : public Component, public Button::Listener, public KeyListener
{
public:

	ErrorWindow()
	{
		addKeyListener(this);
		setWantsKeyboardFocus(true);
	}
	~ErrorWindow()
	{
		clearButtons();
		removeKeyListener(this);
	}

	void showErrorMessage(int buttonToUseForReturnKey = 0, int buttonToUseForEscapeKey = 0);

	void showErrorMessage(String title_, String message_, int buttonToUseForReturnKey = 0, int buttonToUseForEscapeKey = 0);

	void setErrorWindowSize(int width, int height)
	{
		errorWindowBounds.setWidth(width);
		errorWindowBounds.setHeight(height);
		resized();
	}

	void setTitle(String v) { title = v; }

	void setMessage(String v) { message = v; }

	void addButton(String text, std::function<void()> callback);

	void clearButtons();

protected:
	SharedResourcePointer<Resources> RESOURCES;
	Rectangle<int> errorWindowBounds{ 216, 92 };
	juce::Rectangle<int> circle_area{ 25, 25 };
	juce::Rectangle<int> button_area;

	vector<unique_ptr<ErrorMessageButton>> buttons;

	String title = "ERROR";
	String message = "This is an error message.";

	int selectedButtonID = 0;
	int buttonIDForEscapeKey = 0;

	struct ButtonSkin : public LookAndFeel_V4
	{
		SharedResourcePointer<Resources> RESOURCES;

		Font font = RESOURCES->fonts.RobotoBold.withPointHeight(10);

		void drawButtonBackground(Graphics& g, Button& b, const Colour&, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

		void drawButtonText(Graphics& g, TextButton& b, bool, bool);
	};

	ButtonSkin buttonSkin;

	void setButtonSelected(ErrorMessageButton * b)
	{
		for (const auto & button : buttons)
			button.get()->setIsSelected(button.get() == b);
	}

	void decrementSelectedButton()
	{
		buttons[(size_t)selectedButtonID].get()->setIsSelected(false);
		selectedButtonID = wrapSingleOctave_Bipolar(--selectedButtonID, (int)buttons.size());
		buttons[(size_t)selectedButtonID].get()->setIsSelected(true);
	}

	void incrementSelectedButton()
	{
		buttons[(size_t)selectedButtonID].get()->setIsSelected(false);
		selectedButtonID = wrapSingleOctave_Positive(++selectedButtonID, (int)buttons.size());
		buttons[(size_t)selectedButtonID].get()->setIsSelected(true);
	}

	void buttonClicked(Button*) override { setVisible(false); }

	void resized() override;

	void paint(Graphics& g) override;

	bool keyPressed(const KeyPress& key, Component*) override
	{

		if (key.getKeyCode() == key.escapeKey)
		{
			buttons[buttonIDForEscapeKey].get()->triggerClick();
			return true;
		}

		if (key.getKeyCode() == key.returnKey)
		{
			buttons[selectedButtonID].get()->triggerClick();
		}

		if (key.getKeyCode() == key.rightKey)
		{
			incrementSelectedButton();
			return true;
		}

		if (key.getKeyCode() == key.leftKey)
		{
			decrementSelectedButton();
			return true;
		}

		if (key.getKeyCode() == key.tabKey)
		{
			if (key.getModifiers().isShiftDown())
				decrementSelectedButton();
			else
				incrementSelectedButton();
			return true;
		}

		return false;
	}

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ErrorWindow);
};
