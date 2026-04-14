#pragma once

class BackgroundImage : public Component
{
public:
	BackgroundImage() = default;
	~BackgroundImage() = default;

	void setImage(Image * v);
	juce::Rectangle<int> getImageBounds();

protected:
	virtual void paint(Graphics & g) override;

	Image * image;
	juce::Rectangle<int> bounds;
};

class BackgroundImageWithOpacity : public BackgroundImage
{
public:
	BackgroundImageWithOpacity() = default;
	~BackgroundImageWithOpacity() = default;

	void setOpacity(float v);
	void setBackgroundColor(Colour v);

protected:
	void paint(Graphics & g) override;

	float opacity = 1.f;
	Colour backgroundColor{ 255,255,255 };
};
