#pragma once

#include "FCFBinaryData.h"

using namespace se;

struct FCFResources : public ResourcesBase
{
	struct
	{
		OneStateImage background{ FCFBinaryData::backround_2x_png, FCFBinaryData::backround_2x_pngSize };
		OneStateImage smallknob{ FCFBinaryData::knob_small_png, FCFBinaryData::knob_small_pngSize };
		OneStateImage bigknob{ FCFBinaryData::knob_big_png, FCFBinaryData::knob_big_pngSize };
		Filmstrip donateButton{ FCFBinaryData::button_donate_png, FCFBinaryData::button_donate_pngSize, 124, 52 };
		OneStateImage inputBox_bottom{ FCFBinaryData::valuebox_bottom_png, FCFBinaryData::valuebox_bottom_pngSize };
		TwoStateImage monoButton
		{
			FCFBinaryData::btn_mono_disabled_2x_png, FCFBinaryData::btn_mono_disabled_2x_pngSize,
			FCFBinaryData::btn_mono_enabled_1_2x_png, FCFBinaryData::btn_mono_enabled_1_2x_pngSize
		};
		TwoStateImage stereoButton
		{
			FCFBinaryData::btn_stereo_disabled_2x_png, FCFBinaryData::btn_stereo_disabled_2x_pngSize,
			FCFBinaryData::btn_stereo_enabled_2x_png, FCFBinaryData::btn_stereo_enabled_2x_pngSize
		};
		Filmstrip leftTrigAlwaysButton{ FCFBinaryData::btn_left_trigger_always_64x44_2x_png, FCFBinaryData::btn_left_trigger_always_64x44_2x_pngSize, 2, true };
		Filmstrip leftTrigLegatoButton{ FCFBinaryData::btn_left_trigger_legato_64x22_2x_png, FCFBinaryData::btn_left_trigger_legato_64x22_2x_pngSize, 2, true };
		Filmstrip RightTrigAlwaysButton{ FCFBinaryData::btn_right_trigger_always_64x22_2x_png, FCFBinaryData::btn_right_trigger_always_64x22_2x_pngSize, 2, true };
		Filmstrip RightTrigLegatoButton{ FCFBinaryData::btn_right_trigger_legato_64x22_2x_png, FCFBinaryData::btn_right_trigger_legato_64x22_2x_pngSize, 2, true };
		Filmstrip envModeButton{ FCFBinaryData::fcf_modes_buttons_png, FCFBinaryData::fcf_modes_buttons_pngSize, 2, true };
		Filmstrip efAdsr{ FCFBinaryData::ef_adsr_section_2x_png, FCFBinaryData::ef_adsr_section_2x_pngSize, 4, false };
	} image;

	struct
	{
		VectorGraphic smallknob_pointer{ FCFBinaryData::knob_small_ringpointer_svg };
		VectorGraphic bigknob_pointer{ FCFBinaryData::knob_big_ringpointer_svg };
		VectorGraphic saveicon{ FCFBinaryData::save_icon_svg };
		VectorGraphic loadicon{ FCFBinaryData::load_icon_svg };
		VectorGraphic usericon{ FCFBinaryData::user_icon_svg };
		VectorGraphic factoryicon{ FCFBinaryData::factory_icon_svg };
		VectorGraphic sendKnob{ FCFBinaryData::knob_send_svg };
	} vector;

	struct
	{
		FontContainer ArialBold{ Font("Arial Unicode MS", 12, Font::bold) };
		FontContainer TekoMedium{ FCFBinaryData::TekoMedium_ttf, FCFBinaryData::TekoMedium_ttfSize };
		FontContainer TekoLight{ FCFBinaryData::TekoLight_ttf, FCFBinaryData::TekoLight_ttfSize };
		FontContainer RobotoBold{ FCFBinaryData::RobotoBold_ttf, FCFBinaryData::RobotoBold_ttfSize };
		FontContainer RobotoRegular{ FCFBinaryData::RobotoRegular_ttf, FCFBinaryData::RobotoRegular_ttfSize };
		FontContainer ShareTechMonoRegular{ FCFBinaryData::ShareTechMonoRegular_ttf, FCFBinaryData::ShareTechMonoRegular_ttfSize };
	} font;

	struct
	{
		Colour border{ 69, 71, 97 };
		Colour hoverFill{ 64, 64, 92 };
		Colour background{ 85, 85, 122 };
		Colour font{ 211, 211, 213 };
		Colour normal{ 162, 162, 203 };
		Colour hover{ 181, 181, 213 };
		Colour clicked{ 122, 122, 152 };
	} colors;
};

