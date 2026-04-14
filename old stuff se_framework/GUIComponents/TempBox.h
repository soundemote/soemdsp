#pragma once

class LeftClickButton : public Component
{
public:
	using Component::Component;

	void resetState()
	{
		leftButtonWasClicked = false;
		mouseIsOver = false;
		repaint();
	}

	bool isDown()
	{
		return leftButtonWasClicked;
	}

	bool isOver()
	{
		return mouseIsOver;
	}

	std::function<void()> clickFunction = []() { jassertfalse; };
	std::function<void(Graphics&)> paintFunction = [](Graphics&) { jassertfalse; };

protected:
	bool leftButtonWasClicked = false;
	bool mouseIsOver = false;

	void paint(Graphics& g) override
	{
		paintFunction(g);
	}

	void mouseEnter(const MouseEvent&) override
	{
		mouseIsOver = true;
		repaint();
	}

	void mouseExit(const MouseEvent&) override
	{
		leftButtonWasClicked = false;
		mouseIsOver = false;
		repaint();
	}

	void mouseDown(const MouseEvent& e) override
	{
		leftButtonWasClicked = e.mods.isLeftButtonDown();

		if (leftButtonWasClicked)
			repaint();
	}

	void mouseUp(const MouseEvent& e) override
	{
		if (!leftButtonWasClicked || !getLocalBounds().toFloat().contains(e.position))
			return;

		repaint();
		clickFunction();
		leftButtonWasClicked = false;	
	}
};


class ElanPaintButton : public Component
{
	public:
		ElanPaintButton(Image* image_) : image(image_) {}
		~ElanPaintButton() = default;

		std::function<void(Graphics& g)> paintFunc = [](Graphics&) { return; };
		std::function<void()> clickedFunc = []() { return; };

protected:
	Image * image;

	void mouseDown(const MouseEvent&) override
	{
		clickedFunc();
	}

	void paint(Graphics& g) override
	{
		paintFunc(g);
	}
};

class ElanImageButton : public Component
{
public:
	ElanImageButton(Image* image_) : image(image_)
	{
		setSizeToImage();
	}
	~ElanImageButton() = default;

	void centerWithinBounds()
	{
		setCentrePosition(getBounds().getCentre());
	}

	void setSizeToImage()
	{
		auto boundsCenter = getBounds().getCentre();
		setSize(image->getWidth(), image->getHeight());
		setCentrePosition(boundsCenter);
	}

	void setSizeToImageAndCenterWithinBounds()
	{
		centerWithinBounds();
		setSizeToImage();
	}

	void setSizeToImageAndCenterWithinBounds(juce::Rectangle<int> bounds)
	{
		setBounds(bounds);
		centerWithinBounds();
		setSizeToImage();
	}

	std::function<void()> clickedFunc = []() { return; };	
	Image * image;

protected:
	void mouseDown(const MouseEvent&) override
	{
		clickedFunc();
	}

	void paint(Graphics & g) override
	{
		g.drawImage(*image, image->getBounds().toFloat());
	}
};

class TempBox : public Component
{
public:
	TempBox() = default;
	~TempBox() = default;

	void setOpacity(float v)
	{
		opacity = v;
	}
	void setColor(Colour v)
	{
		color = v;
	}

protected:
	void paint(Graphics & g) override
	{
		g.setColour(color.withAlpha(opacity));
		g.fillRect(getLocalBounds());
	}

	float opacity = 0.5f;
	Colour color{ 255,255,255 };
};
