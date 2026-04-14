#pragma once

class ResourcesBase
{
protected:
	static Font loadFont(const void* data, const int size)
	{
		return Typeface::createSystemTypefaceFor(data, size);
	}
};

class FontContainer
{
public:
	class Painter;

	FontContainer(const void* data, const int size)
	{
		font = Font(Typeface::createSystemTypefaceFor(data, size));
	}

	FontContainer(String systemFontName)
	{
		font = Font(systemFontName, 13, Font::plain);
	}

	FontContainer(Font systemFont)
	{
		font = systemFont;
	}

	Font withPointHeight(float v)
	{
		return font.withPointHeight(v);
	}

	Font font;
};

/* under construction */
class FontContainer::Painter
{
public:
	Painter(FontContainer& fontContainerObject) : ptr(fontContainerObject)
	{
		font = ptr.font;
	}

	void setFontSize(float v)
	{
		font = font.withPointHeight(v);
	}

	void drawText(Graphics& g, StringRef text, juce::Rectangle<float> area, const Justification& justification = Justification::centred)
	{
		g.drawText(text, area.translated(xOffset, yOffset), justification);
	}

	void setFontAndDrawText(Graphics& g, StringRef text, juce::Rectangle<float> area, const Justification& justification = Justification::centred)
	{
		g.setFont(font);
		g.drawText(text, area.translated(xOffset, yOffset), justification);
	}

	void setOffset(float x, float y)
	{
		xOffset = x;
		yOffset = y;
	}

	Font get()
	{
		return font;
	}

protected:
	float ptHeight;
	Font font;
	FontContainer& ptr;

	float xOffset = 0;
	float yOffset = 0;
};


/* The reason classes are separated into parent classes and painter subclasses is so that the painter subclass can be used separately from the main class.
For example, we want to store a two-state button image as a TwoStateImage object, but we want to have multiple objects referring to that image. These
objects need to contain a TwoStateImage::Painter so that it can refer to the TwoStateImage object and paint itself based on its state.
*/

class OneStateImage
{
	friend class OneStateImagePainter;

public:
	class Painter;

	OneStateImage(const void* data, const int size)
	{
		image = juce::ImageFileFormat::loadFrom(data, size);
	}
	~OneStateImage() = default;

	Image& getImage() { return image; }
	int getImageWidth() { return image.getWidth(); }
	int getImageHeight() { return image.getHeight(); }
	void draw(Graphics& g)
	{
		g.drawImage(image, image.getBounds().toFloat());
	}
	void draw(Graphics& g, const juce::Rectangle<float>& area) const
	{
		g.drawImage(image, area);
	}
	void drawWithRotation(Graphics& g, const juce::Rectangle<float>& rectangle, float rotationDegrees) const
	{
		juce::AffineTransform xform = RectanglePlacement(RectanglePlacement::centred).getTransformToFit(getImageBounds().toFloat(), rectangle)
			.rotated(juce::degreesToRadians(rotationDegrees), rectangle.getCentreX(), rectangle.getCentreY());

		g.drawImageTransformed(image, xform);
	}

	Rectangle<int> getImageBounds() const { return image.getBounds(); }	

protected:
	Image image;
};

class OneStateImage::Painter
{
public:
	Painter(OneStateImage& image) : ptr(image) {}

	Image& getImage() { return ptr.getImage(); }
	int getImageWidth() { return ptr.getImageWidth(); }
	int getImageHeight() { return ptr.getImageHeight(); }
	Rectangle<int> getImageBounds() { return ptr.getImageBounds(); }

	void draw(Graphics& g) { ptr.draw(g); }
	void draw(Graphics& g, const juce::Rectangle<float>& area) { ptr.draw(g, area); }

protected:
	OneStateImage& ptr;
};

class TwoStateImage
{
public:
	class Painter;

