#pragma once

#include "JuceHeader.h"
#include "se_framework/ElanSynthLib/ElanSynthLib.h"

class RaPG_ClockModule;

class RaPG_PresetWidget : public Component
{
public:
	RaPG_PresetWidget()
	{
		font = { Font("Arial", 10, 1) };
	}

	void mouseEnter(const MouseEvent & event) override
	{
		leftButtonOpacity = opacityLow;
		rightButtonOpacity = opacityLow;
		saveOpacity = opacityLow;
		loadOpacity = opacityLow;

		if (event.eventComponent == leftButton)
			leftButtonOpacity = opacityHigh;

		else if (event.eventComponent == rightButton)
			rightButtonOpacity = opacityHigh;

		else if (event.eventComponent == save)
			saveOpacity = opacityHigh;

		else if (event.eventComponent == load)
			loadOpacity = opacityHigh;

		repaint();
	}

	void mouseExit(const MouseEvent & event) override
	{
		leftButtonOpacity = opacityLow;
		rightButtonOpacity = opacityLow;
		saveOpacity = opacityLow;
		loadOpacity = opacityLow;

		repaint();
	}

	void setImage(Image * v)
	{
		image = v;
		bounds = { 0.f + offsetXY, 0.f + offsetXY, (float)image->getWidth(), (float)image->getHeight() };
	}

	void paint(Graphics & g) override
	{
		// BACKGROUND
		g.setColour({ 0, 0, 0 });
		g.fillRect(bounds);

		// IMAGE
		g.drawImage(*image, bounds, juce::RectanglePlacement::Flags::doNotResize);

		// TEXT
		g.setColour({ 255, 255, 255 });
		g.setFont(font);
		font.setHeight(10.f);
		g.drawText(text, juce::Rectangle<float>{36.f, 0.f, 151.f, 19.f}, juce::Justification::centred);

		// VISUALIZE HITBOXES        
		g.setColour(Colours::white);

		g.setOpacity(leftButtonOpacity);
		g.fillRect(leftButton->getBounds());

		g.setOpacity(rightButtonOpacity);
		g.fillRect(rightButton->getBounds());

		g.setOpacity(saveOpacity);
		g.fillRect(save->getBounds());

		g.setOpacity(loadOpacity);
		g.fillRect(load->getBounds());
	}

	void setText(String v)
	{
		text = v;
		repaint();
	}

	Image * image;
	juce::Rectangle<float> bounds;
	float offsetXY = 0;
	Font font;

	Component * leftButton;
	Component * rightButton;
	Component * save;
	Component * load;
	String text = "PRESET";

	float opacityLow = 0.1f;
	float opacityHigh = 0.3f;

	float leftButtonOpacity = opacityLow;
	float rightButtonOpacity = opacityLow;
	float saveOpacity = opacityLow;
	float loadOpacity = opacityLow;
};

namespace elan
{
	class StaticSwitchImageWithText : public RWidget
	{
	public:
		StaticSwitchImageWithText() {}
		~StaticSwitchImageWithText() = default;

		void paint(Graphics & g) override
		{
			if (isOn)
				imageOn.paint(g);
			else
				imageOff.paint(g);

			g.setFont(font);
			g.setColour(fontColor);
			g.drawText(text, juce::Rectangle<float>{ fontOffsetX, fontOffsetY, (float)getWidth(), (float)getHeight() }, juce::Justification::centred);
		}

		int getFileWidth() { return imageOff.getImage().getWidth(); }
		int getFileHeight() { return imageOff.getImage().getHeight(); }

		void setCenteredBounds(float centerX, float centerY, int width, int height)
		{
			juce::Rectangle<float> rect = juce::Rectangle<float>((float)getFileWidth(), (float)getFileHeight());
			rect.setCentre({ centerX, centerY });

			setSize(getFileWidth(), getFileHeight());
			setTopLeftPosition((int)rect.getX(), (int)rect.getY());
		}

		void setIsOn(bool v)
		{
			isOn = v;
			repaint();
		}

		DrawableImage imageOff;
		DrawableImage imageOn;
		bool isOn = false;
		Font font;
		Colour fontColor;
		float fontOffsetY = 0;
		float fontOffsetX = 0;
		String text;
	};

