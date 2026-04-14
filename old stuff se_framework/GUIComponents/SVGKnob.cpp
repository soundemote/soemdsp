#include "SVGKnob.h"

SVGKnob::SVGKnob(const String & svgTick, const String & svgBackground)
	: background(svgBackground)
	, tick(svgTick)
{
}

void SVGKnob::drawKnob(Graphics & g, juce::Rectangle<float> area)
{
  float minRotationDegrees = -135.f;
  float maxRotationDegrees = +135.f;

	float minRotationRadians = juce::degreesToRadians(minRotationDegrees);
	float maxRotationRadians = juce::degreesToRadians(maxRotationDegrees);

  float x = area.getX();
  float y = area.getY();
  float width = area.getWidth();
  float height = area.getHeight();
  float centerX = x + width / 2.f;
  float centerY = y + height / 2.f;
	 
	currentValue = (float)getNormalizedValue();
  float currentValueToDegrees = juce::jmap<float>(currentValue, 0, 1, minRotationDegrees, maxRotationDegrees);
	float currentValueToRadians = juce::jmap<float>(currentValue, 0, 1, minRotationRadians, maxRotationRadians);

  //float modulatedValue = (float)getModulatableParameter()->getModulatedValue();
  //float modulatedValueToAngle = juce::jmap(modulatedValue, minValue, maxValue, minRotationDegrees, maxRotationDegrees);

	juce::Rectangle<float> fullRectangle{ 0, 0, width, height };

	scale = standardSize > 0 ? width/standardSize : 1;

	if (!arc_highlightColor.isTransparent())
	{// highlight arc
		float strokeThickness = arcUnfilledThickness * scale;
		float arc_radius = width * 0.5f - strokeThickness * 0.5f - arcFillMargin;
		Path p;
		p.addCentredArc(centerX, centerY, arc_radius, arc_radius, 0.f, 0.f, (float)TAU, true);
		g.setColour({ arc_highlightColor });
		g.strokePath(p, juce::PathStrokeType(strokeThickness, juce::PathStrokeType::JointStyle::mitered, juce::PathStrokeType::EndCapStyle::butt));
	}

	if (doDrawArcUnfilled)
	{// base arc
		float strokeThickness = arcUnfilledThickness * scale;
		float arc_radius = width * 0.5f - strokeThickness * 0.5f - arcFillMargin;
		Path p;
		p.addCentredArc(centerX, centerY, arc_radius, arc_radius, 0.f, minRotationRadians, maxRotationRadians, true);
		g.setColour(arc_unfilledColour);
		g.strokePath(p, juce::PathStrokeType(strokeThickness, juce::PathStrokeType::JointStyle::mitered, juce::PathStrokeType::EndCapStyle::butt));
	}

	if (doDrawArcFilled)
	{// value indication arc
		float strokeThickness = arcFillThickness * scale;
		float arc_radius = width * 0.5f - strokeThickness * 0.5f - arcFillMargin;
		Path p;
		p.addCentredArc(centerX, centerY, arc_radius, arc_radius, 0, minRotationRadians, currentValueToRadians, true);
		g.setColour(arc_filledColour);
		g.strokePath(p, juce::PathStrokeType(strokeThickness, juce::PathStrokeType::JointStyle::mitered, juce::PathStrokeType::EndCapStyle::butt));
	}
	
	if (background.isValid())
		background.draw(g, fullRectangle);

	if (tick.isValid())
		tick.drawWithRotation(g, fullRectangle, currentValueToDegrees - knobRotationOffset);
}

void SVGKnob::paint(Graphics & g)
{
	if (painter != nullptr)
	{
		painter->paint(g, this);
		return;
	}
	
	float width = (float)getWidth();
  float height = (float)getHeight();

	if (!backgroundColor.isTransparent())
	{
		g.setColour(backgroundColor);
		g.fillEllipse(0, 0, width, height);
	}

  knobArea.setBounds(0, 0, width, height);
  drawKnob(g, knobArea);

	{// add red overlay to indicate modulation
		jura::ModulatableParameter* mp = dynamic_cast<jura::ModulatableParameter*> (assignedParameter);
		if (mp && mp->hasConnectedSources())
		{
			g.setColour(Colour::fromFloatRGBA(1.f, 0.f, 0.f, 0.5f));
			g.drawRect(getLocalBounds().toFloat(),2.5f);
		}
	}
}

void SVGKnob::mouseDrag(const MouseEvent & e)
{
	if (e.originalComponent != this)
		return; // ignore drag on the label, when it's not 'inside' the actual slider

						//double scale = 0.01; // 1% change per pixel
	double scale = .005;
	if (ModifierKeys::getCurrentModifiers().isShiftDown()) // fine tuning via shift
		scale *= 0.0625;

	if (isEnabled())
	{
		if (!e.mods.isRightButtonDown() && !e.mods.isCommandDown())
		{

			// new, handles press/release of shift during drag correctly:
			int newDragDistance = e.getDistanceFromDragStartY();
			int dragDelta       = newDragDistance - oldDragDistance;
			oldDragDistance     = newDragDistance;  // update - from now on we only need dragDelta
			dragValue += scale*dragDelta;
			double y = normalizedValueOnMouseDown;  // in 0..1
			y -= dragValue;                         // new x
			y = clamp(y, 0.0, 1.0);

			setNormalizedValue(y);
		}
	}
}

// Sets the default knob size so that gui elements will scale accordingly.

void SVGKnob::setStandardSize(float v)
{
	standardSize = v;
}

void SVGKnob::setIsBipolar(bool v)
{
	isBipolar = v;
}

void SVGKnob::setArcUnfilledColor(juce::Colour v)
{
	arc_unfilledColour = v;
}

void SVGKnob::setArcFillColor(juce::Colour v)
{
	arc_filledColour = v;
}
