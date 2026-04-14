#pragma once

#undef RELATIVE
#undef ABSOLUTE

using std::vector;

//=================================================================================================

class JUCE_API rsParameterMapperSinhUni : public jura::rsParameterMapper
{
public:

	rsParameterMapperSinhUni(double minValue, double maxValue, double shape)
	{
		b = shape;
		setRange(minValue, maxValue); // updates the a-coeff
	}
	rsParameterMapperSinhUni() {}

	void setRangeAndShape(double minValue, double maxValue, double shape)
	{
		b = shape;
		setRange(minValue, maxValue); // updates the a-coeff
	}

	double map(double x) const override
	{
		return a * sinh(b*x);  // 0..max
	}

	double unmap(double y) const override
	{
		return asinh(y/a) / b; // 0..1
	}

	/** The range must be symmetrical around 0: newMin == -newMax. */
	void setRange(double newMin, double newMax) override
	{
		jassert(newMin == 0); // supports currently only 0-centered mapping
		rsParameterMapper::setRange(newMin, newMax);
		updateCoeffs();
	}

	void setShape(double newShape)
	{
		jassert(newShape > 0);
		b = newShape;
		updateCoeffs();
	}

protected:

	void updateCoeffs()
	{
		a = max / sinh(b);
	}

	double a = 1, b = 1;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(rsParameterMapperSinhUni)
};

//=================================================================================================

class JUCE_API rsParameterMapperTanhUni : public jura::rsParameterMapper
{
public:

	rsParameterMapperTanhUni(double minValue, double maxValue, double shape)
	{
		b = shape;
		setRange(minValue, maxValue); // updates the a-coeff
	}
	rsParameterMapperTanhUni() {}

	void setRangeAndShape(double minValue, double maxValue, double shape)
	{
		b = shape;
		setRange(minValue, maxValue); // updates the a-coeff
	}

	double map(double x) const override
	{
		return a * tanh(b*x);  // 0..max
	}

	double unmap(double y) const override
	{
		return atanh(y/a) / b; // 0..1
	}

	/** The range must be symmetrical around 0: newMin == -newMax. */
	void setRange(double newMin, double newMax) override
	{
		jassert(newMin == 0); // supports currently only 0-centered mapping
		rsParameterMapper::setRange(newMin, newMax);
		updateCoeffs();
	}

	void setShape(double newShape)
	{
		jassert(newShape > 0);
		b = newShape;
		updateCoeffs();
	}

protected:

	void updateCoeffs()
	{
		a = max / tanh(b);
	}

	double a = 1, b = 1;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(rsParameterMapperTanhUni)
};

//=================================================================================================

class ParameterMapperLinearBipolarSkipZero : public jura::rsParameterMapper
{
public:

	ParameterMapperLinearBipolarSkipZero(double minValue, double maxValue)
	{
		rsParameterMapper::setRange(minValue, maxValue);
		updateCoeffs();
	}

	double skip_zero(double v) const
	{
		if (v < low_zero || v > high_zero)
			return v;

		return v -= normalized_increment;
	}

	double map(double x) const override
	{
		return jmap(skip_zero(x), min, max);
	}

	double unmap(double y) const override
	{
		return skip_zero(jmap(y, min, max, 0.0, 1.0));
	}

protected:

	double low_zero = 0.25;
	double high_zero = 0.75;
	double normalized_increment = 0.5;

	void updateCoeffs()
	{
		normalized_increment = 1/std::abs(max-min);
		double normalized_increment_half = normalized_increment * 0.5;
		low_zero = 0.5 - normalized_increment_half;
		high_zero = 0.5 + normalized_increment_half;
	}
};

//=================================================================================================

class ParamManager;

class RText : public jura::RWidget
{
	static const Font& getAldrichFont()
	{
		static Font aldrich;
		return aldrich;
	}

public:
	RText()
	{
		m_font = getAldrichFont();
	}
	RText(String text) : m_text(text)
	{
		m_font = getAldrichFont();
	}