	class ImageButton : public ElanModulatableButton
	{
	public:
		ImageButton(
			const void * offImageData, size_t offImageSize,
			const void * onImageData, size_t onImageSize)
		{
			offImage.setImage(juce::ImageFileFormat::loadFrom(offImageData, offImageSize));
			onImage.setImage(juce::ImageFileFormat::loadFrom(onImageData, onImageSize));
		}
		~ImageButton() = default;

		int getFileWidth() { return offImage.getImage().getWidth(); }
		int getFileHeight() { return offImage.getImage().getHeight(); }

		void setCenteredBounds(float centerX, float centerY, int width, int height)
		{
			juce::Rectangle<float> rect = juce::Rectangle<float>((float)getFileWidth(), (float)getFileHeight());
			rect.setCentre({ centerX, centerY });

			setSize(getFileWidth(), getFileHeight());
			setTopLeftPosition((int)rect.getX(), (int)rect.getY());
		}

		void paint(Graphics & g) override
		{
			bool isOff = !getToggleState();

			if (isOff)
				offImage.paint(g);
			else
				onImage.paint(g);

			{// add red overlay to indicate modulation
				jura::ModulatableParameter* mp = dynamic_cast<jura::ModulatableParameter*> (assignedParameter);
				if (mp && mp->hasConnectedSources())
				{
					g.setColour(Colour::fromFloatRGBA(1.f, 0.f, 0.f, 0.5f));
					g.drawRect(clickableArea.toFloat(), 2.5f);
				}
			}
		}

		bool getIsOff()
		{
			return !getToggleState();
		}

		void setClickableArea(float centerX, float centerY, int width, int height)
		{
			juce::Rectangle<float> rect = juce::Rectangle<float>((float)width, (float)height);
			rect.setCentre({ centerX, centerY });

			clickableArea.setBounds(
				(int)rect.getX(),
				(int)rect.getY(),
				(int)rect.getWidth(),
				(int)rect.getHeight());
		}

		String getValueDisplayString() const override
		{
			return stringFunc();
		}
		std::function<String()> stringFunc = []() { return String(); };


	protected:
		DrawableImage offImage;
		DrawableImage onImage;

		juce::Rectangle<int> clickableArea;

		bool hitTest(int x, int y) override
		{
			return clickableArea.contains(juce::Point<int>(x, y));
		}
	};
} //namespace elan
class StaticClickButton : public RButton
{
public:
	StaticClickButton(String svg)
		: button(svg)
	{
	}
	~StaticClickButton() = default;

	void setRotation(float v) { rotation = v; }
	void setIncrementAmount(double v) { incrementAmount = v; }

	void paint(Graphics & g) override
	{
		button.drawWithRotation(g, button.getBounds().toFloat(), rotation, 1.f);
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

protected:
	double incrementAmount = 1;
	SVGObject button;
	float rotation = 0;
};

class ModulatableNumberDisplay : public rsModulatableSlider
{
public:
	ModulatableNumberDisplay(Font font, Colour fontColor)
		: font(font)
		,	fontColor(fontColor)
	{}
	~ModulatableNumberDisplay() = default;

	void paint(Graphics & g) override
	{
		g.setFont(font);
		g.setColour(fontColor);
		g.drawText(getValueDisplayString(), juce::Rectangle<float>{ 0.f, offsetY, (float)getWidth(), (float)getHeight() }, juce::Justification::centred);

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
		double scale = .005;
		if (ModifierKeys::getCurrentModifiers().isShiftDown()) // fine tuning via shift
			scale *= 0.0625;

		if (isEnabled())
		{
			if (!e.mods.isRightButtonDown() && !e.mods.isCommandDown())
			{
				int newDragDistance = e.getDistanceFromDragStartY();
				int dragDelta = newDragDistance - oldDragDistance;
				oldDragDistance = newDragDistance;
				dragValue += scale * dragDelta;
				double y = normalizedValueOnMouseDown;
				y -= dragValue;
				y = clip(y, 0.0, 1.0);

				setNormalizedValue(y);
			}
		}
	}

	String getValueDisplayString() const override
	{
		return stringFunc();
	}
	std::function<String()> stringFunc = []() { return ""; };