	TwoStateImage(const void* buttonOff_data, const int buttonOff_size, const void* buttonOn_data, const int buttonOn_size)
	{
		images[0] = juce::ImageFileFormat::loadFrom(buttonOff_data, buttonOff_size);
		images[1] = juce::ImageFileFormat::loadFrom(buttonOn_data, buttonOn_size);
	}
	~TwoStateImage() = default;

	Image& getImage(bool state) { return images[state ? 1 : 0]; }
	int getImageWidth() { return images[0].getWidth(); }
	int getImageHeight() { return images[0].getHeight(); }
	Rectangle<int> getImageBounds() { return images[0].getBounds(); }

	void draw(Graphics& g, bool state)
	{
		g.drawImage(getImage(state), images[0].getBounds().toFloat());
	}
	void draw(Graphics& g, const juce::Rectangle<float>& area, bool state)
	{
		g.drawImage(getImage(state), area);
	}

protected:
	vector<Image> images{ 2 };
};

class TwoStateImage::Painter
{
public:
	Painter(TwoStateImage& image) : ptr(image) {}

	Image& getImage(bool state) { return ptr.getImage(state); }
	int getImageWidth() { return ptr.getImageWidth(); }
	int getImageHeight() { return ptr.getImageHeight(); }
	Rectangle<int> getImageBounds() { return ptr.getImageBounds(); }

	void draw(Graphics& g) { ptr.draw(g, m_state); }
	void draw(Graphics& g, bool state) { ptr.draw(g, state); }
	void draw(Graphics& g, const juce::Rectangle<float>& area, bool state) { ptr.draw(g, area, state); }

	void flipState() { m_state = !m_state; }

protected:
	TwoStateImage& ptr;
	bool m_state = false;
};

class VectorGraphic
{
public:
	class Painter;

	VectorGraphic()
	{
		///////unique_ptr<XmlElement> xml(XmlDocument::parse(defaultSvgString));
		///////drawable.reset(static_cast<DrawableComposite*>(Drawable::createFromSVG(*xml)));
	}

	VectorGraphic(const String& svg)
	{
		if (svg.isEmpty())
		{
			unique_ptr<XmlElement> xml(XmlDocument::parse(defaultSvgString));
			///////drawable.reset(static_cast<DrawableComposite*>(Drawable::createFromSVG(*xml)));
		}
		else
		{
			unique_ptr<XmlElement> xml(XmlDocument::parse(svg));
			///////drawable.reset(static_cast<DrawableComposite*>(Drawable::createFromSVG(*xml)));
			/* compiler error */
			//drawable = std::make_unique<DrawableComposite>(static_cast<DrawableComposite*>(Drawable::createFromSVG(*xml)));
		}
	}

	std::unique_ptr<Drawable> createCopyWithColor(const Colour & colorToReplace, const Colour & newColor) const
	{
		auto temp = drawable.get()->createCopy();
		temp->replaceColour(colorToReplace, newColor);
		return temp;
	}
	~VectorGraphic() = default;

	std::unique_ptr<Drawable> createCopy() const
	{
		return drawable.get()->createCopy();
	}

	void draw(Graphics & g, const juce::Rectangle<float> & rectangle, float opacity = 1.f) const
	{
		juce::AffineTransform xform = RectanglePlacement(RectanglePlacement::centred).getTransformToFit(drawable->getContentArea(), rectangle);
		drawable->draw(g, opacity, xform);
	}

	void drawWithColor(Graphics & g, const juce::Rectangle<float> & rectangle, const Colour & colorToReplace, const Colour & newColor, float opacity = 1.f) const
	{
		juce::AffineTransform xform = RectanglePlacement(RectanglePlacement::centred).getTransformToFit(drawable->getContentArea(), rectangle);
		auto temp = make_unique<DrawableComposite>(*static_cast<DrawableComposite*>(drawable.get()));
		temp->replaceColour(colorToReplace, newColor);
		temp->draw(g, opacity, xform);
	}