	virtual void paint(Graphics & g) override
	{
		if (m_drawMultilineText)
		{
			g.setFont(m_font);
			g.setColour(m_fontColor);
			g.drawMultiLineText(m_text, 0, (int)m_size, getWidth());
		}
		else
		{
			m_font.setHeight((float)getHeight());
			g.setFont(m_font);
			g.setColour(m_fontColor);
			g.drawText(m_text, 0, 0, getWidth(), getHeight(), m_justificationStyle);
		}
	}

	void setFontStyle(String style, float size, Font::FontStyleFlags styleFlags = Font::plain) 
	{ 
		m_size = size;
		m_font = Font(m_style = style, size, styleFlags);
		m_font.setHeight((float)size);
	}

	void setFontSize(float v) { m_font.setHeight(m_size = v); }
	void setFontColor(Colour c) { m_fontColor = c; }
	void setText(String s) { m_text = s; repaint(); }
	void setFontJustification(juce::Justification v)
	{
		m_justificationStyle = v;
	}
	void setDrawMultilineText(bool v) { m_drawMultilineText = v; }

protected:
	Font m_font;
	Colour m_fontColor = Colours::black;
	String m_text;
	String m_style;
	juce::Justification m_justificationStyle = juce::Justification::centred;
	float m_size = 13;

	bool m_drawMultilineText = false;
};

class RVector : public jura::RWidget
{
public:
	RVector(String XML)
	{
		////////ScopedPointer<XmlElement> svg_(XmlDocument::parse(XML));
		////////drawable->createFromSVG(*svg_);
		setInterceptsMouseClicks(false, false);
	};

	virtual void paint(Graphics & g) override
	{
		drawable->drawWithin(g, juce::Rectangle<float>(0, 0, (float)round(getWidth()/2.0)*2, (float)round(getHeight()/2.0)*2),
			juce::RectanglePlacement::Flags::centred,
			1.0);
	}
	
	ScopedPointer<Drawable> drawable;

protected:

	ScopedPointer<XmlElement> svg;
	
};

class ModulatableKnob : public rsModulatableSlider
{
public:
	ModulatableKnob() { 
		font = &jura::BitmapFontRoundedBoldA10D0::instance; 
		nameText.setText(sliderName);
	};
	~ModulatableKnob() = default;
	
	const jura::BitmapFont * font;

	jura::RTextField nameText;
	jura::RTextField valueText;

	float currentValue = 0;

	void mouseDown(const MouseEvent& e) override
	{
		if (e.mods.isRightButtonDown())
		{
			openRightClickPopupMenu();
			return;
		}

		// ignore click on the label, when it's not 'inside' the actual slider
		if (e.originalComponent != this)
			return; 

		if (isEnabled())
		{
			if (e.mods.isCommandDown())
			{
				RSlider::setValue(defaultValue);
			}
			else if (e.mods.isLeftButtonDown())
			{
				RSlider::normalizedValueOnMouseDown = RSlider::getNormalizedValue();

				oldDragDistance  = 0;
				dragValue        = 0.0;
			}
		}
	}

	void mouseDrag(const MouseEvent& e) override
	{
		if (!isMouseDownEventInsideKnob(e))
			return;

		double scale = 0.005;
		if (ModifierKeys::getCurrentModifiers().isShiftDown()) // fine tuning via shift
			scale *= 0.0625;

		if (isEnabled())
		{
			if (!e.mods.isRightButtonDown() && !e.mods.isCommandDown())
			{
				int newDragDistance = e.getDistanceFromDragStartY();
				int dragDelta       = newDragDistance - oldDragDistance;
				oldDragDistance     = newDragDistance;  // update - from now on we only need dragDelta
				dragValue += scale*dragDelta;

				double y = RSlider::normalizedValueOnMouseDown;  // in 0..1

				y -= dragValue;                                          // new x
				y = clamp(y, 0.0, 1.0);
				y = proportionOfLengthToValue(y);                        // convert to value
				setValue(constrainAndQuantizeValue(y), true, false);     // set it
			}
		}
	}