	Colour fontColor;
	Font font;
	float offsetY = 0;
};

class SimpleNumberDisplay : public RSlider
{
public:
	SimpleNumberDisplay(Font font, Colour fontColor)
		: font(font),
		fontColor(fontColor)
	{}
	~SimpleNumberDisplay() = default;

	void paint(Graphics & g) override
	{
		g.setFont(font);
		g.setColour(fontColor);
		g.drawText(getValueDisplayString(), juce::Rectangle<float>{ 0.f, offsetY, (float)getWidth(), (float)getHeight() }, juce::Justification::centred);

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
		double scale = .005;
		if (ModifierKeys::getCurrentModifiers().isShiftDown()) // fine tuning via shift
			scale *= 0.0625;

		if (isEnabled())
		{
			if (!e.mods.isRightButtonDown() && !e.mods.isCommandDown())
			{
				int newDragDistance = e.getDistanceFromDragStartY();
				int dragDelta = newDragDistance - oldDragDistance;
				oldDragDistance = newDragDistance;
				dragValue += scale * dragDelta;
				double y = normalizedValueOnMouseDown;
				y -= dragValue;
				y = clip(y, 0.0, 1.0);

				setNormalizedValue(y);
			}
		}
	}

	String getValueDisplayString() const override
	{
		return stringFunc();
	}
	std::function<String()> stringFunc = []() { return String(); };

	Colour fontColor;
	Font font;
	float offsetY = 0;
};

class NumberCircle : public ElanModulatableSlider
{
public:
	NumberCircle(const Font & font, const Colour & fontColor, const Colour & circleColor)
		: font(font)
		, fontColor(fontColor)
		, circleColor(circleColor)
	{}
	~NumberCircle() = default;

	void paint(Graphics & g) override
	{
		// CIRCLE
		g.setColour(circleColor);
		g.fillEllipse(0.f, 0.f, (float)getWidth(), (float)getHeight());

		// NUMBR
		g.setColour(fontColor);
		g.setFont(font);
		g.drawText(String(getValue(), 0), juce::Rectangle<float>{0.f, offsetY, (float)getWidth(), (float)getHeight()}, juce::Justification::centred, false);

		{//ARROWS
			Path p;
			p.addPolygon({ 13.5f, 3.5f }, 3, 2.f);
			p.addPolygon({ 13.5f, 23.5f }, 3, 2.f, (float)PI);
			g.setColour({ 62, 61, 60 });
			g.fillPath(p);
		}

		{// add red overlay to indicate modulation
			jura::ModulatableParameter * mp = dynamic_cast<jura::ModulatableParameter*> (assignedParameter);
			if (mp && mp->hasConnectedSources())
			{
				g.setColour(Colour::fromFloatRGBA(1.f, 0.f, 0.f, 0.5f));
				g.drawRect(getLocalBounds().toFloat(), 2.5f);
			}
		}
	}

	void mouseDrag(const MouseEvent & e) override
	{
		double scale = .005;
		if (ModifierKeys::getCurrentModifiers().isShiftDown()) // fine tuning via shift
			scale *= 0.0625;

		if (isEnabled())
		{
			if (!e.mods.isRightButtonDown() && !e.mods.isCommandDown())
			{
				int newDragDistance = e.getDistanceFromDragStartY();
				int dragDelta = newDragDistance - oldDragDistance;
				oldDragDistance = newDragDistance;
				dragValue += scale * dragDelta;
				double y = normalizedValueOnMouseDown;
				y -= dragValue;
				y = clip(y, 0.0, 1.0);

				setNormalizedValue(y);
			}
		}
	}

	Colour circleColor;
	Colour fontColor;
	Font font;
	float offsetY = 0;
};

class RaPG_EnvelopeVisualizer : public jura::ModulatorCurveEditor
{
public:
	RaPG_EnvelopeVisualizer(const juce::String & name = "ModulatorCurveEditor")
		: rsPlotEditor(name)
		, ModulatorCurveEditor(name)
	{

	}

	void parameterChanged(Parameter* parameterThatHasChanged) override
	{
		updateMaximumRange(true);
		updatePlotCurveData();
	}

	void paint(Graphics & g) override
	{
		if (plotImage != NULL)
			g.drawImage(*plotImage, 0, 0, plotImage->getWidth(), plotImage->getHeight(),
				0, 0, plotImage->getWidth(), plotImage->getHeight(), false);
	}