	void drawWithRotation(Graphics & g, const juce::Rectangle<float> & rectangle, float rotationDegrees, float opacity = 1.f) const
	{
		juce::AffineTransform xform = RectanglePlacement(RectanglePlacement::centred).getTransformToFit(drawable->getContentArea(), rectangle)
			.rotated(juce::degreesToRadians(rotationDegrees), rectangle.getCentreX(), rectangle.getCentreY());

		drawable->draw(g, opacity, xform);
	}

	Rectangle<int> getBounds() const
	{
		return drawable->getBounds();
	}

	bool isValid()
	{
		return drawable.get() != nullptr;
	}

protected:
	unique_ptr<DrawableComposite> drawable;

	static const String defaultSvgString;
};

class VectorGraphic::Painter
{
public:
	Painter(VectorGraphic& svgObject) : ptr(svgObject)
	{
		drawable = make_unique<DrawableComposite>(*static_cast<DrawableComposite*>(ptr.drawable.get()));
	}
	~Painter() = default;

	void draw(Graphics& g)
	{
		drawable->draw(g, opacity, transform);
	}

	void setSize(float width, float height)
	{
		area.setWidth(width);
		area.setHeight(height);

		updateTransform();
	}

	void setRotationDegrees(float v)
	{
		rotationRadians = juce::degreesToRadians(v);

		updateTransform();
	}

	void replaceColor(const Colour& colorToReplace, const Colour& newColor)
	{
		drawable->replaceColour(colorToReplace, newColor);
	}

	VectorGraphic& getVectorGraphic()
	{
		return ptr;
	}

	std::unique_ptr<Drawable> createDrawable()
	{
		return ptr.createCopy();
	}

protected:
	VectorGraphic& ptr;

	juce::Rectangle<float> area{ 0,0,30,30 };
	float rotationRadians = 0;
	float opacity = 1;

	unique_ptr<DrawableComposite> drawable;
	juce::AffineTransform transform;

	void updateTransform()
	{
		transform = RectanglePlacement(RectanglePlacement::centred).getTransformToFit(drawable->getContentArea(), area)
			.rotated(rotationRadians, area.getCentreX(), area.getCentreY());
	}
};

class ButtonBase : public ElanModulatableButton, public jura::RButtonPainter
{
public:
	ButtonBase() = default;
	~ButtonBase() = default;

	virtual void setFloatBounds(float x_, float y_, float w_, float h_, float scale = 1, juce::Point<int> offset = {0,0})
	{
		float x = x_ * scale + offset.x;
		float y = y_ * scale + offset.y;
		float w = w_ * scale;
		float h = h_ * scale;

		int floor_x = (int)floor(x);
		int floor_y = (int)floor(y);
		int ceil_w = (int)ceil(w);
		int ceil_h = (int)ceil(h);

		setBounds(floor_x, floor_y, ceil_w, ceil_h);

		hitbox = { 0, 0, ceil_w, ceil_h };

		floatLocalBounds = { x - floor_x, y - floor_y, w, h };		
	}

	void setStateForOn(int v)
	{
		stateForOn = v == 0 ? false : true;
		repaint();
	}

protected:
	float scale = 1;
	juce::Rectangle<float> floatLocalBounds;
	Rectangle<int> hitbox;
	Rectangle<int> hitboxSize;
	bool stateForOn = true;

	//void resized() override
	//{
	//	hitbox = Rectangle<int>(ElanRect::place(hitboxSize, getLocalBounds(), MidCenter, MidCenter));
	//}

	bool hitTest(int x, int y) override
	{
		return hitbox.contains(x, y);
	}
};

class TwoStateImageButton : public ButtonBase
{
public:
	TwoStateImageButton(TwoStateImage& image) : painter(image)
	{
		floatLocalBounds = getImageBounds().toFloat();
	}

	~TwoStateImageButton() = default;