	void mouseDoubleClick(const MouseEvent& e) override
	{
		if (isEnabled() && e.mods.isLeftButtonDown())
			setValue(openModalNumberEntryField(getValue()), true, false);
	}

	void mouseWheelMove(const MouseEvent & e, const MouseWheelDetails & wheel) override
	{
		if (!isMouseDownEventInsideKnob(e))
			return;

		double tmpValue;
		if (isEnabled())
		{
			float s;
			if (wheel.deltaY >= 0.0)
				s = 1.0;
			else
				s = -1.0;

			float scale = 0.5;
			if (ModifierKeys::getCurrentModifiers().isShiftDown())
				scale = 0.0125f;

			if (interval > 0.0)
			{
				tmpValue = getValue() + s * interval;
				setValue(constrainAndQuantizeValue(tmpValue), true, false);
			}
			else
				setNormalizedValue(getNormalizedValue() + scale * 0.01 * wheel.deltaY, true, false);
		}
	}

	void drawKnob(Graphics & g, juce::Rectangle<float> area, float strokeThickness, bool bipolar)
	{
		constexpr float minRotationRadians = juce::degreesToRadians(-135.f);
		constexpr float maxRotationRadians = juce::degreesToRadians(+135.f);

		float x = area.getX();
		float y = area.getY();
		float width = area.getWidth();
		float height = area.getHeight();
		float centerX = x + width / 2;
		float centerY = y + height / 2;

		float minValue_ = (float)getMinimum();
		float maxValue_ = (float)getMaximum();
		float currentValueToAngle = juce::jmap(currentValue, minValue_, maxValue_, minRotationRadians, maxRotationRadians);

		float modulatedValue = (float)getModulatableParameter()->getModulatedValue();
		float modulatedValueToAngle = juce::jmap(modulatedValue, minValue_, maxValue_, minRotationRadians, maxRotationRadians);

		{// backgroundImage circle fill			
			Path p;
			p.addEllipse(x+1, y+1, width-2, height-2);
			g.setColour(Colour(235, 248, 247));
			g.fillPath(p);
		}

		{// inner pie indicator
			Path p;
			if (bipolar)
			{
				p.addPieSegment({ x, y, width, height }, 0, currentValueToAngle, 0);
			}
			else
			{
				p.addPieSegment({ x, y, width, height }, minRotationRadians, currentValueToAngle, 0);
			}
			g.setColour(Colour(155, 221, 218));
			g.fillPath(p);
		}

		{// line indicator
			Path p;
			p.addLineSegment(juce::Line<float>(centerX, centerY, centerX, y + strokeThickness + strokeThickness/2), strokeThickness);
			g.setColour(Colour(0, 0, 0));
			g.fillPath(p, AffineTransform::rotation(currentValueToAngle, centerX, centerY));
		}

		{// line indicator for modulation
			Path p;
			p.addLineSegment(juce::Line<float>(centerX, centerY, centerX, y + strokeThickness + strokeThickness/2), strokeThickness);
			g.setColour(Colour(255, 0, 0));
			g.fillPath(p, AffineTransform::rotation(modulatedValueToAngle, centerX, centerY));
		}

		{// center dot
			Path p;
			g.setColour({ 0, 0, 0 });
			p.addEllipse(centerX - strokeThickness/2, centerY - strokeThickness/2, strokeThickness, strokeThickness);
			g.fillPath(p);
		}

		{// inner circle stroke		
			Path p;
			p.addEllipse({
				x + strokeThickness + strokeThickness/2 - 1,
				y + strokeThickness + strokeThickness/2 - 1,
				width + 2 - strokeThickness*3,
				height + 2 - strokeThickness*3
			});
			g.setColour(Colour(118, 125, 255));
			g.strokePath(p, juce::PathStrokeType(strokeThickness+1, juce::PathStrokeType::JointStyle::curved, juce::PathStrokeType::EndCapStyle::square));
		}

		{// outer arc outline
			Path p;
			p.addArc( x + strokeThickness/2, y + strokeThickness/2, width-strokeThickness, height-strokeThickness, minRotationRadians, maxRotationRadians, true);
			g.setColour(Colour(92, 200, 194));
			g.strokePath(p, juce::PathStrokeType(strokeThickness, juce::PathStrokeType::JointStyle::curved, juce::PathStrokeType::EndCapStyle::square));
		}
	}

