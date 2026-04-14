#pragma once

class ClickableArea : public RButton, public ParameterInfo::InfoSender
{
public:
	ClickableArea() = default;
	~ClickableArea() = default;
	
	void paint(Graphics & g) override
	{
		if (doHighlight)
			g.fillAll(Colours::white.withAlpha(0.3f));
	}
	
	void clicked() override
	{
		if (assignedParameter != nullptr)
			assignedParameter->setValue(assignedParameter->getValue() + incrementAmount, true, true);

		sendClickMessage();
	}
	
	String getValueDisplayString() const override
	{
		return stringFunc();
	}
	std::function<String()> stringFunc = []() { return String(); };

	void setIncrementAmount(double v)
	{
		incrementAmount = v;
	}
	
protected:
	double incrementAmount = 1;
	float rotation = 0;
	bool doHighlight = false;

	void mouseEnter(const MouseEvent&) override
	{
		doHighlight = true;
		repaint();
	}

	void mouseExit(const MouseEvent&) override
	{
		doHighlight = false;
		repaint();
	}

	void mouseDown(const MouseEvent& e) override
	{
		RButton::mouseDown(e);
		ParameterInfo::InfoSender::updateParameterInfo();
	}
};

class ModulatableNumberDisplay : public ElanModulatableSlider
{
public:
	ModulatableNumberDisplay() = default;
	~ModulatableNumberDisplay() = default;

	std::function<String()> stringFunc = []() { return String(); };

	void setMouseScaling(double v)
	{
		mouseScaling = v;
	}

protected:
	SharedResourcePointer<Resources> RESOURCES;
	Image * image = &RESOURCES->images.radar_number_box.getImage();
	Font font = RESOURCES->fonts.TekoMedium;
	double mouseScaling = .003;

	void paint(Graphics & g) override
	{
		g.drawImage(*image, getLocalBounds().toFloat());

		g.setColour({ 255, 255, 255 });
		g.setFont(font.withPointHeight(13));
		g.drawText(stringFunc(), getLocalBounds().translated(0, 1), Justification::centred, false);

		{// add red overlay to indicate modulation
			jura::ModulatableParameter* mp = dynamic_cast<jura::ModulatableParameter*> (assignedParameter);
			if (mp && mp->hasConnectedSources())
			{
				g.setColour(Colour::fromFloatRGBA(1.f, 0.f, 0.f, 0.5f));
				g.drawRect(getLocalBounds().toFloat(), 2.5f);
			}
		}
	}

	void mouseDrag(const MouseEvent & e) override
	{
		double scale = mouseScaling;

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

	String getValueDisplayString() const override
	{
		return stringFunc();
	}
};


class NumberDisplay : public ElanRSlider
{
public:
	NumberDisplay()
	{
		image = &RESOURCES->images.radar_number_box.getImage();
		font = RESOURCES->fonts.TekoMedium;
	}
	
	~NumberDisplay() = default;

	std::function<String()> stringFunc = []() { return String(); };

	void setMouseScaling(double v)
	{
		mouseScaling = v;
	}
	
protected:
	SharedResourcePointer<Resources> RESOURCES;
	Image * image;
	Font font;
	double mouseScaling = .003;
	
	void paint(Graphics & g) override
	{
		g.drawImage(*image, getLocalBounds().toFloat());

		g.setColour({ 255, 255, 255 });
		g.setFont(font.withPointHeight(13));
		g.drawText(stringFunc(), getLocalBounds().translated(0,1), Justification::centred, false);
	}
	
	void mouseDrag(const MouseEvent & e) override
	{
		double scale = mouseScaling;
				
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
	
	String getValueDisplayString() const override
	{
		return stringFunc();
	}
};

