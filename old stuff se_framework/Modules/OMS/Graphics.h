#pragma once

using namespace elan;

inline juce::Rectangle<float> centeredBounds(const juce::Rectangle<int>& bounds, int width, int height)
{
	auto area = juce::Rectangle<int>(width, height);
	area.setCentre(bounds.getCentre());
	return std::move(area.toFloat());
}
inline juce::Rectangle<float> centeredBounds(const juce::Rectangle<int>& bounds, Image* image)
{
	auto area = juce::Rectangle<int>(image->getWidth(), image->getHeight());
	area.setCentre(bounds.getCentre());
	return std::move(area.toFloat());
}

template <typename T> juce::Rectangle<T> removeMargin(juce::Rectangle<T>& bounds, T marginLeft, T marginTop, T marginRight, T marginBottom)
{
	return bounds.removeFromLeft(marginLeft)
		.removeFromTop(marginTop)
		.removeFromRight(marginRight)
		.removeFromBottom(marginBottom);
}

//DROPDOWNS
class PresetButton : public Component
{
public:

	PresetButton() = default;
	~PresetButton() = default;

	void setImage(Image* v)
	{
		image = v;
	}

	std::function<void()> clickedFunc = []() { return; };

protected:
	Image* image = nullptr;

	void paint(Graphics& g) override
	{
		g.setColour({ 119,123,121 });
		g.drawRect(getLocalBounds());
		g.drawImage(*image, centeredBounds(getLocalBounds(), image));
	}

	void mouseDown(const MouseEvent&) override
	{
		clickedFunc();
	}
};

class SavePreset : public Component
{
public:
	SavePreset() = default;
	~SavePreset() = default;

protected:
	SharedResourcePointer<Resources> RESOURCES;
	OneStateImage::Painter image{ RESOURCES->images.icon_save };

	void paint(Graphics& g) override
	{
		image.draw(g, getLocalBounds().toFloat());
	}
};

class LoadPreset : public Component
{
public:
	LoadPreset() = default;
	~LoadPreset() = default;

protected:
	SharedResourcePointer<Resources> RESOURCES;
	OneStateImage::Painter image{ RESOURCES->images.icon_load };

	void paint(Graphics& g) override
	{
		image.draw(g, getLocalBounds().toFloat());
	}
};

class FactoryPresetButton : public Component
{
public:
	FactoryPresetButton() = default;
	~FactoryPresetButton() = default;

	bool isEnabled = false;

	std::function<void()> clickedFunc = []() { return; };

protected:
	SharedResourcePointer<Resources> RESOURCES;
	TwoStateImage::Painter image{ RESOURCES->images.presetWindow_f };

	void paint(Graphics& g) override
	{
		image.draw(g, isEnabled);
	}

	void mouseDown(const MouseEvent&) override
	{
		clickedFunc();
		isEnabled = !isEnabled;
		repaint();
	}
};

class UserPresetButton : public Component
{
public:
	UserPresetButton() = default;
	~UserPresetButton() = default;

	bool isEnabled = false;

	std::function<void()> clickedFunc = []() { return; };

protected:
	SharedResourcePointer<Resources> RESOURCES;
	TwoStateImage::Painter image{ RESOURCES->images.presetWindow_u };

	void paint(Graphics& g) override
	{
		image.draw(g, isEnabled);
	}

	void mouseDown(const MouseEvent&) override
	{
		clickedFunc();
		isEnabled = !isEnabled;
		repaint();
	}
};

class MenuSM : public ElanModulatableComboBox
{
public:
	MenuSM() = default;
	~MenuSM() = default;

protected:
	SharedResourcePointer<Resources> RESOURCES;
	OneStateImage::Painter image{ RESOURCES->images.radar_menu_sm };
	Font font = RESOURCES->fonts.TekoSemiBold.withPointHeight(11.f);

