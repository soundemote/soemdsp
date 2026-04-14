#pragma once

#pragma warning(disable: 4267) // size_t to GLsizei
#pragma warning(disable: 4244) // size_t to to int

/*
needs:
it is apparently easy to rescale the textures for non-corruptive-scope-gui-rescaling
proper waterfall effect?
need gamma correction
blur, see: http://in2gpu.com/2014/09/24/render-to-texture-in-opengl/
other saturation functions for more or less visible low intensity pixels
draw lines with dots
switch for lengthwise colormap
*/


#ifndef _WIN32
#include "../JuceLibraryCode/JuceHeader.h"

// Silence macro redefinition warnings
#undef GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT
#undef GL_DRAW_FRAMEBUFFER_BINDING
#undef GL_INVALID_INDEX
#undef GL_TIMEOUT_IGNORED

#endif // !_WIN32

#include <algorithm>

// Temporarily silence 'Compiler will not invoke errors if using removed OpenGL functionality.'
// This is not a good sign however, we should definitely fix this
//#define GL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED

// Enable 1D mode fix hack attempt
#define LINE1D_FIX_HACK 0

using std::min;
using std::max;

//#ifdef JUCE_DEBUG
void _check_gl_error(const char* file, int line, const char* fn_name = nullptr);
void _WriteDebugLog(const char* sinfo, int debugint, std::vector<bool> & logChecks);

void WriteLog(const juce::String& msg);

inline void WriteLog(const juce::String& msg) { DBG(msg); jura::writeToLogFile(msg); }

#if JUCE_DEBUG
#define ENABLE_GL_LOGGING 1
#else
#define ENABLE_GL_LOGGING 1
#endif // JUCE_DEBUG

#if ENABLE_GL_LOGGING

#if JUCE_MSVC || JUCE_GCC
#define glErrorCheck() _check_gl_error(__FILE__, __LINE__, __FUNCTION__)
#else
#define glErrorCheck() _check_gl_error(__FILE__, __LINE__, __PRETTY_FUNCTION__)
#endif // JUCE_MSVC || JUCE_GCC

#else
#define glErrorCheck() JUCE_BLOCK_WITH_FORCED_SEMICOLON ( ; )
#endif // ENABLE_GL_LOGGING

#if JUCE_MSVC || JUCE_GCC
#define WriteDebugLog(debugint) _WriteDebugLog(__FUNCTION__ "() l." JUCE_STRINGIFY(__LINE__) " ", debugint, logChecks)
#else
#define WriteDebugLog(debugint) _WriteDebugLog(__FILE__ ":" JUCE_STRINGIFY(__LINE__) " ", debugint, logChecks)
#endif // JUCE_MSVC || JUCE_GCC

//#else
//#define glErrorCheck {}
//#endif

struct RGBA
{
	RGBA(GLfloat r = 0.0, GLfloat g = 0.0, GLfloat b = 0.0, GLfloat a = 0.0) : r(r), g(g), b(b), a(a) {}
	~RGBA() = default;

	union
	{
		struct
		{
			GLfloat r;
			GLfloat g;
			GLfloat b;
			GLfloat a;
		};
		GLfloat v[4];
	};

	GLfloat& operator[] (unsigned i) { return v[i]; }
	GLfloat operator[] (unsigned i) const { return v[i]; }
};

class ColorMem
{
public:

	enum ColorMode { RGB, HSL, TieDie, Rainbow };

	ColorMem() = default;
	~ColorMem() = default;

	void r(float v);
	void g(float v);
	void b(float v);

	void h(float v);
	void s(float v);
	void l(float v);

	Colour getColor();
	RGBA getOpenGLColor();	

	void setColorMode(ColorMode m)
	{
		colorMode = m;
		updateColor();
	}

protected:

	void updateColor();
	void updateRGBA();
	void updateHSLA();
	//void updateHSLAQuantized();

	float 
		alpha = 1.0;
	float
		red = 1.0,
		green = 1.0,
		blue = 1.0;
	float
		hue = 1.0,
		saturation = 1.0,
		lightness = 1.0;

	Colour rgba;
	ColorMode colorMode = ColorMode::RGB;
};

class Brush
{
public:

	enum Type { line, dot };

	Brush() = default;
	Brush(Brush::Type t) : type(t) {}

	virtual ~Brush() = default;

	void setColorMode(ColorMem::ColorMode v);

	void setRed(float v);
	void setGreen(float v);
	void setBlue(float v);

	void setHue(float v);
	void setSaturation(float v);
	void setLightness(float v);

	bool doDraw = true;

	virtual void passVariablesToShaderProgram(ScopedPointer<OpenGLShaderProgram> & shader) const = 0;

public:

	RGBA color ={ 0.1f,1.0f,0.1f,1.0f };
	ColorMem colorMem;
	float intensity = 0.3f;
	float size = 0.04f;
	float lengthFade = 1.0f; //typically 0 to num samples
	float blur = 0.0;
	int profile = 0;
	ColorMem::ColorMode colorMode = ColorMem::ColorMode::HSL;
	Type type = Brush::Type::dot;
};