	void setBounds(int x, int y, int w, int h, Anchor anchor = TopLeft)
	{		
		auto r = ElanRect::place(Rectangle<int>{ painter.getImageWidth(), painter.getImageHeight() }, Point<int>{ x, y }, anchor);
		Component::setBounds(r.getX(), r.getY(), r.getWidth(), r.getHeight());
		hitbox = ElanRect::place(Rectangle<int>{ w, h }, Rectangle<int>{ r.getWidth(), r.getHeight() }, anchor, anchor);
	}

	void setBounds(Point<int> p, int w, int h, Anchor anchor = TopLeft)
	{
		setBounds(p.x, p.y, w, h, anchor);
	}	

	Rectangle<int> getImageBounds() { return painter.getImageBounds(); }
	int getImageWidth() { return painter.getImageWidth(); }
	int getImageHeight() { return painter.getImageHeight(); }

protected:

	void paint(Graphics& g, RButton* button) override
	{
		Rectangle<float> bounds = ElanRect::place({ (float)painter.getImageWidth(), (float)painter.getImageHeight() }, button->getLocalBounds().toFloat(), MidCenter, MidCenter);

		bool state = stateForOn == true ? button->getToggleState() : !button->getToggleState();

		//painter.draw(g, bounds, state);
		painter.draw(g, floatLocalBounds, state);

		{// add red overlay to indicate modulation
			jura::ModulatableParameter* mp = dynamic_cast<jura::ModulatableParameter*> (button->getAssignedParameter());
			if (mp && mp->hasConnectedSources())
			{
				g.setColour(Colour::fromFloatRGBA(1.f, 0.f, 0.f, 0.5f));
				g.drawRect(hitbox.toFloat(), 2.5f);
			}
		}
	}

	void paint(Graphics& g) override
	{
		paint(g, this);
	}

	TwoStateImage::Painter painter;
};

class FilmstripButton : public ButtonBase
{
public:
	FilmstripButton(Filmstrip& image) : painter(image) {}

	~FilmstripButton() = default;

	void setFrameForOn(int v)
	{
		jassert(v >= 0);
		jassert(v < painter.getNumFrames());
		frameForOn = v;
	}

	void setFrameForOff(int v)
	{
		jassert(v >= 0);
		jassert(v < painter.getNumFrames());
		frameForOff = v;
	}

protected:

	void paint(Graphics& g, RButton* button) override
	{
		bool state = stateForOn == true ? button->getToggleState() : !button->getToggleState();

		//painter.draw(g, bounds, state);
		painter.draw(g, floatLocalBounds, state ? frameForOn : frameForOff);

		{// add red overlay to indicate modulation
			jura::ModulatableParameter* mp = dynamic_cast<jura::ModulatableParameter*> (button->getAssignedParameter());
			if (mp && mp->hasConnectedSources())
			{
				g.setColour(Colour::fromFloatRGBA(1.f, 0.f, 0.f, 0.5f));
				g.drawRect(hitbox.toFloat(), 2.5f);
			}
		}
	}

	void paint(Graphics& g) override
	{
		paint(g, this);
	}

	Filmstrip::Painter painter;
	int frameForOn = 1;
	int frameForOff = 0;
};

class KnobBase : public ElanModulatableSlider, public jura::RSliderPainter
{
public:
	KnobBase() = default;
	~KnobBase() = default;

	virtual void setFloatBounds(float x_, float y_, float w_, float h_, float scale = 1, juce::Point<int> offset = { 0,0 })
	{
		float x = x_ * scale + offset.x;
		float y = y_ * scale + offset.y;
		float w = w_ * scale;
		float h = h_ * scale;

		int floor_x = (int)floor(x);
		int floor_y = (int)floor(y);
		int ceil_w = (int)ceil(w);
		int ceil_h = (int)ceil(h);

		setBounds(floor_x, floor_y, ceil_w, ceil_h);

		hitbox = { 0, 0, ceil_w, ceil_h };

		floatLocalBounds = { x - floor_x, y - floor_y, w, h };
	}

protected:
	Rectangle<int> hitbox;
	Rectangle<int> hitboxSize;
	juce::Rectangle<float> floatLocalBounds;

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
			y = clamp(y, 0.0, 1.0);