	void paint(Graphics & g) override
	{
		currentValue = (float)getValue();

		float width = (float)getWidth();
		float height = (float)getHeight();

		{// add red overlay to indicate modulation
			jura::ModulatableParameter* mp = dynamic_cast<jura::ModulatableParameter*> (assignedParameter);

			if (mp && mp->hasConnectedSources())
				g.fillAll(Colour::fromFloatRGBA(1.f, 0.f, 0.f, 0.125f)); // preliminary
		}

		knobArea.setBounds(12, 12, width-24, height-24);
		drawKnob(g, knobArea, 2.5, true);

		{// name text
			Colour colour = getTextColour();
			int paddingForCentering = int(width - font->getTextPixelWidth(sliderName, 1)) / 2;
			drawBitmapFontText(g, paddingForCentering, 0, sliderName, font, colour);
		}

		{// value text
			Colour colour = getTextColour();
			String valueText_ = stringConversionFunction(getValue());
			int paddingForCentering = int(width - font->getTextPixelWidth(valueText_, 1)) / 2;
			drawBitmapFontText(g, paddingForCentering, int(height - 10), valueText_, font, colour);
		}
	}

	bool isMouseDownEventInsideKnob(const MouseEvent & e)
	{
		int x = e.getMouseDownPosition().getX();
		int y = e.getMouseDownPosition().getY();
		return !(x < knobArea.getX() || x > knobArea.getX() + knobArea.getWidth() ||
			       y < knobArea.getY() || y > knobArea.getY() + knobArea.getHeight());
	}

	bool isMousePositionInsideKnob(const MouseEvent & e)
	{
		int x = e.getPosition().getX();
		int y = e.getPosition().getY();
		return !(x < knobArea.getX() || x > knobArea.getX() + knobArea.getWidth() ||
			       y < knobArea.getY() || y > knobArea.getY() + knobArea.getHeight());
	}

protected:
	juce::Rectangle<float> knobArea;
	juce::Component knobComponent;
};

class myparams
{
	friend class ParamManager;

public:
	myparams(ParamManager * ptr) : managerPtr(ptr) {}
	virtual ~myparams()
	{
		//if (mapper != nullptr)
		//	delete mapper;
	}
	enum Type { BUTTON, CLICKBUTTON, SLIDER, KNOB, COMBOBOX, };

	void init(int type, String strID, double mini, double maxi, double def, int scaling = -1);
	void initButton(String strID, int def);
	void initClickButton(String strID);
	void initSlider(String strID, double mini, double maxi, double def);
	void initKnob(String strID, double mini, double maxi, double def);
	void initCombobox(String strID, int def, vector<String> list = vector<String>{});

	/* parameter profile */
	virtual void initializeParameter() {};
	jura::Parameter::scalings scaling = jura::Parameter::scalings::LINEAR;

	jura::ModulationConnection::modModes modMode = jura::ModulationConnection::modModes::ABSOLUTE;
	double modLimitMin = -1.e+100;
	double modLimitMax = +1.e+100;
	double modDepthMin = 0;
	double modDepthMax = 0;
	double defaultModDepth = 0;

	double shouldBeSmoothed = true;
	double interval = 0;
	juce::String(*stringConvertFunc) (double v) = elan::StringFunc3;

	/* jura::Parameter */
	String strID;
	String text;
	int type = 0;
	double mini = 0, maxi = 0, def = 0;
	jura::ModulatableParameter2 * ptr = nullptr;
	void addMenuItems(const vector<String> & list);
	void instantiateParameter();
	bool saveAndRecall = true;