class FCFResourcePointer
{
protected:
	SharedResourcePointer<FCFResources> RESOURCES;
};

class Knob : public ElanModulatableSlider, public FCFResourcePointer
{
public:
	String label = "LABEL";
	juce::Rectangle<int> hitbox;

protected:
	juce::Rectangle<float> floatLocalBounds;

	float scale = 1.0;

	bool hitTest(int x, int y) override
	{
		return hitbox.contains(x, y);
	}

	void resized() override
	{
		int s = getWidth();
		hitbox = { s,s };
	}

	void mouseDrag(const MouseEvent & e) override
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

class BigKnob : public Knob
{
public:
	int imageWidth = RESOURCES->image.bigknob.getImageWidth();
	int imageHeight = RESOURCES->image.bigknob.getImageHeight();
	float ratioScale = imageWidth / (float)imageHeight;

	void setFloatBounds(float x_, float y_, float _w)
	{
		scale = dynamic_cast<BasicEditor*>(getParentComponent())->getScale();
		auto offset = dynamic_cast<BasicEditor*>(getParentComponent())->getOffset();

		float x = x_ * scale + offset.x;
		float y = y_ * scale + offset.y;
		//float w = imageWidth * scale * 0.5f;
		//float h = imageHeight * scale * 0.5f;

		auto imageScale = _w / imageWidth;
		auto newImageHeight = imageHeight * imageScale;
		float w = _w * scale;
		float h = newImageHeight * scale;

		int floor_x = (int)floor(x);
		int floor_y = (int)floor(y);
		int ceil_w = (int)ceil(w);
		int ceil_h = (int)ceil(h);

		setBounds(floor_x, floor_y, ceil_w, ceil_h);

		floatLocalBounds = { x - floor_x, y - floor_y, w, h };
	}

protected:
	void paint(Graphics & g) override
	{
		float currentValueToDegrees = juce::jmap((float)getNormalizedValue(), 0.f, 1.f, -135.f, +135.f);

		RESOURCES->image.bigknob.draw(g, floatLocalBounds);

		juce::Rectangle<float> pointerArea = { floatLocalBounds.getX(), floatLocalBounds.getY(), floatLocalBounds.getWidth(), floatLocalBounds.getWidth() };
		RESOURCES->vector.bigknob_pointer.drawWithRotation(g, pointerArea, currentValueToDegrees);
	}
};

class AlgorithmDropdownSkin : public LookAndFeel_V4, public FCFResourcePointer
{
public:
	void setSize(int w, int h)
	{
		width = w;
		height = h;
	}

	void setScale(float v)
	{
		scale = v;
	}

protected:
	float scale = 1.0;
	int width = 0;
	int height = 0;

	void drawPopupMenuBackground(Graphics& g, int, int) override
	{
		g.fillAll({ 38, 38, 48 });
	}

	Font getPopupMenuFont() override
	{
		return RESOURCES->font.RobotoBold.withPointHeight(10.f * scale);
	}

	void getIdealPopupMenuItemSize(const String& text, bool isSeparator, int standardMenuItemHeight, int& idealWidth, int& idealHeight) override
	{
		idealWidth = width;
		idealHeight = height;
	}

	int getPopupMenuBorderSize() override { return 0; }

	void drawPopupMenuItem(Graphics& g, const Rectangle<int>& area,
		bool isSeparator, bool isActive,
		bool isHighlighted, bool isTicked,
		bool hasSubMenu, const String& text,
		const String& shortcutKeyText,
		const Drawable* icon, const Colour* textColourToUse) override
	{
		auto textColour = (textColourToUse == nullptr ? findColour(PopupMenu::textColourId) : *textColourToUse);

		auto r = area.reduced(1);

		if (isHighlighted && isActive)
		{
			g.setColour(findColour(PopupMenu::highlightedBackgroundColourId));
			g.fillRect(r);

			g.setColour(findColour(PopupMenu::highlightedTextColourId));
		}
		else
		{
			g.setColour(textColour.withMultipliedAlpha(isActive ? 1.0f : 0.5f));
		}
		
		r.removeFromLeft(10);
		r.removeFromRight(3);
		g.setColour({ 204, 204, 204 });

		g.setFont(getPopupMenuFont());

		g.drawText(text, r, Justification::centredLeft, 1);

	}
};

class Dropdown : public ElanModulatableComboBox, public FCFResourcePointer
{
public:
	Dropdown()
	{
		fontPainter.setOffset(0.f, 1.f);
	}

