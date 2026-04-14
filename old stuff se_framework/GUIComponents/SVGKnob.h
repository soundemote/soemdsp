#pragma once

namespace elan
{
class StaticImage : public jura::RWidget
{
public:
	StaticImage(const void * data, size_t size)
	{
		image.setImage(juce::ImageFileFormat::loadFrom(data, size));
		setInterceptsMouseClicks(false, false);
	}
	~StaticImage() = default;

	void setRotation(float v) { rotation = v; }

	void setDoNotPaint(bool v)
	{
		doNotPaint = v;
		repaint();
	} 

	void paint(Graphics & g)
	{
		if (doNotPaint)
			return;

		image.paint(g);
	}

	int getFileWidth() { return image.getImage().getWidth(); }
	int getFileHeight() { return image.getImage().getHeight(); }

	void setCenteredBounds(float centerX, float centerY, int /*width*/, int /*height*/)
	{
		juce::Rectangle<float> rect = juce::Rectangle<float>((float)getFileWidth(), (float)getFileHeight());
		rect.setCentre({ centerX, centerY });

		setSize(getFileWidth(), getFileHeight());
		setTopLeftPosition((int)rect.getX(), (int)rect.getY());
	}

	void parameterChanged(Parameter* parameterThatHasChanged)
	{
		setDoNotPaint(parameterThatHasChanged->getValue() <= 0.0);
	}

protected:
	DrawableImage image;
	bool doNotPaint = false;
	double rotation = 0;
};

class TextComponent : public Component
{
	TextComponent() = default;
	~TextComponent() = default;
};

class StaticText : public Component, public AsyncUpdater
{
public:
	StaticText()
	{
		setInterceptsMouseClicks(false, false);
	};

	StaticText(Font font_) : font(font_)
	{
		setInterceptsMouseClicks(false, false);
	};

	StaticText(String text_, Font font_, float pointHeight_, Colour color_, juce::Justification justification_ = juce::Justification::centred)
		: text(text_)
		, font(font_)
		, pointHeight(pointHeight_)
		, fontColor(color_)
		, justificationStyle(justification_)
	{
		setInterceptsMouseClicks(false, false);
	};

	void draw(Graphics & g)
	{
		if (drawMultilineText)
		{
			g.setFont(font);
			g.setColour(fontColor);
			g.drawMultiLineText(text, 0, (int)pointHeight, getWidth());
		}
		else
		{
			g.setColour(fontColor);
			g.setFont(font.withPointHeight(pointHeight));
			if (doDrawFittedText)
				g.drawFittedText(text, getLocalBounds(), justificationStyle, 1, 0.2f);
			else
				g.drawText(text, getLocalBounds(), justificationStyle);
		}
	}

	void setFont(Font v)
	{
		font = v;
		pointHeight = v.getHeightInPoints();
		triggerAsyncUpdate();
	}

	void setFontPointHeight(float v)
	{
		pointHeight = v;
		triggerAsyncUpdate();
	}

	void setFontColor(Colour c)
	{
		fontColor = c;
		triggerAsyncUpdate();
	}

	void setText(const String & s)
	{
		text = s;
		triggerAsyncUpdate();
	}

	void setFontJustification(juce::Justification v)
	{
		justificationStyle = v;
		repaint();
	}

	void setDrawMultilineText(bool v)
	{
		drawMultilineText = v;
		triggerAsyncUpdate();
	}

	void setDoDrawFittedText(bool v)
	{
		doDrawFittedText = v;
	}

	void setTextYOffset(float v)
	{
		offsetY = v;
		triggerAsyncUpdate();
	}

protected:
	Font font;
	Colour fontColor = Colours::black;
	String text;
	String style;
	juce::Justification justificationStyle = juce::Justification::centred;
	float pointHeight = 13;
	float offsetY = 0;
	bool drawMultilineText = false;
	bool doDrawFittedText = false;

	Component textBounds;

	void paint(Graphics & g)
	{
		draw(g);
	}

	void handleAsyncUpdate() override
	{
		repaint();
	}
};


}// namespace elan

class TextDisplay : public elan::StaticText, public jura::RSliderPainter
{

public:
	TextDisplay(Font font)
	{
		setFont(font);
	}

	void paint(Graphics & g, jura::RSlider * slider) override
	{
		String txt = textFunction();

		if (drawMultilineText)
		{
			g.setFont(font);
			g.setColour(fontColor);
			g.drawMultiLineText(txt, 0, (int)pointHeight, /*slider->*/getWidth());
		}
		else
		{
			g.setFont(font);
			g.setColour(fontColor);
			g.drawText(txt, juce::Rectangle<float>{ 0.f, 0.f + offsetY, (float)slider->getWidth(), (float)slider->getHeight() }, justificationStyle);
		}
	}

	std::function<String()> textFunction = []() { return ""; };
};

class SVGKnob : public ElanModulatableSlider
{
public:
	SVGKnob(const String & svgTick = "", const String & svgBackground = "");
	~SVGKnob() = default;

	const jura::BitmapFont * font;
	float currentValue = 0;

	void drawKnob(Graphics & g, juce::Rectangle<float> area);
	void paint(Graphics & g) override;

	void mouseDrag(const MouseEvent& e) override;

	// Sets the default knob size so that gui elements will scale accordingly.
	void setStandardSize(float v);


	void setArcHighlightColor(Colour v)
	{
		arc_highlightColor = v;
	}

	void setBackgroundColor(Colour v)
	{
		backgroundColor = v;
	}

	void setArcUnfilledMargin(float v) { arcUnfilledMargin = v; }
	void setArcUnfilledThickness(float v) { arcUnfilledThickness = v; }
	void setArcUnfilledColor(juce::Colour v);

	void setArcFillMargin(float v) { arcFillMargin = v; }
	void setArcFillThickness(float v) { arcFillThickness = v; }
	void setArcFillColor(juce::Colour v);


	void setArcUnfilled(float margin, float thickness, Colour color)
	{
		setArcUnfilledMargin(margin);
		setArcUnfilledThickness(thickness);
		setArcUnfilledColor(color);

		doDrawArcUnfilled = thickness > 0 || color.isOpaque();
	}

	void setArcFill(float margin, float thickness, Colour color)
	{
		setArcFillMargin(margin);
		setArcFillThickness(thickness);
		setArcFillColor(color);

		doDrawArcFilled = thickness > 0 || color.isOpaque();
	}

	void setIsBipolar(bool v);

	void setKnobRotationOffset(float v)
	{
		knobRotationOffset = v;
	}

protected:
	juce::Rectangle<float> knobArea;
	juce::Component knobComponent;

	VectorGraphic background;
	VectorGraphic tick;

	float knobRotationOffset = 0;

	float standardSize = -1;
	float scale = 1;
	float arcFillThickness = 6;
	float arcUnfilledThickness = 6;
	float arcFillMargin = 0;
	float arcUnfilledMargin = 0;

	bool isBipolar = false;

	Colour arc_unfilledColour{ 0,0,0 };
	Colour arc_filledColour{ 255, 255, 255 };
	Colour arc_highlightColor{ 0.f, 0.f, 0.f, 0.f };
	Colour backgroundColor{ 0.f,0.f,0.f,0.f };

	bool doDrawArcUnfilled = true;
	bool doDrawArcFilled = true;
};