	void paint(Graphics& g) override
	{
		g.setColour({ 215,215,215 });
		image.draw(g, getLocalBounds().toFloat());

		g.setFont(font);
		g.drawFittedText(getSelectedItemText().toUpperCase(), ElanRect::pad(getLocalBounds(), 2, 2, 14, 2).translated(0, 1), juce::Justification::centred, 1, 0.5f);

		{// add red overlay to indicate modulation
			jura::ModulatableParameter* mp = dynamic_cast<jura::ModulatableParameter*> (assignedParameter);
			if (mp && mp->hasConnectedSources())
			{
				g.setColour(Colour::fromFloatRGBA(1.f, 0.f, 0.f, 0.5f));
				g.drawRect(getLocalBounds().toFloat(), 2.5f);
			}
		}
	}
};

class MenuLG : public ElanModulatableComboBox
{
public:
	MenuLG() = default;
	~MenuLG() = default;

protected:
	SharedResourcePointer<Resources> RESOURCES;
	OneStateImage::Painter image{ RESOURCES->images.radar_menu_lg };
	Font font = RESOURCES->fonts.TekoSemiBold.withPointHeight(11.f);

	void paint(Graphics& g) override
	{	
		image.draw(g, getLocalBounds().toFloat());

		g.setColour({ 215,215,215 });
		g.setFont(font);
		g.drawFittedText(getSelectedItemText().toUpperCase(), ElanRect::pad(getLocalBounds(), 2, 2, 14, 2).translated(0, 1), juce::Justification::centred, 1, 0.5f);

		{// add red overlay to indicate modulation
			jura::ModulatableParameter* mp = dynamic_cast<jura::ModulatableParameter*> (assignedParameter);
			if (mp && mp->hasConnectedSources())
			{
				g.setColour(Colour::fromFloatRGBA(1.f, 0.f, 0.f, 0.5f));
				g.drawRect(getLocalBounds().toFloat(), 2.5f);
			}
		}
	}
};

//KNOBS & SLIDERS FILMSTRIPS

class VerticalHandleSlider : public ElanModulatableSlider
{
protected:
	SharedResourcePointer<Resources> RESOURCES;

public:
	VerticalHandleSlider() = default;
	~VerticalHandleSlider() = default;

	Rectangle<int> getBoundsForSliderArea(int x, int y, int w, int h)
	{
		originalBounds = { w, h };

		auto globalCenteredRect = ElanRect::place(image.getImageBounds(), { x, y, w, h }, Anchor::TopCenter, Anchor::TopCenter);

		int newY = y - sliderStopperLocationInImage.getY();		
		int newH = (h + sliderStopperLocationInImage.getY()) + (image.getImageHeight() - sliderStopperLocationInImage.getBottom());

		maxY = newH - image.getImageHeight();

		return { globalCenteredRect.getX(), newY, image.getImageWidth(), newH };
	}

	OneStateImage::Painter image{ RESOURCES->images.slider_handle };

protected:
	juce::Rectangle<int> currentHandleArea;
	int currentYoffset = 0;
	int maxY = 0;

	bool debug = false;

	Rectangle<int> originalBounds;
	const Rectangle<int> sliderHandleLocationInImage{ 12, 23, 10, 19 };
	const Rectangle<int> sliderStopperLocationInImage{ 12, 30, 10, 5 };

	void paint(Graphics& g) override
	{
		currentYoffset = (1 - getNormalizedValue()) * maxY;
		image.draw(g, image.getImageBounds().withY(currentYoffset).toFloat());

		currentHandleArea = sliderHandleLocationInImage.withPosition(sliderHandleLocationInImage.getX(), currentYoffset + sliderHandleLocationInImage.getY());

		if (debug)
		{
			g.setColour(Colour::fromFloatRGBA(1.f, 0.f, 0.f, 0.5f));
			g.drawRect(getLocalBounds().toFloat(), 2.5f);

			g.setColour(Colour::fromFloatRGBA(0.f, 1.f, 0.f, 0.5f));
			g.drawRect(ElanRect::place(originalBounds.toFloat(), getLocalBounds().toFloat(), Anchor::MidCenter, Anchor::MidCenter), 2.5f);
		}

		{// add red overlay to indicate modulation
			jura::ModulatableParameter* mp = dynamic_cast<jura::ModulatableParameter*> (assignedParameter);
			if (mp && mp->hasConnectedSources())
			{
				g.setColour(Colour::fromFloatRGBA(1.f, 0.f, 0.f, 0.5f));
				g.drawRect(currentHandleArea.toFloat(), 2.5f);
			}
		}
	}

