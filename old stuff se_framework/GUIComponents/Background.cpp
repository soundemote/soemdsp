#include "Background.h"

void BackgroundImage::setImage(Image * v)
{
	image = v;
	bounds = { 0, 0, image->getWidth(), image->getHeight() };
}

juce::Rectangle<int> BackgroundImage::getImageBounds()
{
	return bounds;
}

void BackgroundImage::paint(Graphics & g)
{
	g.drawImage(*image, bounds.toFloat(), juce::RectanglePlacement::Flags::doNotResize);
}

inline void BackgroundImageWithOpacity::setOpacity(float v)
{
	opacity = v;
}

inline void BackgroundImageWithOpacity::setBackgroundColor(Colour v)
{
	backgroundColor = v;
}

void BackgroundImageWithOpacity::paint(Graphics & g)
{
	// BACKGROUND COLOR
	if (opacity < 1.f)
	{
		g.setColour(backgroundColor);
		g.fillRect(image->getBounds());
		g.setOpacity(opacity);
	}

	// IMAGE
	BackgroundImage::paint(g);
}
