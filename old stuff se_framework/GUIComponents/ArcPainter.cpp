#include "ArcPainter.h"

using juce::Rectangle;
using juce::Point;
using juce::PathStrokeType;
using juce::degreesToRadians;


ArcPainter::ArcPainter()
{
	recalculateArcRadius();
	recalculateRadians();
}

void ArcPainter::setBounds(Rectangle<float> area)
{
	centerX = area.getCentre().getX();
	centerY = area.getCentre().getY();
	this->diameter = std::min(area.getWidth(), area.getHeight());
	recalculateArcRadius();
}

void ArcPainter::setBounds(Point<float> center, float diameter)
{
	centerX = center.getX();
	centerY = center.getY();
	this->diameter = diameter;
	recalculateArcRadius();
}

void ArcPainter::setMinMaxRotationDegrees(float min, float max)
{
	minRotationDegrees = min;
	maxRotationDegrees = max;
	recalculateRadians();
}

void ArcPainter::setArcThickness(float v)
{
	thickness = v;
	recalculateArcRadius();
}

void ArcPainter::setArcMargin(float v)
{
	margin = v;
	recalculateArcRadius();
}

void ArcPainter::draw(Graphics & g, float newDiameterForScaling)
{
	float scale = newDiameterForScaling > 0 ? diameter / newDiameterForScaling : 1;

	Path p;
	p.addCentredArc(centerX, centerY, arcRadius, arcRadius, 0.f, minRotationRadians, maxRotationRadians, true);
	g.strokePath(p, PathStrokeType(thickness * scale, PathStrokeType::JointStyle::mitered, PathStrokeType::EndCapStyle::butt));
}

void ArcPainter::recalculateArcRadius()
{
	arcRadius = diameter * 0.5f - thickness * 0.5f - margin;
}

void ArcPainter::recalculateRadians()
{
	minRotationRadians = degreesToRadians(minRotationDegrees);
	maxRotationRadians = degreesToRadians(maxRotationDegrees);
}