class LineBrush : public Brush
{
public:
	LineBrush() { type = Brush::Type::line; }
	virtual ~LineBrush() = default;

	float lenColorIntensity = 0;

	void passVariablesToShaderProgram(ScopedPointer<OpenGLShaderProgram> & shader) const override
	{
		shader->setUniform("uSize", size);
		shader->setUniform("uIntensity", intensity);
		shader->setUniform("uBlur", blur);
		shader->setUniform("brushColor", color.r, color.g, color.b, color.a);
		shader->setUniform("tailFade", lengthFade);
		shader->setUniform("brushProfile", profile);
		shader->setUniform("colorMode", colorMode);

		shader->setUniform("lenColorIntensity", lenColorIntensity);
	}
};

class DotBrush : public Brush
{
public:
	DotBrush() { type = Brush::Type::dot; }
	virtual ~DotBrush() = default;

	void passVariablesToShaderProgram(ScopedPointer<OpenGLShaderProgram> & shader) const override
	{
		shader->setUniform("uSize", size);
		shader->setUniform("uIntensity", intensity);
		shader->setUniform("uBlur", blur);
		shader->setUniform("brushColor", color.r, color.g, color.b, color.a);
		shader->setUniform("tailFade", lengthFade);
		shader->setUniform("brushProfile", profile);
		shader->setUniform("colorMode", colorMode);
	}
};



/* ORIGINAL ORIGINAL ORIGINAL LINE SHADER CODE

// Line.frag
const char* lineFragShaderString = R"%%(
#version 330

#define EPS 1E-6
#define TAU 6.283185307179586
#define TAUR 2.5066282746310002
#define SQRT2 1.4142135623730951

precision highp float;
uniform float tailFade;
uniform float uSize;
uniform float LenFactor;
uniform float uIntensity;
uniform float uBlur;
uniform float lenColorIntensity;
uniform vec4 brushColor;
uniform int brushProfile;
uniform int colorMode;
uniform int nSamples;
uniform vec2 ScreenSize;
in vec4 uvl;
out vec4 color;

// A standard gaussian function, used for weighting samples
float gaussian(float x, float sigma) {
	return exp(-(x * x) / (2.0 * sigma * sigma)) / (TAUR * sigma);
}

// This approximates the error function, needed for the gaussian integral
float erf(float x) {
	float s = sign(x), a = abs(x);
	x = 1.0 + (0.278393 + (0.230389 + 0.078108 * (a * a)) * a) * a;
	x *= x;
	return s - s / (x * x);
}

void main(void)
{
		float len = uvl.z;
		vec2 xy = uvl.xy;
		float alpha;

		float sigma = uSize/4.0;
		if (len < EPS) {
		// If the beam segment is too short, just calculate intensity at the position.
				alpha = exp(-pow(length(xy),2.0)/(2.0*sigma*sigma))/2.0/sqrt(uSize);
		} else {
		// Otherwise, use analytical integral for accumulated intensity.
				alpha = erf(xy.x/SQRT2/sigma) - erf((xy.x-len)/SQRT2/sigma);
				alpha *= exp(-xy.y*xy.y/(2.0*sigma*sigma))/2.0/len*uSize;
		}

		float nSamps = nSamples;

		float afterglow = smoothstep(0.0, 0.33, uvl.w/nSamps);
		alpha *= afterglow * uIntensity;
		color = vec4(brushColor.r, brushColor.g, brushColor.b, alpha);
}
)%%";


// Line.vert
const char* lineVertShaderString = R"%%(
#version 330

#define EPS 1E-6

precision highp float;

uniform float uSize;
uniform vec2 ScreenSize;
uniform vec2 ScaleFactor;
in vec2 aStart, aEnd;
in float aIdx;
out vec4 uvl;

void main()
{
		float tang;
		vec2 current;
		// All points in quad contain the same data:
		// segment start point and segment end point.
		// We determine point position using its index.
		float idx = mod(aIdx,4.0);

		// `dir` vector is storing the normalized difference
		// between end and start
		vec2 dir = aEnd-aStart;
		uvl.z = length(dir);

		if (uvl.z > EPS) {
				dir = dir / uvl.z;
		} else {
		// If the segment is too short, just draw a square
				dir = vec2(1.0, 0.0);
		}
		// norm stores direction normal to the segment difference
		vec2 norm = vec2(-dir.y, dir.x);

		// `tang` corresponds to shift "forward" or "backward"
		if (idx >= 2.0) {
				current = aEnd;
				tang = 1.0;
				uvl.x = -uSize;
		} else {
				current = aStart;
				tang = -1.0;
				uvl.x = uvl.z + uSize;
		}
		// `side` corresponds to shift to the "right" or "left"
		float side = (mod(idx, 2.0)-0.5)*2.0;
		uvl.y = side * uSize;
		uvl.w = floor(aIdx / 4.0 + 0.5);

		gl_Position = vec4((current+(tang*dir+norm*side)*uSize)*ScaleFactor,0.0,1.0);
}

*/
