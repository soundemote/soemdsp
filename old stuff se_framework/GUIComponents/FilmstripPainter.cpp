#include "TempBox.h"
#include "FilmstripPainter.h"

Filmstrip::Filmstrip(const void* data, const int size, int singleFrameWidth, int singleFrameHeight)
{
	image = juce::ImageFileFormat::loadFrom(data, size);
	imageIsVertical = image.getBounds().getWidth() < image.getBounds().getHeight();

	frameWidth = singleFrameWidth;
	frameHeight = singleFrameHeight;

	numFrames = imageIsVertical ? image.getBounds().getHeight() / frameHeight : image.getBounds().getWidth() / frameWidth;

	numFrames -= 1;
}

Filmstrip::Filmstrip(const void* data, const int size, int frames, bool isVertical)
{
	image = juce::ImageFileFormat::loadFrom(data, size);
	imageIsVertical = isVertical;
	numFrames = frames;

	if (imageIsVertical)
	{
		frameHeight = image.getBounds().getHeight() / numFrames;
		frameWidth = image.getBounds().getWidth();
	}
	else
	{
		frameHeight = image.getBounds().getHeight();
		frameWidth = image.getBounds().getWidth() / numFrames;
	}
}

Rectangle<int> Filmstrip::getFrameBounds() { return { 0, 0, getFrameWidth(), getFrameHeight() }; }

int Filmstrip::getFrameWidth() { return frameWidth; }

int Filmstrip::getFrameHeight() { return frameHeight; }

int Filmstrip::getNumFrames() { return numFrames; }

Image& Filmstrip::getImage() { return image; }

void Filmstrip::draw(Graphics& g, juce::Rectangle<float> area, float normalizedValue)
{
	int frameX, frameY;
	if (imageIsVertical)
	{
		frameX = 0;
		frameY = (int)round(normalizedValue * numFrames) * frameHeight;
		lastUsedFrame = frameY;
	}
	else
	{
		frameX = (int)round(normalizedValue * numFrames) * frameWidth;
		frameY = 0;
		lastUsedFrame = frameX;
	}

	g.drawImage(image, (int)area.getX(), (int)area.getY(), (int)area.getWidth(), (int)area.getHeight(), frameX, frameY, frameWidth, frameHeight);
}

void Filmstrip::draw(Graphics& g, juce::Rectangle<float> area, int frame)
{
	int frameX, frameY;
	if (imageIsVertical)
	{
		frameX = 0;
		frameY = frame * frameHeight;
	}
	else
	{
		frameX = frame * frameWidth;
		frameY = 0;
	}

	lastUsedFrame = frame;

	g.drawImage(image, (int)area.getX(), (int)area.getY(), (int)area.getWidth(), (int)area.getHeight(), frameX, frameY, frameWidth, frameHeight);
}

Rectangle<int> Filmstrip::Painter::getFrameBounds() { return ptr.getFrameBounds(); }

int Filmstrip::Painter::getFrameWidth() { return ptr.getFrameWidth(); }

int Filmstrip::Painter::getFrameHeight() { return ptr.getFrameHeight(); }

int Filmstrip::Painter::getNumFrames() { return  ptr.getNumFrames(); }

Image& Filmstrip::Painter::getImage() { return ptr.getImage(); }

void Filmstrip::Painter::draw(Graphics& g, juce::Rectangle<float> area) { ptr.draw(g, area, currentFrame); }

void Filmstrip::Painter::draw(Graphics& g, juce::Rectangle<float> area, int frame)
{
	ptr.draw(g, area, frame);
	currentFrame = ptr.lastUsedFrame;
}

void Filmstrip::Painter::draw(Graphics& g, juce::Rectangle<float> area, float normalizedValue)
{
	ptr.draw(g, area, normalizedValue);
	currentFrame = ptr.lastUsedFrame;
}

void Filmstrip::Painter::incrementFrame()
{
	currentFrame = wrapSingleOctave_Bipolar(++currentFrame, ptr.numFrames);
}

void Filmstrip::Painter::decrementFrame()
{
	currentFrame = wrapSingleOctave_Bipolar(--currentFrame, ptr.numFrames);
}
