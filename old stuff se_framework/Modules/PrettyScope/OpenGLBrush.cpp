#include "OpenGLBrush.h"

void ColorMem::updateRGBA()
{
	rgba = Colour::fromFloatRGBA(red, green, blue, alpha);
}

void ColorMem::updateHSLA()
{
	rgba = Colour::fromHSV(hue, saturation, lightness, alpha);
}

void ColorMem::updateColor()
{
	switch (colorMode)
	{
	case ColorMode::RGB: updateRGBA(); break;
	case ColorMode::HSL: updateHSLA(); break;
	case ColorMode::TieDie: break;
	case ColorMode::Rainbow: break;
	}
}

void ColorMem::r(float v)
{
	red = v;
	updateColor();
}

void ColorMem::g(float v)
{
	green = v;
	updateColor();
}

void ColorMem::b(float v)
{
	blue = v;
	updateColor();
}

void ColorMem::h(float v)
{
	hue = v;
	updateColor();
}

void ColorMem::s(float v)
{
	saturation = v;
	updateColor();
}

void ColorMem::l(float v)
{
	lightness = v;
	updateColor();
}

Colour ColorMem::getColor() { return rgba; }

RGBA ColorMem::getOpenGLColor()
{
	return RGBA(rgba.getFloatRed(), rgba.getFloatGreen(), rgba.getFloatBlue(), rgba.getFloatAlpha());
}

//void ColorMem::updateHSLAQuantized()
//{
//	rgba = Colour::fromHSV(rosic::round(hue * 3.0) / 3.0, saturation, lightness, alpha);
//}

void Brush::setColorMode(ColorMem::ColorMode v)
{
	colorMem.setColorMode(colorMode = v);
	color = colorMem.getOpenGLColor();
}
void Brush::setRed(float v)
{
	colorMem.r(v); 
	color = colorMem.getOpenGLColor();
}
void Brush::setGreen(float v)
{
	colorMem.g(v); 
	color = colorMem.getOpenGLColor();
}
void Brush::setBlue(float v)
{ 
	colorMem.b(v);
	color = colorMem.getOpenGLColor(); 
}
void Brush::setHue(float v) 
{ 
	colorMem.h(v); 
	color = colorMem.getOpenGLColor(); 
}
void Brush::setSaturation(float v)
{
	colorMem.s(v); 
	color = colorMem.getOpenGLColor(); 
}
void Brush::setLightness(float v)
{
	colorMem.l(v); 
	color = colorMem.getOpenGLColor();
}