	/* Mappers */
	jura::rsParameterMapper * mapper = nullptr;

	/* Parameter Manager */
	ParamManager * managerPtr = nullptr;

	/* Widget */
	jura::RWidget * widget = nullptr;
	void setBounds(int x, int y, int w, int h);
	vector<String> menuItems;
	String helpText;

	/* Conveniences */
	std::function<void(double)> callback;
	void setCallback(std::function<void(double)> cb);

	operator double() const { return ptr->getValue(); }
	bool operator==(const double & rhs) const { return ptr->getValue() == rhs; }
	bool operator!=(const double & rhs) const { return ptr->getValue() != rhs; }
	bool operator<(const double & rhs) const { return ptr->getValue() < rhs; }
	bool operator>(const double & rhs) const { return ptr->getValue() > rhs; }
	void setValue(double v) { ptr->setValue(v, true, true); }

protected:
	// set by the parameter manager class
	int id = -1;
	// set by manager to true when added to smoother manager, false when removed
	bool isAddedToSmootherManager = false; 
	// for smoother
	double prev_sliderVal = 0; 
};

namespace ParameterProfile {

class Linear : public myparams
{
public:
	Linear(ParamManager * ptr) : myparams(ptr) {}
	virtual ~Linear() = default;

	void initializeParameter() override
	{
		defaultModDepth = .1;
		if (mini < 0)
			scaling = jura::Parameter::scalings::LINEAR_BIPOLAR;
	}
};

class LinearQuantized : public myparams
{
public:
	LinearQuantized(ParamManager * ptr) : myparams(ptr) {}
	virtual ~LinearQuantized() = default;

	void initializeParameter() override
	{
		defaultModDepth = jmin(1, (int)std::abs(maxi-mini)/5);
		interval = 1;
		if (mini < 0)
			scaling = jura::Parameter::scalings::LINEAR_BIPOLAR;
		stringConvertFunc = elan::StringFunc0;
	}
};

class ExponentialNoSmoothing : public myparams
{
public:
	ExponentialNoSmoothing(ParamManager * ptr) : myparams(ptr) {}
	virtual ~ExponentialNoSmoothing() = default;

	void initializeParameter() override
	{
		jassert(mini > 0);
		scaling = jura::Parameter::scalings::EXPONENTIAL;
		modMode = jura::ModulationConnection::modModes::ABSOLUTE;
		modLimitMin = mini;
		modLimitMax = maxi;
		defaultModDepth = 1;
		shouldBeSmoothed = false;
	}
};

class ParameterSmoothingAmount : public myparams
{
public:
	ParameterSmoothingAmount(ParamManager * ptr) : myparams(ptr) {}
	virtual ~ParameterSmoothingAmount() = default;

	void initializeParameter() override
	{
		jassert(mini > 0);
		scaling = jura::Parameter::scalings::EXPONENTIAL;
		modMode = jura::ModulationConnection::modModes::ABSOLUTE;
		modLimitMin = mini;
		modLimitMax = maxi;
		defaultModDepth = 1;
		shouldBeSmoothed = false;
		saveAndRecall = false;
	}
};

class Exponential : public myparams
{
public:
	Exponential(ParamManager * ptr) : myparams(ptr) {}
	virtual ~Exponential() = default;

	void initializeParameter() override
	{
		jassert(mini > 0);
		scaling = jura::Parameter::scalings::EXPONENTIAL;
		modMode = jura::ModulationConnection::modModes::ABSOLUTE;
		modLimitMin = mini;
		modLimitMax = maxi;
		defaultModDepth = 1;
	}
};

class Gain : public myparams
{
public:
	Gain(ParamManager * ptr) : myparams(ptr) {}
	virtual ~Gain() = default;

	void initializeParameter() override
	{
		modDepthMin = -maxi;
		modDepthMax = +maxi;
		defaultModDepth = 1;
		stringConvertFunc = elan::decibelsToStringWithUnit2;
	}
};

class Phase : public myparams
{
public:
	Phase(ParamManager * ptr) : myparams(ptr) {}
	virtual ~Phase() = default;

