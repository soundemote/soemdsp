#pragma once

#include "JuceHeader.h"

class ArcPainter
{
public:
	ArcPainter();
	~ArcPainter() = default;

	void setBounds(juce::Rectangle<float> area);
	void setBounds(juce::Point<float> center, float diameter);

	void setMinMaxRotationDegrees(float min, float max);
	void setArcThickness(float v);
	void setArcMargin(float v);

	void draw(juce::Graphics & g, float newDiameterForScaling = -1);

protected:
	float minRotationDegrees = -135.f;
	float maxRotationDegrees = +135.f;

	float thickness = 3;
	float margin = 0;

	float diameter = 20.f;
	float centerX = 10.f;
	float centerY = 10.f;

	void recalculateArcRadius();
	void recalculateRadians();

	// coefficients
	float minRotationRadians;
	float maxRotationRadians;
	float arcRadius;
};