	void setFloatBounds(float x_, float y_, float w_, float h_)
	{
		scale = dynamic_cast<BasicEditor*>(getParentComponent())->getScale();
		auto offset = dynamic_cast<BasicEditor*>(getParentComponent())->getOffset();

		float x = x_ * scale + offset.x;
		float y = y_ * scale + offset.y;
		float w = w_ * scale;
		float h = h_ * scale;

		int floor_x = (int)floor(x);
		int floor_y = (int)floor(y);
		int ceil_w = (int)ceil(w);
		int ceil_h = (int)ceil(h);

		setBounds(floor_x, floor_y, ceil_w, ceil_h);

		floatLocalBounds = { x - floor_x, y - floor_y, w, h };
	}

protected:
	float scale = 1;
	juce::Rectangle<float> floatLocalBounds;
	FontContainer::Painter fontPainter{ RESOURCES->font.RobotoBold };

	void paint(Graphics& g) override
	{
		auto area = floatLocalBounds;

		g.setColour({ 148, 143, 129 });
		auto buttonArea = area.removeFromRight(24.f * scale);
		g.fillRect(buttonArea);

		g.setColour({ 204, 204, 204 });
		Path p = ElanPath::v(ElanRect::place({ 7.3f * scale, 3.7f * scale }, buttonArea, MidCenter, MidCenter), 2.6f * scale, Edge::Bottom);
		g.fillPath(p);

		g.setColour({ 51, 51, 64 });
		g.fillRect(area);

		g.setColour({ 204, 204, 204 });

		fontPainter.setFontSize(9.5f * scale);
		fontPainter.setFontAndDrawText(g, getStateAsString(), area);
	}

	void parameterChanged(Parameter* p) override
	{
		int algo = getAssignedParameter()->getValue();
		switch (algo)
		{
		default:
		case 0:
			ParameterInfo::InfoSender::description = "";
			break;
		case 1:
		case 2:
			ParameterInfo::InfoSender::description = "Flower Child was developed by the United States in partnership with Japan during the filter wars. Revision 1 has a peace-loving sound. Revision 2 does not.";
			break;
		case 3:
			ParameterInfo::InfoSender::description = "Sawtooth Resonator was developed by the Soviet Union as a filter of mass destruction for the invasion of Europe. 'The destruction of Europe is coming.' ";
			break;
		case 4:
			ParameterInfo::InfoSender::description = "Superlove Highpass description here, 'quote here.' ";
			break;
		}
		rsAutomatableComboBox::parameterChanged(p);
		ParameterInfo::InfoSender::updateParameterInfo();
	}
};

class SmallKnob : public Knob
{
public:
	int imageWidth = RESOURCES->image.smallknob.getImageWidth();
	int imageHeight = RESOURCES->image.smallknob.getImageHeight();
	float ratioScale = imageWidth / (float)imageHeight;

	void setFloatBounds(float x_, float y_)
	{
		scale = dynamic_cast<BasicEditor*>(getParentComponent())->getScale();
		auto offset = dynamic_cast<BasicEditor*>(getParentComponent())->getOffset();

		float x = x_ * scale + offset.x;
		float y = y_ * scale + offset.y;
		float w = imageWidth * scale * 0.5f;
		float h = imageHeight * scale * 0.5f;

		int floor_x = (int)floor(x);
		int floor_y = (int)floor(y);
		int ceil_w = (int)ceil(w);
		int ceil_h = (int)ceil(h);

		setBounds(floor_x, floor_y, ceil_w, ceil_h);

		floatLocalBounds = { x - floor_x, y - floor_y, w, h };
	}

protected:
	void paint(Graphics & g) override
	{
		float currentValueToDegrees = juce::jmap((float)getNormalizedValue(), 0.f, 1.f, -135.f, +135.f);

		RESOURCES->image.smallknob.draw(g, floatLocalBounds);

		juce::Rectangle<float> pointerArea = { floatLocalBounds.getX(), floatLocalBounds.getY(), floatLocalBounds.getWidth(), floatLocalBounds.getWidth() };
		RESOURCES->vector.bigknob_pointer.drawWithRotation(g, pointerArea, currentValueToDegrees);
	}
};

class ArcKnob : public Knob
{
public:
	void setFloatBounds(float x_, float y_, float w_, float arcThickness_)
	{
		arcThickness = arcThickness_;

		scale = dynamic_cast<BasicEditor*>(getParentComponent())->getScale();
		auto offset = dynamic_cast<BasicEditor*>(getParentComponent())->getOffset();

		float x = x_ * scale + offset.x;
		float y = y_ * scale + offset.y;
		float w = w_ * scale;

		int floor_x = (int)floor(x);
		int floor_y = (int)floor(y);
		int ceil_w = (int)ceil(w);

		setBounds(floor_x, floor_y, ceil_w, ceil_w);

		floatLocalBounds = { x - floor_x, y - floor_y, w, w };

		arcFill.setBounds(floatLocalBounds.reduced(0.5,0.5));
	}