	void initializeParameter() override
	{
		modDepthMin = -10;
		modDepthMax = +10;
		defaultModDepth = +1;
	}
};

class FeedbackAmount : public myparams
{
public:
	FeedbackAmount(ParamManager * ptr) : myparams(ptr) {}
	virtual ~FeedbackAmount() = default;

	void initializeParameter() override
	{
		scaling = jura::Parameter::scalings::LINEAR_BIPOLAR;
		modLimitMin = -1.e+10;
		modLimitMax = +1.e+10;
		modDepthMin = -8;
		modDepthMax = +8;
	}
};

class LowpassFilterFrequency : public myparams
{
public:
	LowpassFilterFrequency(ParamManager * ptr) : myparams(ptr) {}
	virtual ~LowpassFilterFrequency() = default;

	void initializeParameter() override
	{
		jassert(mini > 0);
		scaling = jura::Parameter::scalings::EXPONENTIAL;
		modMode = jura::ModulationConnection::modModes::EXPONENTIAL;
		modLimitMin = 0;
		modLimitMax = +1.e+100;
		modDepthMin = -20;
		modDepthMax = 20;
		defaultModDepth = 14;
		stringConvertFunc = elan::hertzToStringLowpassFilter;
	}
};

class HighpassFilterFrequency : public myparams
{
public:
	HighpassFilterFrequency(ParamManager * ptr) : myparams(ptr) {}
	virtual ~HighpassFilterFrequency() = default;

	void initializeParameter() override
	{
		jassert(mini > 0);
		scaling = jura::Parameter::scalings::EXPONENTIAL;
		modMode = jura::ModulationConnection::modModes::EXPONENTIAL;
		modDepthMin = -20;
		modDepthMax = 20;
		modLimitMin = 0;
		modLimitMax = +1.e+100;
		defaultModDepth = 14;
		stringConvertFunc = elan::hertzToStringHighpassFilter;
	}
};

class UnipolarAmplitude : public myparams
{
public:
	UnipolarAmplitude(ParamManager * ptr) : myparams(ptr) {}
	virtual ~UnipolarAmplitude() = default;

	void initializeParameter() override
	{
		modLimitMin = -maxi*10;
		modLimitMax = +maxi*10;
		modDepthMin = -maxi;
		modDepthMax = +maxi;
		defaultModDepth = 1;
	}
};

class BipolarAmplitude : public myparams
{
public:
	BipolarAmplitude(ParamManager * ptr) : myparams(ptr) {}
	virtual ~BipolarAmplitude() = default;

	void initializeParameter() override
	{
		defaultModDepth = 1;
		if (maxi > 1)
			mapper = new jura::rsParameterMapperSinh(mini, maxi, 4);
	}
};

class UnipolarPhase : public myparams
{
public:
	UnipolarPhase(ParamManager * ptr) : myparams(ptr) {}
	virtual ~UnipolarPhase() = default;

	void initializeParameter() override
	{
		modDepthMin = -10;
		modDepthMax = +10;
		defaultModDepth = 1;
	}
};

class BipolarPhase : public myparams
{
public:
	BipolarPhase(ParamManager * ptr) : myparams(ptr) {}
	virtual ~BipolarPhase() = default;

	void initializeParameter() override
	{
		modDepthMin = -10;
		modDepthMax = +10;
		defaultModDepth = 1;
	}
};

class ClipLevel : public myparams
{
public:
	ClipLevel(ParamManager * ptr) : myparams(ptr) {}
	virtual ~ClipLevel() = default;

	void initializeParameter() override
	{
		jassert(mini > 0);
		scaling = jura::Parameter::scalings::EXPONENTIAL;
		modLimitMin = +1.e-6;
		modLimitMax = 2;
		modDepthMin = -2;
		modDepthMax = 2;
		defaultModDepth = .5;
	}
};

class WaveDensity : public myparams
{
public:
	WaveDensity(ParamManager * ptr) : myparams(ptr) {}
	virtual ~WaveDensity() = default;

