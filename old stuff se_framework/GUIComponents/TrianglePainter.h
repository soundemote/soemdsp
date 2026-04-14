#pragma once

class TrianglePath
{
public:
	TrianglePath() = default;
	~TrianglePath() = default;

	/*
	This creates a //\\ shape where the path is closed and the base is flat, so you must use a fill to color it.
	*/
	static Path pointerWithFlatBase(juce::Rectangle<float> bounds, float stroke);
};