	void paint(Graphics & g)
	{
		float maxFillDegrees = 130;
		float centerFillDegrees = 0;
		float minFillDegress = -maxFillDegrees;
		float remainingFillDegrees = 360 - maxFillDegrees;
		float currentFillDegrees = jmap((float)getNormalizedValue(), minFillDegress, maxFillDegrees);

		bool isBipolar = getMinimum() < 0 && getMaximum() > 0;

		if (isBipolar)
		{
			if (currentFillDegrees < centerFillDegrees)
			{
				arcFill.setArcThickness(arcThickness * scale);
				arcFill.setMinMaxRotationDegrees(minFillDegress, currentFillDegrees);
				g.setColour({ 122, 124, 217 });
				arcFill.draw(g);

				arcFill.setArcThickness(arcThickness * scale);
				arcFill.setMinMaxRotationDegrees(currentFillDegrees, centerFillDegrees);
				g.setColour({ 237, 234, 229 });
				arcFill.draw(g);

				arcFill.setArcThickness(arcThickness * scale);
				arcFill.setMinMaxRotationDegrees(centerFillDegrees, maxFillDegrees);
				g.setColour({ 122, 124, 217 });
				arcFill.draw(g);

				arcFill.setArcThickness(arcThickness * scale);
				arcFill.setMinMaxRotationDegrees(maxFillDegrees, remainingFillDegrees);
				g.setColour(Colour({ 61, 62, 109 }).withAlpha(0.8f));
				arcFill.draw(g);
			}
			else
			{
				arcFill.setArcThickness(arcThickness * scale);
				arcFill.setMinMaxRotationDegrees(minFillDegress, centerFillDegrees);
				g.setColour({ 122, 124, 217 });
				arcFill.draw(g);

				arcFill.setArcThickness(arcThickness * scale);
				arcFill.setMinMaxRotationDegrees(centerFillDegrees, currentFillDegrees);
				g.setColour({ 237, 234, 229 });
				arcFill.draw(g);

				arcFill.setArcThickness(arcThickness * scale);
				arcFill.setMinMaxRotationDegrees(currentFillDegrees, maxFillDegrees);
				g.setColour({ 122, 124, 217 });
				arcFill.draw(g);

				arcFill.setArcThickness(arcThickness * scale);
				arcFill.setMinMaxRotationDegrees(maxFillDegrees, remainingFillDegrees);
				g.setColour(Colour({ 61, 62, 109 }).withAlpha(0.8f));
				arcFill.draw(g);
			}
		}
		else
		{
			arcFill.setArcThickness(arcThickness * scale);
			arcFill.setMinMaxRotationDegrees(minFillDegress, currentFillDegrees);
			g.setColour({ 237, 234, 229 });
			arcFill.draw(g);

			arcFill.setArcThickness(arcThickness * scale);
			arcFill.setMinMaxRotationDegrees(currentFillDegrees, maxFillDegrees);
			g.setColour({ 122, 124, 217 });
			arcFill.draw(g);

			arcFill.setArcThickness(arcThickness * scale);
			arcFill.setMinMaxRotationDegrees(maxFillDegrees, remainingFillDegrees);
			g.setColour(Colour({ 61, 62, 109 }).withAlpha(0.8f));
			arcFill.draw(g);
		}
	}

	ArcPainter arcFill;
	float arcThickness = 4.8f;
};

class OversamplingArcKnob : public ArcKnob
{
	void paint(Graphics & g)
	{
		ArcKnob::paint(g);

		g.setColour({ 237, 234, 229 });
		g.setFont(RESOURCES->font.TekoMedium.withPointHeight(12.f * scale));
		g.drawText(ParameterInfo::InfoSender::valueStringFunction(), getLocalBounds().toFloat().translated(0.f, +1.f * scale), Justification::centred);
	}
};

class MonoStereoButton : public ElanModulatableButton, public FCFResourcePointer
{
protected:
	float scale = 1;
	juce::Rectangle<float> floatLocalBounds;

public:
	void setFloatBounds(float x_, float y_, float w_, float h_)
	{
		scale = dynamic_cast<BasicEditor*>(getParentComponent())->getScale();
		auto offset = dynamic_cast<BasicEditor*>(getParentComponent())->getOffset();

		float x = x_ * scale + offset.x;
		float y = y_ * scale + offset.y;
		float w = w_ * scale;
		float h = h_ * scale;

		int floor_x = (int)floor(x);
		int floor_y = (int)floor(y);
		int ceil_w = (int)ceil(w);
		int ceil_h = (int)ceil(h);

		setBounds(floor_x, floor_y, ceil_w, ceil_h);

		floatLocalBounds = { x - floor_x, y - floor_y, w, h };
	}