	void resized() override
	{

	}

	bool hitTest(int x, int y) override
	{
		return currentHandleArea.contains(x, y);
	}

	void mouseDrag(const MouseEvent& e) override
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
			y = clip(y, 0.0, 1.0);

			setNormalizedValue(y);
		}

		ParameterInfo::InfoSender::updateParameterInfo();
	}
};

class BoxSlider : public ElanModulatableSlider
{
public:
	BoxSlider() = default;
	~BoxSlider() = default;

protected:
	void paint(Graphics& g) override
	{
		g.setColour({ 188,183,75 });
		g.fillRect(0, 0, int(getNormalizedValue() * getWidth()), getHeight());
		g.setColour({ 119,123,121 });
		g.drawRect(getLocalBounds());

		{// add red overlay to indicate modulation
			jura::ModulatableParameter* mp = dynamic_cast<jura::ModulatableParameter*> (assignedParameter);
			if (mp && mp->hasConnectedSources())
			{
				g.setColour(Colour::fromFloatRGBA(1.f, 0.f, 0.f, 0.5f));
				g.drawRect(getLocalBounds().toFloat(), 2.5f);
			}
		}
	}

	void mouseDrag(const MouseEvent& e) override
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
			y = clip(y, 0.0, 1.0);

			setNormalizedValue(y);
		}

		ParameterInfo::InfoSender::updateParameterInfo();
	}
};

// TABS
class TabHandler
{
public:
	TabHandler() = default;
	~TabHandler() = default;

	class Tab : public Component
	{
		friend TabHandler;
	public:
		Tab() = default;
		~Tab() = default;

		// Function for when the tab state is changed
		void setIsActive(bool v)
		{
			isActive = v;

			for (auto& t : associatedComponents)
				t->setVisible(isActive);

			repaint();

			tabStateChanged();
		}

		bool getIsActive()
		{
			return isActive;
		}

		void addAssociatedComponentsForShowHide(Array<Component*> v)
		{
			for (const auto& c : v)
			{
				jassert(!associatedComponents.contains(c)); // a component has been added more than once
				associatedComponents.add(c);
			}
		}

		std::function<void()> tabStateChanged = []() { return; };

	protected:
		Array<Component*> associatedComponents;
		TabHandler* tabHandler = nullptr;
		bool isActive = false;

		void mouseDown(const MouseEvent&) override
		{
			tabHandler->enableTab(this);
		}
	};

	void enableTab(Tab* tab)
	{
		for (auto& t : tabs)
			t->setIsActive(tab == t);
	}

	void addTab(Tab * tab)
	{
		tabs.addIfNotAlreadyThere(tab);
		tab->tabHandler = this;
	}

private:
	Array<Tab*> tabs;
};

class SectionTab : public TabHandler::Tab
{
public:
	SectionTab(String text_) : text(text_)
	{
		font = RESOURCES->fonts.TekoSemiBold;
	}

	void setText(String v)
	{
		text = v;
	}

protected:
	void paint(Graphics& g) override
	{
		image.draw(g, getLocalBounds().toFloat(), isActive);

		if (isActive)
			g.setColour({ 213,215,216 });
		else
			g.setColour({ 69,85,89 });

		g.setFont(font.withPointHeight(14.f));
		g.drawText(text, getLocalBounds().translated(0, 2), juce::Justification::centred, false);
	}

	SharedResourcePointer<Resources> RESOURCES;
	TwoStateImage::Painter image{ RESOURCES->images.radar_tab };
	Font font;
	String text;
};

class TabCorner : public TabHandler::Tab
{
public:
	TabCorner(String text_) : text(text_)
	{
		font = RESOURCES->fonts.RobotoBold;
	}

protected:
	SharedResourcePointer<Resources> RESOURCES;
	TwoStateImage::Painter image{ RESOURCES->images.radar_tab_corner };
	Font font;
	String text;

	void paint(Graphics& g) override
	{
		image.draw(g, getLocalBounds().toFloat(), isActive);

		g.setColour({ 255,241,229 });
		g.setFont(font.withPointHeight(12.f));
		g.drawText(text, getLocalBounds().translated(3, 1), juce::Justification::centred, false);
	}
};