			setNormalizedValue(y);
		}

		ParameterInfo::InfoSender::updateParameterInfo();
	}
};

class FilmStripKnob : public KnobBase
{
public:
	FilmStripKnob(Filmstrip& image) : painter(image) {}
	~FilmStripKnob() = default;

	Filmstrip::Painter* getPainter() { return &painter; }

	Rectangle<int> getImageBounds() { return painter.getFrameBounds(); }
	int getImageWidth() { return painter.getFrameWidth(); }
	int getImageHeight() { return painter.getFrameHeight(); }

	void setBounds(int x, int y, int w, int h, Anchor anchor = TopLeft)
	{
		hitboxSize = { w, h };
		Component::setBounds(ElanRect::place(Rectangle<int>{ getImageWidth(), getImageHeight() }, Point<int>{ x, y }, anchor));
	}

	void setBounds(Point<int> p, int w, int h, Anchor anchor = TopLeft)
	{
		setBounds(p.x, p.y, w, h, anchor);
	}

protected:
	void paint(Graphics& g, RSlider* slider) override
	{
		painter.draw(g, slider->getLocalBounds().toFloat(), (float)slider->getNormalizedValue());

		{// add red overlay to indicate modulation
			jura::ModulatableParameter* mp = dynamic_cast<jura::ModulatableParameter*> (slider->getAssignedParameter());
			if (mp && mp->hasConnectedSources())
			{
				g.setColour(Colour::fromFloatRGBA(1.f, 0.f, 0.f, 0.5f));
				g.drawRect(slider->getLocalBounds().toFloat(), 2.5f);
			}
		}
	}

	void paint(Graphics& g) override
	{
		paint(g, this);
	}

	void resized() override
	{
		hitbox = Rectangle<int>(ElanRect::place(hitboxSize, getLocalBounds(), MidCenter, MidCenter));
	}

	bool hitTest(int x, int y) override
	{
		return hitbox.contains(x, y);
	}

	Filmstrip::Painter painter;
};

class VectorKnob : public KnobBase
{
public:
	VectorKnob(VectorGraphic& image) : painter(image) {}
	~VectorKnob() = default;

	VectorGraphic::Painter* getPainter() { return &painter; }

	void setBounds(int x, int y, int w, int h, Anchor anchor = TopLeft)
	{
		hitboxSize = { w, h };
		Component::setBounds(ElanRect::place(Rectangle<int>{ w, h }, Point<int>{ x, y }, anchor));
	}

	void setBounds(Point<int> p, int w, int h, Anchor anchor = TopLeft)
	{
		setBounds(p.x, p.y, w, h, anchor);
	}

protected:
	void paint(Graphics& g, RSlider* slider) override
	{
		painter.setSize((float)slider->getLocalBounds().getWidth(), (float)slider->getLocalBounds().getHeight());
		painter.setRotationDegrees(juce::jmap<float>((float)getNormalizedValue(), -135, +135));
		painter.draw(g);

		{// add red overlay to indicate modulation
			jura::ModulatableParameter* mp = dynamic_cast<jura::ModulatableParameter*> (slider->getAssignedParameter());
			if (mp && mp->hasConnectedSources())
			{
				g.setColour(Colour::fromFloatRGBA(1.f, 0.f, 0.f, 0.5f));
				g.drawRect(slider->getLocalBounds().toFloat(), 2.5f);
			}
		}
	}

	void paint(Graphics& g) override
	{
		paint(g, this);
	}

	void resized() override
	{
		hitbox = Rectangle<int>(ElanRect::place(hitboxSize, getLocalBounds(), MidCenter, MidCenter));
	}

	bool hitTest(int x, int y) override
	{
		return hitbox.contains(x, y);
	}

	VectorGraphic::Painter painter;
};