	void paint(Graphics & g)
	{
		auto area = floatLocalBounds;

		auto monoCircleArea = ElanRect::setSize(area.removeFromTop(14.f * scale), 14.f * scale, 14.f * scale, Anchor::MidCenter);
		auto stereoCircleArea = ElanRect::setSize(area.removeFromBottom(14.f * scale), 26.f * scale, 14.f * scale, Anchor::MidCenter);

		if (isOn)
		{
			monoButton.draw(g, monoCircleArea, 0);
			stereoButton.draw(g, stereoCircleArea, 1);
		}
		else
		{
			monoButton.draw(g, monoCircleArea, 1);
			stereoButton.draw(g, stereoCircleArea, 0);
		}
	}

	TwoStateImage::Painter monoButton{ RESOURCES->image.monoButton };
	TwoStateImage::Painter stereoButton{ RESOURCES->image.stereoButton };
};

class DonateButton : public LeftClickButton, public FCFResourcePointer, public ParameterInfo::InfoSender
{
public:
	DonateButton()
	{
		clickFunction = []() { URL("http://paypal.me/soundemotelabs").launchInDefaultBrowser(); };
		paintFunction = [this](Graphics & g)
		{
			if (isDown())
				filmstrip.draw(g, getLocalBounds().toFloat(), 2);
			else if (isMouseOver())
				filmstrip.draw(g, getLocalBounds().toFloat(), 1);
			else
				filmstrip.draw(g, getLocalBounds().toFloat(), 0);
		};
	}

	void setFloatBounds(float x, float y)
	{
		auto scale = dynamic_cast<BasicEditor*>(getParentComponent())->getScale();
		auto offset = dynamic_cast<BasicEditor*>(getParentComponent())->getOffset();

		LeftClickButton::setBounds(int(x * scale) + offset.x, int(y * scale) + offset.y, int(filmstrip.getFrameWidth() * scale * 0.5), int(filmstrip.getFrameHeight() * scale * 0.5));
	}

protected:
	void mouseEnter(const MouseEvent& e) override
	{
		LeftClickButton::mouseEnter(e);
		ParameterInfo::InfoSender::updateParameterInfo();
	}

	Filmstrip::Painter filmstrip{ RESOURCES->image.donateButton };
};

class FCFValueEntry : public Component, public FCFResourcePointer
{
public:
	FCFValueEntry()
	{
		addAndMakeVisible(editor);
		editor.setColour(TextEditor::backgroundColourId, Colours::transparentBlack);
		editor.setColour(TextEditor::outlineColourId, Colours::transparentWhite);
		editor.setColour(TextEditor::focusedOutlineColourId, Colours::transparentWhite);
		editor.setColour(TextEditor::textColourId, Colours::white);
		editor.setColour(TextEditor::highlightedTextColourId, Colours::white);
		editor.setColour(TextEditor::highlightColourId, Colours::white.withAlpha(0.3f));
		editor.setColour(CaretComponent::caretColourId, Colours::white);
		editor.setInputRestrictions(10, "-+.0123456789");
	}
	~FCFValueEntry() = default;

	TextEditor editor;

	void setBoundsToComponent(Component * component)
	{
		auto scale = dynamic_cast<BasicEditor*>(getParentComponent())->getScale();
		auto bounds = component->getBounds();

		juce::Rectangle<int> valueEntryBounds = { int(RESOURCES->image.inputBox_bottom.getImageWidth() * 0.5f * scale), int(RESOURCES->image.inputBox_bottom.getImageHeight() * 0.5f * scale) };
		valueEntryBounds = ElanRect::place(valueEntryBounds, bounds, TopCenter, BottomCenter);

		setBounds(valueEntryBounds);
		editor.setBounds(ElanRect::pad(getLocalBounds(), int(2.f * scale), int(7.f * scale), int(2.f * scale), int(0.f * scale)));
		editor.applyFontToAllText(RESOURCES->font.RobotoRegular.withPointHeight(11.f * scale));
	}

protected:

	Font font;