	void initializeParameter() override
	{
		jassert(mini > 0);
		scaling = jura::Parameter::scalings::EXPONENTIAL;
		modMode = jura::ModulationConnection::modModes::ABSOLUTE;
		modDepthMin = -maxi;
		modDepthMax = +maxi;
		defaultModDepth = 1;
	}
};

class WaveSharpness : public myparams
{
public:
	WaveSharpness(ParamManager * ptr) : myparams(ptr) {}
	virtual ~WaveSharpness() = default;

	void initializeParameter() override
	{
		modLimitMin = -1;
		modLimitMax = +1;
		modDepthMin = -1;
		modDepthMax = +1;
		defaultModDepth = .5;
		mapper = new jura::rsParameterMapperTanh(mini, maxi, 3);
	}
};

class LinearTime : public myparams
{
public:
	LinearTime(ParamManager * ptr) : myparams(ptr) {}
	virtual ~LinearTime() = default;

	void initializeParameter() override
	{
		modLimitMin = 0;
		modLimitMax = +1.e+100;
		modDepthMin = -maxi;
		modDepthMax = +maxi;
		defaultModDepth = maxi/10.0;
		stringConvertFunc = elan::secondsToStringWithUnitTotal4;
	}
};

class ExponentialTime : public myparams
{
public:
	ExponentialTime(ParamManager * ptr) : myparams(ptr) {}
	virtual ~ExponentialTime() = default;

	void initializeParameter() override
	{
		jassert(mini > 0);
		scaling = jura::Parameter::scalings::EXPONENTIAL;
		modLimitMin = 0;
		modLimitMax = +1.e+100;
		modDepthMin = -maxi;
		modDepthMax = +maxi;
		defaultModDepth = maxi/10.0;
		stringConvertFunc = elan::secondsToStringWithUnitTotal4;
	}
};

class Decibel : public myparams
{
public:
	Decibel(ParamManager * ptr) : myparams(ptr) {}
	virtual ~Decibel() = default;

	void initializeParameter() override
	{
		modLimitMin = -1.1+100;
		modLimitMax = +1.e+100;
		modDepthMin = -maxi;
		modDepthMax = +maxi;
		defaultModDepth = 6;
		stringConvertFunc = elan::decibelsToStringWithUnit2;
	}
};

class Percent : public myparams
{
public:
	Percent(ParamManager * ptr) : myparams(ptr) {}
	virtual ~Percent() = default;

	void initializeParameter() override
	{
		modLimitMin = mini;
		modLimitMax = maxi;
		modDepthMin = -maxi;
		modDepthMax = +maxi;
		defaultModDepth = maxi/10.0;
		stringConvertFunc = elan::percentToStringWith2Decimals;
	}
};

class BipolarRate : public myparams
{
public:
	BipolarRate(ParamManager * ptr) : myparams(ptr) {}
	virtual ~BipolarRate() = default;

	void initializeParameter() override
	{
		modDepthMin = -10;
		modDepthMax = +10;
		defaultModDepth = +1;
		stringConvertFunc = elan::bipolarRateToString;
		mapper = new jura::rsParameterMapperSinh(mini, maxi, 4);
	}
};

class BipolarCoarseFrequency : public myparams
{
public:
	BipolarCoarseFrequency(ParamManager * ptr) : myparams(ptr) {}
	virtual ~BipolarCoarseFrequency() = default;

	void initializeParameter() override
	{
		modDepthMin = -8000;
		modDepthMax = +8000;
		defaultModDepth = +100;
		stringConvertFunc = elan::hertzToStringWithUnitTotal3_bipolar;
		mapper = new jura::rsParameterMapperSinh(mini, maxi, 4);
	}
};

class BipolarFineFrequency : public myparams
{
public:
	BipolarFineFrequency(ParamManager * ptr) : myparams(ptr) {}
	virtual ~BipolarFineFrequency() = default;

