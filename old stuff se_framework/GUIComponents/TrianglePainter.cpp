#include "TrianglePainter.h"

juce::Path TrianglePath::pointerWithFlatBase(juce::Rectangle<float> bounds, float stroke)
{
	juce::Path p;

	jassert(stroke * 0.5f < bounds.getWidth());

	p.startNewSubPath(ElanRect::getPoint(bounds, BottomLeft));
	p.lineTo(ElanRect::getPoint(bounds, TopCenter));
	p.lineTo(ElanRect::getPoint(bounds, BottomRight));
	p.lineTo(ElanRect::getPoint(bounds, BottomRight).translated(-stroke, 0));
	p.lineTo(ElanRect::getPoint(bounds, TopCenter).translated(0, +stroke));
	p.lineTo(ElanRect::getPoint(bounds, BottomLeft).translated(+stroke, 0));
	p.closeSubPath();
	return p;
}