	void paint(Graphics& g) override
	{
		RESOURCES->image.inputBox_bottom.draw(g, getLocalBounds().toFloat());
	}
};

class FCFPresetBarDropdownSkin : public PresetBarDropdownSkin, public FCFResourcePointer
{
protected:

	void drawPopupMenuBackground(Graphics& g, int, int) override
	{
		g.fillAll({ 38, 38, 48 });
	}

	Font getPopupMenuFont() override
	{
		return RESOURCES->font.RobotoBold.withPointHeight(10.f * scale);
	}

	void getIdealPopupMenuItemSize(const String& text, bool isSeparator, int standardMenuItemHeight, int& idealWidth, int& idealHeight) override
	{
		if (isSeparator)
		{
			idealWidth = 50;
			idealHeight = int(7 * scale);
		}
		else
		{
			auto font = getPopupMenuFont();

			if (standardMenuItemHeight > 0 && font.getHeight() > standardMenuItemHeight / 1.3f)
				font.setHeight(standardMenuItemHeight / 1.3f);

			idealHeight = standardMenuItemHeight > 0 ? standardMenuItemHeight : roundToInt(font.getHeight() * 1.3f);
			idealWidth = font.getStringWidth(text) + idealHeight * 2;
		}
	}

	int getPopupMenuBorderSize() override { return 0; }

	void drawPopupMenuItem(Graphics& g, const Rectangle<int>& area,
		bool isSeparator, bool isActive,
		bool isHighlighted, bool /*isTicked*/,
		bool /*hasSubMenu*/, const String& text,
		const String& /*shortcutKeyText*/,
		const Drawable* icon, const Colour* textColourToUse) override
	{
		auto textColour = (textColourToUse == nullptr ? findColour(PopupMenu::textColourId) : *textColourToUse);

		auto r = area.reduced(1);

		if (isHighlighted && isActive)
		{
			g.setColour({ 160, 43, 73 });
			if (!isSeparator)
				g.fillRect(r);

			g.setColour(findColour(PopupMenu::highlightedTextColourId));
		}
		else
		{
			g.setColour(textColour.withMultipliedAlpha(isActive ? 1.0f : 0.5f));
		}

		if (isSeparator)
		{
			g.setColour({ 85, 85, 121 });
			g.drawLine((float)area.getX(), (float)area.getCentreY(), (float)area.getWidth(), (float)area.getCentreY(), 1.f * scale);
		}
		else
		{
			if (icon != nullptr)
			{
				auto iconArea = r.removeFromLeft(area.getHeight()).toFloat();
				iconArea = ElanRect::fit(icon->getDrawableBounds(), iconArea.withSizeKeepingCentre(11 * scale, 11 * scale), MidCenter, MidCenter);
				unique_ptr<Drawable> iconToUse = unique_ptr<Drawable>(icon->createCopy());
				if (isHighlighted)
				{
					iconToUse.get()->replaceColour({ 176, 128, 191 }, { 205, 205, 205 });
					iconToUse.get()->replaceColour({ 116, 156, 105 }, { 205, 205, 205 });
					iconToUse.get()->replaceColour(colors.normal, { 205, 205, 205 });
				}
				iconToUse.get()->drawWithin(g, iconArea.toNearestInt().toFloat(), RectanglePlacement(RectanglePlacement::centred), 1.f);
			}

			g.setFont(getPopupMenuFont());
			r.removeFromRight(3);
			g.setColour({ 204, 204, 204 });
			g.drawText(text, r, Justification::centredLeft, 1);
		}
	}
};

class FlowerChildFilterEditor : public BasicEditor, public ParameterInfo::Listener, public FCFResourcePointer, PluginFileManager::FileMemory
{
public:
	FlowerChildFilterEditor(FlowerChildFilterModule * module);

	virtual ~FlowerChildFilterEditor() = default;

	// overriden callbacks
	void resized() override;
	void rButtonClicked(RButton* button) override;
	void updateWidgetsAccordingToState() override;

	PluginFileManager pluginFileManager;

	FlowerChildFilterModule * moduleToEdit;

	Dropdown algorithm_combobox;
	AlgorithmDropdownSkin algorithmDropdownSkin;

	BigKnob cutoff_knob;
	SmallKnob resonance_knob;
	SmallKnob chaos_knob;
	SmallKnob chaos2_knob;
	SmallKnob input_knob;
	SmallKnob output_knob;
	SmallKnob drywet_knob;

	ArcKnob freqAttack_knob;
	ArcKnob freqHold_knob;
	ArcKnob freqDecay_knob;	