	void plotLoopLocators(Graphics &g, juce::Image *targetImage, rosic::BreakpointModulator* modulator, const Colour& locatorColour, bool fullHeight = true) override {}


	~RaPG_EnvelopeVisualizer() = default;
protected:
	void mouseDown(const MouseEvent &e) override {}
	void mouseDrag(const MouseEvent &e) override {}
	void mouseMove(const MouseEvent &e) override {}
	void mouseUp(const MouseEvent &e) override {}
};

class PulseVisualizer : public jura::RWidget
{
public:
	PulseVisualizer(RaPG_ClockModule * clockToEdit) : clockToEdit(clockToEdit) {}
	~PulseVisualizer() = default;

	void registerSlider(RSlider * slider)
	{
		registeredSliders.push_back(slider);
	}

	RaPG_ClockModule * clockToEdit;

protected:

	void paint(Graphics & g) override
	{
		juce::Rectangle<float> componentArea{ 0.0,0.0,(float)getWidth(), (float)getHeight() };
		float square = (float)std::min<int>(getWidth(), getHeight());

		juce::Rectangle<float> ellipseArea{ 0,0.f,square,square };
		ellipseArea.setCentre(componentArea.getCentre());

		{
			Path p;
			p.addEllipse(ellipseArea);
			g.setColour({ 113, 119, 149 });
			g.fillPath(p);
		}

		juce::Rectangle<float> pulseArea = ellipseArea.reduced(8, 8).translated(0, -1);

		{
			g.setColour({ 255, 255, 255 });
			for (const auto & slider : registeredSliders)
			{
				float value = (float)slider->getNormalizedValue();
				float drawAreaHorizontal = pulseArea.getWidth();
				float lineLocationX = pulseArea.getTopLeft().getX() + drawAreaHorizontal * value;
				float topY = pulseArea.getTopLeft().getY();
				float lineLocationBottomY = pulseArea.getBottom();
				float farthestX = pulseArea.getRight();

				g.drawLine(floor(lineLocationX), floor(topY), floor(lineLocationX), floor(lineLocationBottomY), 2);
				g.drawLine(pulseArea.getBottomLeft().getX() - 1, pulseArea.getBottomLeft().getY(), farthestX + 1, lineLocationBottomY, 2);
			}
		}
	}

	void parameterChanged(Parameter* parameterThatHasChanged) override
	{
		repaint();
	}

	vector<RSlider *> registeredSliders;
};

class RaPG_WaveformVisualizer : public jura::RWidget
{
public:
	Phasor phasor;

	RaPG_WaveformVisualizer()
	{
		waveformVisualizer.setSampleRate(44100);
		phasor.setSampleRate(44100);
		phasor.setFrequency(100);
	}

	jura::rsWaveformPlot waveformVisualizer;

protected:
	void paint(Graphics & g) override
	{
		waveformVisualizer.paint(g);
	}

	void fillBuffer()
	{
		points[0].clear();
		updateCoefficients();
		for (int i = 0; i < 441; ++i)
		{
			double phase = phasor.getSample();
			double x;
			if (phase < h)
				x = RAPT::rsTriSawOscillator<double>::shape(a0 + a1 * phase, atkbend, -0.5 * atksig); // upward section
			else
				x = RAPT::rsTriSawOscillator<double>::shape(b0 + b1 * phase, -decbend, -0.5 * decsig);  // downward section
			points[0].push_back(x);
		}

		waveformVisualizer.setWaveform((double**)points.data(), (int)points[0].size(), (int)points.size());
	}

	void parameterChanged(Parameter* parameterThatHasChanged) override
	{
		//fillBuffer();
		//repaint();
	}

	void updateCoefficients()
	{
		h = 0.5 * (asym + 1);
		a0 = -1;
		a1 = 2 / h;
		b0 = (1 + h) / (1 - h);
		b1 = -1 - b0;
	}

	vector<vector<double>> points{ 1 };

	double h, a0, a1, b0, b1;

	double phaseOffset = 0;
	double asym = 0;
	double atkbend = 0;
	double atksig = 0;
	double decbend = 0;
	double decsig = 0;
};
