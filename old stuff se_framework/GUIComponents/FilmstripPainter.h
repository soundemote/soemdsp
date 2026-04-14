#pragma once

class Filmstrip
{
	friend class FilmstripPainter;

public:
	Filmstrip(const void* data, const int size, int singleFrameWidth, int singleFrameHeight);
	Filmstrip(const void* data, const int size, int numframes, bool isVertical = true);

	enum 
	{
		HORIZONTAL,
		VERTICAL
	};

	Rectangle<int> getFrameBounds();
	int getFrameWidth();
	int getFrameHeight();
	int getNumFrames();
	Image& getImage();

	void draw(Graphics& g, juce::Rectangle<float> area, float normalizedValue);

	void draw(Graphics& g, juce::Rectangle<float> area, int frame);

	class Painter;

protected:
	Image image;

	int numFrames = 0;
	int frameWidth = 0;
	int frameHeight = 0;

	int lastUsedFrame = 0;

	bool imageIsVertical = true;
};

class Filmstrip::Painter
{
public:
	Painter(Filmstrip& filmstrip) : ptr(filmstrip) {}
	~Painter() = default;

	Rectangle<int> getFrameBounds();
	int getFrameWidth();
	int getFrameHeight();
	int getNumFrames();
	Image& getImage();

	void draw(Graphics& g, juce::Rectangle<float> area);

	void draw(Graphics& g, juce::Rectangle<float> area, int frame);

	void draw(Graphics& g, juce::Rectangle<float> area, float normalizedValue);

	void incrementFrame();

	void decrementFrame();

protected:
	Filmstrip& ptr;

	int currentFrame = 0;
};