	ArcKnob ampAttack_knob;
	ArcKnob ampHold_knob;
	ArcKnob ampDecay_knob;

	ArcKnob adsrFreqAttack_knob;
	ArcKnob adsrFreqDecay_knob;
	ArcKnob adsrFreqSustain_knob;
	ArcKnob adsrFreqRelease_knob;

	ArcKnob adsrFreqAttackCurve_knob;
	ArcKnob adsrFreqDecayCurve_knob;
	ArcKnob adsrFreqReleaseCurve_knob;

	ArcKnob adsrAmpAttack_knob;
	ArcKnob adsrAmpDecay_knob;
	ArcKnob adsrAmpSustain_knob;
	ArcKnob adsrAmpRelease_knob;

	ArcKnob adsrAmpAttackCurve_knob;
	ArcKnob adsrAmpDecayCurve_knob;
	ArcKnob adsrAmpReleaseCurve_knob;

	VectorKnob freqSend_knob{ RESOURCES->vector.sendKnob };
	VectorKnob ampSend_knob{ RESOURCES->vector.sendKnob };

	OversamplingArcKnob oversampling_knob;
	ArcKnob smoothing_knob;

	MonoStereoButton monoStereo_button;

	FilmstripButton freqEnvMode_button{ RESOURCES->image.envModeButton };
	FilmstripButton ampEnvMode_button{ RESOURCES->image.envModeButton };

	FilmstripButton freqAdsrResetModeAlways_button{ RESOURCES->image.leftTrigAlwaysButton };
	FilmstripButton freqAdsrResetModeLegato_button{ RESOURCES->image.leftTrigLegatoButton };
	FilmstripButton ampAdsrResetModeAlways_button{ RESOURCES->image.RightTrigAlwaysButton };
	FilmstripButton ampAdsrResetModeLegato_button{ RESOURCES->image.RightTrigLegatoButton };

	// Help and Value Display
	ParameterInfo parameterInfo;
	StaticText parameterName_text;
	StaticText parameterValue_text;
	StaticText parameterHelpDescription_text;
	StaticText parameterHelpName_text;
	juce::Rectangle<int> HELPStringTextArea;

	// Value Input
	FCFValueEntry valueEntry;
	ParameterValueInput parameterValueInput{ this, &valueEntry, valueEntry.editor };

	//Oscilloscope
	OscilloscopeEditor* oscilloscopeEditor;
	Component oscilloscopeArea;

	DonateButton donate_button;
	
	LeftClickButton burgerMenu_button;
	FCFPresetBarDropdownSkin presetDropdownSkin;

	PresetBar presetBar;

	int presetBarHeight = 20;

	int width_x1_scale = int(RESOURCES->image.background.getImageWidth() / 2.f);
	int height_x1_scale = int(presetBarHeight + RESOURCES->image.background.getImageHeight() / 2.f);
	float width_height_ratio = height_x1_scale / (float)width_x1_scale;

protected:

	void paint(Graphics & g) override
	{
		g.fillAll({ 16, 15, 28 });

		RESOURCES->image.background.draw(g, (Rectangle<float>{0.f, 0.f, 640.f, 360.f}*scale).translated((float)offset.x, (float)offset.y));

		RESOURCES->image.efAdsr.draw(g, (Rectangle<float>{0.f, 56.f, 150.f, 200.f}*scale).translated((float)offset.x, (float)offset.y), freqEnvMode_button.getToggleState());
		RESOURCES->image.efAdsr.draw(g, (Rectangle<float>{490.f, 56.f, 150.f, 200.f}*scale).translated((float)offset.x, (float)offset.y), ampEnvMode_button.getToggleState() + 2);

		g.setColour({ 204, 204, 204 });
		g.setFont(RESOURCES->font.TekoMedium.withPointHeight(12.f * scale));
		g.drawText(chaos_knob.label, ElanRect::place(Rectangle<float>{ 82, 26 }*scale, chaos_knob.getBounds().toFloat(), BottomCenter, TopCenter), juce::Justification::centred);
	};

	void paintOverChildren(Graphics& g) override {}

	void infoComponentChanged() override
	{
		parameterName_text.setText(ParameterInfo::Listener::getParameterInfoState().name);
		parameterValue_text.setText(ParameterInfo::Listener::getParameterInfoState().value);
		parameterHelpName_text.setText(ParameterInfo::Listener::getParameterInfoState().name.toUpperCase());
		parameterHelpDescription_text.setText(ParameterInfo::Listener::getParameterInfoState().description);
	}