class TabSquare : public TabHandler::Tab
{
public:
	TabSquare(String text_) : text(text_)
	{
		font = RESOURCES->fonts.RobotoBold;
	}

	void setText(String v)
	{
		text = v;
	}

protected:
	void paint(Graphics& g) override
	{
		painter.draw(g, getLocalBounds().toFloat(), TabHandler::Tab::isActive);

		g.setColour({ 255,241,229 });
		g.setFont(font.withPointHeight(12.f));
		g.drawText(text, getLocalBounds().translated(0, 1), juce::Justification::centred, false);
	}

	SharedResourcePointer<Resources> RESOURCES;
	TwoStateImage::Painter painter{ RESOURCES->images.radar_tab_mid };
	Font font;
	String text;	
};

//WIDGETSETS

class NumberSlider : public ElanModulatableSlider
{
protected:
	SharedResourcePointer<Resources> RESOURCES;
	OneStateImage::Painter image{ RESOURCES->images.radar_number_box };
	Font font{ RESOURCES->fonts.TekoMedium };

	void paint(Graphics& g) override
	{
		image.draw(g, getLocalBounds().toFloat());

		g.setColour({ 255,255,255 });
		g.setFont(font.withPointHeight(12.f));
		g.drawText("x8", getLocalBounds(), juce::Justification::centred, false);

		{// add red overlay to indicate modulation
			jura::ModulatableParameter* mp = dynamic_cast<jura::ModulatableParameter*> (assignedParameter);
			if (mp && mp->hasConnectedSources())
			{
				g.setColour(Colour::fromFloatRGBA(1.f, 0.f, 0.f, 0.5f));
				g.drawRect(getLocalBounds().toFloat(), 2.5f);
			}
		}
	}
};

class ValueEntry : public Component
{
public:
	ValueEntry()
	{
		addAndMakeVisible(editor);
		editor.setColour(TextEditor::backgroundColourId, Colours::transparentBlack.withAlpha(0.5f));
		editor.setColour(TextEditor::outlineColourId, Colours::transparentWhite);
		editor.setColour(TextEditor::focusedOutlineColourId, Colours::transparentWhite);
		editor.setColour(TextEditor::textColourId, Colours::white);
		editor.setColour(TextEditor::highlightedTextColourId, Colours::white);
		editor.setColour(TextEditor::highlightColourId, Colours::white.withAlpha(0.3f));
		editor.setColour(CaretComponent::caretColourId, Colours::white);
		editor.setFont(font.withPointHeight(13.f));
	}

	void resized() override
	{
		if (doDrawWithLeftArrow)
			editor.setBounds(ElanRect::pad(getLocalBounds(), 8, 3, 3, 3));
		else
			editor.setBounds(ElanRect::pad(getLocalBounds(), 8, 3, 3, 3));
	}

	void moved() override
	{
		if (doDrawWithLeftArrow)
			editor.setBounds(ElanRect::pad(getLocalBounds(), 8, 3, 3, 3));
		else
			editor.setBounds(ElanRect::pad(getLocalBounds(), 3, 3, 8, 3));
	}

	void drawWithLeftArrow()
	{
		doDrawWithLeftArrow = true;
	}
	void drawWithRightArrow()
	{
		doDrawWithLeftArrow = false;
	}

	~ValueEntry() = default;

	TextEditor editor;
	Image imageFlipped;

protected:
	SharedResourcePointer<Resources> RESOURCES;
	OneStateImage::Painter leftSideImage{ RESOURCES->images.leftInputBox };
	OneStateImage::Painter middleImage{ RESOURCES->images.midInputBox };
	OneStateImage::Painter rightSideImage{ RESOURCES->images.rightInputBox };

	Font font;

	bool doDrawWithLeftArrow = true;

	void paint(Graphics& g) override
	{
		if (!doDrawWithLeftArrow)
			g.addTransform(AffineTransform(-1.f, 0.f, (float)getWidth(), 0.f, 1.f, 0.f));

		auto area = getLocalBounds().toFloat();

		leftSideImage.draw(g, area.removeFromLeft(8));
		leftSideImage.draw(g, area.removeFromRight(3));
		leftSideImage.draw(g, area);
	}
};