	void initializeParameter() override
	{
		modDepthMin = -100;
		modDepthMax = +100;
		defaultModDepth = +20;
		stringConvertFunc = elan::hertzToStringWithUnitTotal4_bipolar;
		mapper = new jura::rsParameterMapperSinh(mini, maxi, 4);
	}
};

class Oversampling : public myparams
{
public:
	Oversampling(ParamManager * ptr) : myparams(ptr) {}
	virtual ~Oversampling() = default;

	void initializeParameter() override
	{
		modLimitMin = -maxi;
		modLimitMax = +maxi;
		defaultModDepth = 0;
		interval = 1;
		shouldBeSmoothed = false;
		stringConvertFunc = elan::StringFunc0WithX;
	}
};

class Octave : public myparams
{
public:
	Octave(ParamManager * ptr) : myparams(ptr) {}
	virtual ~Octave() = default;

	void initializeParameter() override
	{
		scaling = jura::Parameter::scalings::LINEAR_BIPOLAR;
		modDepthMin = -8;
		modDepthMax = +8;
		defaultModDepth = +2;
		stringConvertFunc = elan::octavesToStringWithUnit0;
		interval = 1;
		shouldBeSmoothed = false;
		stringConvertFunc = elan::StringFunc0;
	}
};

class Semitone : public myparams
{
public:
	Semitone(ParamManager * ptr) : myparams(ptr) {}
	virtual ~Semitone() = default;

	void initializeParameter() override
	{
		scaling = jura::Parameter::scalings::LINEAR_BIPOLAR;
		modDepthMin = -72;
		modDepthMax = +72;
		defaultModDepth = 12;
		stringConvertFunc = elan::semitonesToStringWithUnit2;
	}
};

class Harmonic : public myparams
{
public:
	Harmonic(ParamManager * ptr) : myparams(ptr) {}
	virtual ~Harmonic() = default;

	void initializeParameter() override
	{
		scaling = jura::Parameter::scalings::LINEAR_BIPOLAR;
		modDepthMin = 0;
		modDepthMax = maxi;
		defaultModDepth = 4;
		interval = 1;
		shouldBeSmoothed = false;
		stringConvertFunc = elan::StringFunc0;
	}
};

class VelocityInfluence : public myparams
{
public:
	VelocityInfluence(ParamManager * ptr) : myparams(ptr) {}
	virtual ~VelocityInfluence() = default;

	void initializeParameter() override
	{
		stringConvertFunc = elan::StringFunc2;
	}
};

class Tempo : public myparams
{
public:
	Tempo(ParamManager * ptr) : myparams(ptr) {}
	virtual ~Tempo() = default;

	void initializeParameter() override
	{
		interval = 1;
		stringConvertFunc = elan::tempoMulToString;
	}
};

} // namespace ParameterProfile

class ParamManager
{
	friend class myparams;
public:
	ParamManager() {}
	
	void addParameters(vector<myparams*> list);
	void addParameters(myparams* ptr);

	void instantiateParameters(jura::AudioModule * audioModulePtr)
	{
		int i = 0;

		for (auto & p : ParamList)
		{
			p->instantiateParameter();
			audioModulePtr->addObservedParameter(p->ptr);

			p->id = i++;
			if (p->shouldBeSmoothed)
				paramsThatShouldBeSmoothed.push_back(p);
		}
	}

	vector<myparams*>::iterator begin() { return ParamList.begin(); }
	vector<myparams*>::iterator end() { return ParamList.end(); }
	vector<myparams*>::const_iterator begin() const { return ParamList.cbegin(); }
	vector<myparams*>::const_iterator end() const { return ParamList.cend(); }

	size_t size() const { return ParamList.size(); }

	// convenience function to set smoothing amount for all smoothers
	void setGlobalSmoothingAmount(double v);

protected:
	vector<myparams*> ParamList;
	vector<myparams*> paramsThatShouldBeSmoothed;

	void addForSmoothing(myparams * sp);
	void removeForSmoothing(myparams * sp);
};