	void mouseEnter(const MouseEvent& event) override
	{
		if (oscilloscopeEditor->isMouseOverOrDragging(true))
			oscilloscopeEditor->setWidgetsToHideForMouseover(true);
		else
			oscilloscopeEditor->setWidgetsToHideForMouseover(false);

		if (event.eventComponent->getProperties().contains("BubbleHelp"))
		{
			BubbleMessageComponent* bubbleMessage = new BubbleMessageComponent();
			bubbleMessage->setAllowedPlacement(2);
			addChildComponent(bubbleMessage);
			bubbleMessage->showAt(event.eventComponent, AttributedString(event.eventComponent->getProperties().getWithDefault("BubbleHelp", "")), 1000, true, true);
		}
	}

	void mouseExit(const MouseEvent& event) override
	{
		if (!oscilloscopeEditor->isMouseOverOrDragging(true))
			oscilloscopeEditor->setWidgetsToHideForMouseover(false);
	}

	void mouseUp(const MouseEvent &event) override
	{
		if (!oscilloscopeEditor->isMouseOverOrDragging(true))
			oscilloscopeEditor->setWidgetsToHideForMouseover(false);
	}

	void mouseDrag(const MouseEvent &event) override
	{
		if (!oscilloscopeEditor->isMouseOverOrDragging(true))
			oscilloscopeEditor->setWidgetsToHideForMouseover(false);
	}

	void setComponentVisibilityAndText()
	{
		freqHold_knob.setVisible(!freqEnvMode_button.getToggleState());
		freqAttack_knob.setVisible(!freqEnvMode_button.getToggleState());
		freqDecay_knob.setVisible(!freqEnvMode_button.getToggleState());

		adsrFreqAttack_knob.setVisible(freqEnvMode_button.getToggleState());
		adsrFreqDecay_knob.setVisible(freqEnvMode_button.getToggleState());
		adsrFreqSustain_knob.setVisible(freqEnvMode_button.getToggleState());
		adsrFreqRelease_knob.setVisible(freqEnvMode_button.getToggleState());

		adsrFreqAttackCurve_knob.setVisible(freqEnvMode_button.getToggleState());
		adsrFreqDecayCurve_knob.setVisible(freqEnvMode_button.getToggleState());
		adsrFreqReleaseCurve_knob.setVisible(freqEnvMode_button.getToggleState());

		ampHold_knob.setVisible(!ampEnvMode_button.getToggleState());
		ampAttack_knob.setVisible(!ampEnvMode_button.getToggleState());
		ampDecay_knob.setVisible(!ampEnvMode_button.getToggleState());

		adsrAmpAttack_knob.setVisible(ampEnvMode_button.getToggleState());
		adsrAmpDecay_knob.setVisible(ampEnvMode_button.getToggleState());
		adsrAmpSustain_knob.setVisible(ampEnvMode_button.getToggleState());
		adsrAmpRelease_knob.setVisible(ampEnvMode_button.getToggleState());

		adsrAmpAttackCurve_knob.setVisible(ampEnvMode_button.getToggleState());
		adsrAmpDecayCurve_knob.setVisible(ampEnvMode_button.getToggleState());
		adsrAmpReleaseCurve_knob.setVisible(ampEnvMode_button.getToggleState());

		freqAdsrResetModeAlways_button.setVisible(freqEnvMode_button.getToggleState());
		ampAdsrResetModeAlways_button.setVisible(ampEnvMode_button.getToggleState());
		freqAdsrResetModeLegato_button.setVisible(freqEnvMode_button.getToggleState());
		ampAdsrResetModeLegato_button.setVisible(ampEnvMode_button.getToggleState());

		int algo = algorithm_combobox.getAssignedParameter()->getValue();
		switch (algo)
		{
		default:
		case 0:
			chaos_knob.label = "<nullptr>";
			chaos_knob.setVisible(false);
			chaos2_knob.setVisible(false);
			break;
		case 1:
		case 2:
			chaos_knob.label = "NOISE";
			chaos_knob.setVisible(true);
			chaos2_knob.setVisible(false);
			break;
		//case 5:
			//chaos_knob.label = "?";
			//chaos_knob.setVisible(false);
			//chaos2_knob.setVisible(true);
			//break;
		case 3:
			chaos_knob.label = "CRUNCH";
			chaos_knob.setVisible(false);
			chaos2_knob.setVisible(true);
			break;
		case 4:
			chaos_knob.label = "BIAS";
			chaos_knob.setVisible(false);
			chaos2_knob.setVisible(true);
			break;
		}
		//algorithm_combobox.updateParameterInfo();
		repaint();
	}

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FlowerChildFilterEditor)
};
