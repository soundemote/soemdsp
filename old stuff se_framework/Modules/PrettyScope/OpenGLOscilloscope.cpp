#include "OpenGLOscilloscope.h"

#include "Shaders.h" // file is generated in pre-build step

#include <functional>
#include <vector>

#if !JUCE_DEBUG && ENABLE_GL_LOGGING
JUCE_COMPILER_WARNING("OpenGL logging enabled in release mode, this forces a pipeline stall and reduces graphics performance")
#endif // JUCE_DEBUG

//#ifdef JUCE_DEBUG
void _check_gl_error(const char* file, int line, const char* fn_name)
{
	if (false == OpenGLHelpers::isContextActive())
	{
		juce::String error("ERROR ");

		error += file;
		error += " ";

		if (fn_name) { error += fn_name; error += " "; }

		error += "l.";
		error += line;
		error += " -> No current GL context !!";
		DBG(error);

		error += "\n";
		jura::writeToLogFile(error);

		return;
	}

	// WARNING glGetError should only be called while context is current
	GLenum err(glGetError());

	while (err != GL_NO_ERROR)
	{
		juce::String error(fn_name ? fn_name : "");

		error += " error: GL_";

		switch (err)
		{
		case GL_INVALID_OPERATION:  error += "INVALID_OPERATION";   break;
		case GL_INVALID_ENUM:       error += "INVALID_ENUM";        break;
		case GL_INVALID_VALUE:      error += "INVALID_VALUE";       break;
		case GL_OUT_OF_MEMORY:      error += "OUT_OF_MEMORY";       break;
		default:										error += juce::String(err);		  break;
		}

		error += " - "; error += file; error += ":"; error += line;

		DBG(error);

		error += '\n';

		jura::writeToLogFile(error);

		//jassertfalse;

		err = glGetError();
	}
}

void _WriteDebugLog(const char* sinfo, int debugint, std::vector<bool> & logChecks)
{
	if (logChecks[debugint] == true)
	{
		logChecks[debugint] = false;

		juce::String msg(sinfo);
		msg += "id: ";
		msg += debugint;
		msg +=" was hit.";

		DBG(msg);

		msg += "\n";

		juce::File logFile = File::getSpecialLocation(File::currentApplicationFile).withFileExtension(String("log"));

		if (!logFile.existsAsFile())
			logFile.create();

		if (logFile.getSize() < 1000000)
			logFile.appendText(msg);
#if ENABLE_GL_LOGGING
		_check_gl_error(sinfo, debugint);
#endif // ENABLE_GL_LOGGING
	}
}

#if JUCE_OPENGL3

juce::String translateVertexShaderToV3(const juce::String& code)
{
	if (OpenGLShaderProgram::getLanguageVersion() > 1.2)
		return code.replace ("attribute", "in")
		.replace ("varying", "out");

	return code;
}

juce::String translateFragmentShaderToV3(const juce::String& code)
{
	if (OpenGLShaderProgram::getLanguageVersion() > 1.2)
		return //JUCE_GLSL_VERSION "\n" "out vec4 fragColor;\n"
		code.replace ("varying", "in")
		.replace ("texture2D", "texture")
		.replace ("gl_FragColor", "fragColor");

	return code;
}

#else
const juce::String& translateVertexShaderToV3(const juce::String& code) { return code; }
const juce::String& translateFragmentShaderToV3(const juce::String& code) { return code; }
#endif // JUCE_OPENGL3

MainContentComponent::MainContentComponent() : openglOscilloscope(openGLContext)
{
	logChecks = std::vector<bool>(10000, true);
	setSize(width, height);
	openglOscilloscope.setSize(width, height);

#if 1
	openGLContext.setOpenGLVersionRequired(juce::OpenGLContext::OpenGLVersion::openGL3_2);
#else
#error Shaders are written using GL > 3.x, we must therefore request at least a 3.x context
#endif // 1
}

void MainContentComponent::initialise()
{
	ScopedLock lock(openGLCriticalSection);

	typedef juce::String String;

	const String s_header("---------------------------------------------------------\n");

	// Log various info for forensic purposes
	{
		using namespace jura;
		clearLogFile();
		WriteLog(s_header);
		WriteLog("PrettyScope "+juce::String(JucePlugin_VersionString)+" (built on " __DATE__ " at " __TIME__ ")\n");
		WriteLog(s_header);

		const juce::Time currentTime = juce::Time::getCurrentTime();

		WriteLog("\n\tInitialization started at " + currentTime.toString(true, true) + "\n");

		typedef juce::SystemStats SysStats;

		//WriteLog("\tCPU: " + SysStats::getCpuModel() + "\n"); // JUCE 5
		WriteLog("\tCPU: " + SysStats::getCpuVendor()
			+ " @ " + String(1e-3f * SysStats::getCpuSpeedInMegahertz()) + "GHz - " + String(SysStats::getNumCpus()) + " cores, "
			+ String(1e-3f * SysStats::getMemorySizeInMegabytes()) + "Gb RAM\n");

		WriteLog("\tOS:  " + SysStats::getOperatingSystemName()
			+ " " + (SysStats::isOperatingSystem64Bit() ? "64" : "32") + "bit\n");

#if JUCE_DEBUG
		WriteLog("GL context: " + String(OpenGLHelpers::isContextActive() ? "current" : "not current") + "\n");

		//openGLContext
#endif // JUCE_DEBUG
	}

	// {Lorcan} BUG we sometimes get crashes here, probably due to something performing GL commands before context was initialized
#if GL_LOADER_GLAD
	if (!gladLoadGL())
	{
		WriteLog("ERROR: GL loader failed to initialize!\n");
		failedToStart = true;
		return;
	}
#else
	else if (glewInit() != GLEW_OK)
	{
		WriteLog("ERROR: GLEW failed to initialize!\n");
		failedToStart = true;
		return;
	}
#endif // GL_LOADER_GLAD

	const char* version = reinterpret_cast<const char*>(glGetString(GL_VERSION));

	if ((nullptr != version) && (version[0] < '3'))
	{
		const char* versionx = (nullptr != version) ? version : "null";
		WriteLog("ERROR: OpenGL versions under 3.0 are not supported! (OpenGL version " + String(versionx) + " detected.)\n");
		failedToStart = true;
		return;
	}

	GLint majorVersion = 0, minorVersion = 0;
	glGetIntegerv(GL_MAJOR_VERSION, &majorVersion); WriteDebugLog(200);
	glGetIntegerv(GL_MINOR_VERSION, &minorVersion); WriteDebugLog(201);
	{
		WriteLog(s_header);
		WriteLog("OPENGL adapter details\n");
		WriteLog(s_header);

		const char* sGLSL = reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION));
		const char* sVendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
		const char* sRenderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));

		WriteLog("\tVersion:      " + juce::String(version) + "\n");
		WriteLog("\tGLSL:         " + juce::String(sGLSL ? sGLSL : "?") + "\n");
		WriteLog("\tVendor:       " + juce::String(sVendor ? sVendor : "?") + "\n");
		WriteLog("\tRenderer:     " + juce::String(sRenderer ? sRenderer : "?") + "\n\n");

		GLint lineWidths[2] = { -1, -1 };
		GLint pointWidths[2] = { -1, -1 };

		glGetIntegerv(GL_ALIASED_LINE_WIDTH_RANGE, lineWidths);
		glGetIntegerv(GL_ALIASED_LINE_WIDTH_RANGE, pointWidths);

		WriteLog("\tLine range:   " + juce::String(lineWidths[0]) + " .." + juce::String(lineWidths[1]) + "\n");
		WriteLog("\tPoint range:  " + juce::String(pointWidths[0]) + " .." + juce::String(pointWidths[1]) + "\n");

		if (minorVersion < 3)
			WriteLog("\n\nNOTE: GL version is less than 3.3, PrettyScope will most likely crash.\n\n");
	}
	//glDebugMessageCallback(&debugCallback, nullptr);

	openGLContext.setContinuousRepainting(true); WriteDebugLog(203);
	openGLContext.setSwapInterval(openglOscilloscope.framerate); WriteDebugLog(204);
	openglOscilloscope.startup(); WriteDebugLog(205);

	basicShader = new OpenGLShaderProgram(openGLContext); WriteDebugLog(206);
	basicShader->addVertexShader(translateVertexShaderToV3(basicVertShaderString)); WriteDebugLog(207);
	basicShader->addFragmentShader(translateFragmentShaderToV3(textureFragShaderString)); WriteDebugLog(208);
	basicShader->link(); WriteDebugLog(209);

	combineShader = new OpenGLShaderProgram(openGLContext); WriteDebugLog(210);
	combineShader->addVertexShader(translateVertexShaderToV3(basicVertShaderString)); WriteDebugLog(210);
	combineShader->addFragmentShader(translateFragmentShaderToV3(combineFragShaderString)); WriteDebugLog(211);
	combineShader->link(); WriteDebugLog(212);

	colormapShader = new OpenGLShaderProgram(openGLContext); WriteDebugLog(213);
	colormapShader->addVertexShader(translateVertexShaderToV3(basicVertShaderString)); WriteDebugLog(214);
	colormapShader->addFragmentShader(translateFragmentShaderToV3(colormapFragShaderString)); WriteDebugLog(215);
	colormapShader->link(); WriteDebugLog(216);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // debug

	float vertices[] = // position (x, y), texture coordinates(u, v)
	{
		-1, -1,  0,  0,
		-1,  1,  0,  1,
		 1, -1,  1,  0,
		 1,  1,  1,  1,
	};

	glGenBuffers(1, &fullScreenQuadBufferID); WriteDebugLog(217);
	glBindBuffer(GL_ARRAY_BUFFER, fullScreenQuadBufferID); WriteDebugLog(218);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); WriteDebugLog(219);
	glBindBuffer(GL_ARRAY_BUFFER, 0); WriteDebugLog(210);

	glGenFramebuffers(1, &frameBufferID); WriteDebugLog(211);
	glGenTextures(TEXTURE_COUNT, textureIDs); glErrorCheck(); WriteDebugLog(212);
	updateTextureDimensions(); WriteDebugLog(213);
}

void MainContentComponent::resized()
{
	ScopedLock lock(openGLCriticalSection);  // prevent changes to size during rendering
	width = getWidth();
	height = getHeight();

	openglOscilloscope.resize(width, height);

	isResizing = true;
}

void MainContentComponent::updateTextureDimensions()
{
	if (frameBufferID == -1)
		return;

	WriteDebugLog(214);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBufferID); WriteDebugLog(215);
	glDrawBuffer(GL_COLOR_ATTACHMENT0); WriteDebugLog(216);
	glPixelStorei(GL_PACK_ALIGNMENT, 1); WriteDebugLog(217);

	for (unsigned i = 0; i < TEXTURE_COUNT; ++i)
	{
		glBindTexture(GL_TEXTURE_2D, textureIDs[i]); WriteDebugLog(218);
		glTexImage2D(GL_TEXTURE_2D, 0, openglOscilloscope.GL_RGBA_FORMAT, width, height, 0, GL_RGBA, GL_FLOAT, nullptr); WriteDebugLog(219);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); WriteDebugLog(220);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); WriteDebugLog(221);

		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textureIDs[i], 0); WriteDebugLog(222);
		glClearColor(0, 0, 0, 0); WriteDebugLog(223);
		glClear(GL_COLOR_BUFFER_BIT); WriteDebugLog(224);
		WriteDebugLog(225);
	}

	glErrorCheck();
}

void MainContentComponent::clearCanvas()
{
	if (frameBufferID == -1)
		return;

	glBindFramebuffer(GL_FRAMEBUFFER, frameBufferID);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	for (unsigned i = 0; i < TEXTURE_COUNT; ++i)
	{
		glBindTexture(GL_TEXTURE_2D, textureIDs[i]);
		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	glErrorCheck();
}

OpenGLOscilloscope::OpenGLOscilloscope(OpenGLContext & context)
	: context(context)
{
	logChecks = std::vector<bool>(10000, true);

	updateColorMap();
}

void OpenGLOscilloscope::set1DTransformation(double zoom, double offset, double scale)
{
	zoom_1d = zoom;
	offset_1d = offset;
	scale_1d = scale;
}

void OpenGLOscilloscope::setBlendMode(int newMode)
{
	// check if changing from alpha blended mode to non-alpha blending
	if ((blendMode == 2 || blendMode == 3 || blendMode == 4) && (newMode == 0 || newMode == 1))
		applyAlphaBlend = true;
	blendMode = newMode;
}

void MainContentComponent::renderFullScreenQuad(OpenGLShaderProgram* shaderProgram, const char* positionName, const char* texCoordName)
{
	GLint positionAttrib = glGetAttribLocation(shaderProgram->getProgramID(), positionName); WriteDebugLog(235);
	GLint texCoordAttrib = glGetAttribLocation(shaderProgram->getProgramID(), texCoordName); WriteDebugLog(236);
	jassert(positionAttrib != -1);
	jassert(texCoordAttrib != -1);

	glBindBuffer(GL_ARRAY_BUFFER, fullScreenQuadBufferID); WriteDebugLog(237);

	glEnableVertexAttribArray(positionAttrib); WriteDebugLog(238);
	glEnableVertexAttribArray(texCoordAttrib); WriteDebugLog(239);
	glVertexAttribPointer(positionAttrib, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, nullptr); WriteDebugLog(240);
	glVertexAttribPointer(texCoordAttrib, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)(sizeof(float) * 2)); WriteDebugLog(241);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); WriteDebugLog(242);

	glDisableVertexAttribArray(positionAttrib); WriteDebugLog(243);
	glDisableVertexAttribArray(texCoordAttrib); WriteDebugLog(244);
	glBindBuffer(GL_ARRAY_BUFFER, 0); WriteDebugLog(245);

	glUseProgram(0); WriteDebugLog(246);
	glErrorCheck();
}

void OpenGLOscilloscope::setSize(int w, int h)
{
	width = w;
	height = h;

	fWidth = width;
	fHeight = height;

	fMaxSize = std::max<float>(fWidth, fHeight);
	fMinSize = std::min<float>(fWidth, fHeight);

	fRatio = fMaxSize/fMinSize;

	xScale = fHeight / fMaxSize;
	yScale = fWidth / fMaxSize;
}

void OpenGLOscilloscope::resize(int w, int h)
{
	const float desktopScale = float(context.getRenderingScale()); WriteDebugLog(400);
	w = roundToInt(desktopScale * w);
	h = roundToInt(desktopScale * h);
	setSize(w, h);
}

enum : size_t { line_vertex_count = 6 };

void OpenGLOscilloscope::clearColor(float r, float g, float b, float a)
{
	glClearColor(r, g, b, a); WriteDebugLog(247);
	glClear(GL_COLOR_BUFFER_BIT); WriteDebugLog(248);
}

void OpenGLOscilloscope::updateShaders(String folderPathToShaders)
{
	String shaderStr1;
	String shaderStr2;
	String shaderStr3;

	if (FileHelper::doesDirContainFiles(folderPathToShaders))
	{
		try
		{
			FileHelper::openFolder(folderPathToShaders, true);
		}
		catch (exception & e)
		{
			// do nothing for now
		}
	}
	else
	{
		try
		{
			shaderStr1 = FileHelper::readFile(folderPathToShaders + "/someShader1.ext");
			shaderStr2 = FileHelper::readFile(folderPathToShaders + "/someShader2.ext");
			shaderStr3 = FileHelper::readFile(folderPathToShaders + "/someShader3.ext");
		}
		catch (exception & e)
		{
			// do nothing for now
		}
	}

	// DO STUFF
}

void OpenGLOscilloscope::setFramerate(double v)
{
	framerate = v;
	swapIntervalChanged = true;
}

void OpenGLOscilloscope::startup()
{
	createAndLinkShader();

	resize(width, height);
	glGenBuffers(1, &vertexBufferID); WriteDebugLog(249);
	glErrorCheck();
}

//ScopedPointer<TestData> testData;
// There's nothing to stop you recreating the shader on the fly when doing development.
void OpenGLOscilloscope::createAndLinkShader()
{
	/* --- LINE SHADER STUFF --- */

	lineShader = new OpenGLShaderProgram(context); WriteDebugLog(252);
	bool successful = lineShader->addVertexShader(translateVertexShaderToV3(lineVertShaderString));

	successful &= lineShader->addFragmentShader(translateFragmentShaderToV3(commonFragShaderString));
	successful &= lineShader->addFragmentShader(translateFragmentShaderToV3(lineFragShaderString));
	successful &= lineShader->link(); WriteDebugLog(253);

	if (!successful) { DBG(lineShader->getLastError()); jassertfalse; }

	aIdxLocation_line = glGetAttribLocation(lineShader->getProgramID(), "aIdx"); WriteDebugLog(47);
	aStartLocation_line = glGetAttribLocation(lineShader->getProgramID(), "aStart"); WriteDebugLog(48);
	aEndLocation_line = glGetAttribLocation(lineShader->getProgramID(), "aEnd"); WriteDebugLog(49);

	/* --- DOT SHADER STUFF --- */

	dotShader = new OpenGLShaderProgram(context); WriteDebugLog(254);

	successful &= dotShader->addVertexShader(translateVertexShaderToV3(dotVertShaderString));
	successful &= dotShader->addFragmentShader(translateFragmentShaderToV3(commonFragShaderString));
	successful &= dotShader->addFragmentShader(translateFragmentShaderToV3(dotFragShaderString));
	successful &= dotShader->link(); WriteDebugLog(255);

	if (!successful) { DBG(dotShader->getLastError()); jassertfalse; }

	aIdxLocation_dot = glGetAttribLocation(dotShader->getProgramID(), "aIdx"); WriteDebugLog(47);
	aStartLocation_dot = glGetAttribLocation(dotShader->getProgramID(), "aStart"); WriteDebugLog(48);

	/* --- LINEPLUSDOT SHADER STUFF (for experimental alphablend stuff) --- */

	linePlusDotShader = new OpenGLShaderProgram(context); WriteDebugLog(256);

	successful &= linePlusDotShader->addVertexShader(translateVertexShaderToV3(basicVertShaderString));
	successful &= linePlusDotShader->addFragmentShader(translateFragmentShaderToV3(linePlusDotFragShaderString));
	successful &= linePlusDotShader->link(); WriteDebugLog(257);

	if (!successful) { DBG(linePlusDotShader->getLastError()); jassertfalse; }
}

// NOTE this is called by UI callback, GL context is probably not active at this point
void OpenGLOscilloscope::setColorMap(const vector<RGBA> & cm)
{
	mColormapColors = cm;
	updateColorMap();
}

void OpenGLOscilloscope::updateColorMap()
{
	// Mark for upload to GL
	mColorMapDirty = true;

	const float fscale = 1.f / static_cast<float>(mColormapColors.size());

	mColorMapCorrection = (1.f - fscale) + (0.5f * fscale);
	// mColorMapCorrection = (1.f - (1.f / (float)cm.size())) + (0.5f / (float)cm.size());
}

void OpenGLOscilloscope::uploadColorMap()
{
	if (!mColorMapDirty)
		return;

	mColorMapDirty = false;

	glActiveTexture(GL_TEXTURE0); WriteDebugLog(228);

	// Create texture if it doesn't yet exist
	if (colormap_texture_id == 0)
	{
		glGenTextures(1, &colormap_texture_id); WriteDebugLog(229);
	}

	glBindTexture(GL_TEXTURE_1D, colormap_texture_id); WriteDebugLog(230);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); WriteDebugLog(231);// required: stop texture wrapping
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); WriteDebugLog(232);// required: scale texture with linear sampling
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); WriteDebugLog(233); // doesn't seem to be required
	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA32F, mColormapColors.size(), 0, GL_RGBA, GL_FLOAT, mColormapColors.data()); WriteDebugLog(234);// setup memory	
}

void OpenGLOscilloscope::setXYDataPoints(const ScopeAudioBuffer & audioBuffer, const vector<float> & data_x, const vector<float> & data_y)
{
	verticesList.clear();
	numSamples = audioBuffer.getNumPointsToRender();

	for (size_t i = 0; i < numSamples; i++)
	{
		int n = (audioBuffer.getPointStartToRender() + i) & audioBuffer.bufferMask; // index in circular buffer
		float x = data_x[n] * zoom_1d + offset_1d;
		float y = data_y[n] * scale_1d;

		const size_t current_size = verticesList.size();

		verticesList.resize(current_size + line_vertex_count);

		VertexData* const dst = verticesList.data() + current_size;

		VertexData vd ={ { x, y }, static_cast<int>(i * 4) };

		dst[0] = vd;
		++vd.index;

		dst[2] = vd;
		dst[3] = vd;
		++vd.index;

		dst[1] = vd;
		dst[4] = vd;
		++vd.index;

		dst[5] = vd;
	}
}

void OpenGLOscilloscope::copyVertexDataToGPU()
{
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID); WriteDebugLog(32);
	glBufferData(GL_ARRAY_BUFFER, verticesList.size() * sizeof(verticesList[0]), verticesList.data(), GL_STREAM_DRAW); WriteDebugLog(34);
	glBindBuffer(GL_ARRAY_BUFFER, NULL); WriteDebugLog(35);
}

void OpenGLOscilloscope::render(OpenGLCanvas* GLCanvas, const vector<Brush*>& brushesToRender)
{
	prepareForRender(GLCanvas);

	copyVertexDataToGPU(); WriteDebugLog(109);

	// enable color blending
	glEnable(GL_BLEND); WriteDebugLog(110);

	// choose the math of color blending
	glBlendEquation(GL_FUNC_ADD); WriteDebugLog(111);

	// setup the blend function
	switch (blendMode)
	{
	case 0:
		glBlendFunc(GL_SRC_ALPHA, GL_ONE); WriteDebugLog(112);
		for (const auto& b : brushesToRender)
			renderBrush(*b);
		break;
	case 1:
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); WriteDebugLog(112);
		for (const auto& b : brushesToRender)
			renderBrush(*b);
		break;
	case 2: // Light
		glBlendFunc(GL_SRC_ALPHA, GL_ONE); WriteDebugLog(116);
		renderBrushesAlphaBlend(GLCanvas, brushesToRender);
		break;
	case 3: // Paint (broken)
		glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ONE, GL_ONE_MINUS_SRC_ALPHA); WriteDebugLog(118);
		renderBrushesAlphaBlend(GLCanvas, brushesToRender);
		break;
	case 4: // Solid
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); WriteDebugLog(120);
		renderBrushesAlphaBlend(GLCanvas, brushesToRender);
		break;
	}

	glDisable(GL_BLEND); WriteDebugLog(122);
}

void OpenGLOscilloscope::renderBrushesAlphaBlend(OpenGLCanvas* GLCanvas, const vector<Brush*>& brushesToRender)
{
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GLCanvas->textureIDs[GLCanvas->currentTexture], 0); WriteDebugLog(124);
	clearColor(0, 0, 0, 0); WriteDebugLog(125);

	if (brushesToRender.size() > 1)
		renderBrush(*brushesToRender[1]);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GLCanvas->textureIDs[3], 0); WriteDebugLog(127);
	clearColor(0, 0, 0, 0); WriteDebugLog(128);

	if (brushesToRender.size() > 0)
		renderBrush(*brushesToRender[0]);

	// writing to texture 2, which is normally written to first
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GLCanvas->textureIDs[2], 0); WriteDebugLog(130);

	const GLuint textures[] = { GLCanvas->textureIDs[GLCanvas->currentTexture], GLCanvas->textureIDs[3] }; WriteDebugLog(131);
	const GLenum targets[] = { GL_TEXTURE_2D, GL_TEXTURE_2D };

	bind_textures(targets, textures); WriteDebugLog(132);

	linePlusDotShader->use(); WriteDebugLog(133);
	linePlusDotShader->setUniform("aboveTexture", 0); WriteDebugLog(134);
	linePlusDotShader->setUniform("belowTexture", 1); WriteDebugLog(135);

	GLCanvas->renderFullScreenQuad(linePlusDotShader, "position", "texCoord"); WriteDebugLog(136);
}

void OpenGLOscilloscope::renderBrush(const Brush & p)
{
	if (is_paused || !p.doDraw)
		return;

	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID); WriteDebugLog(50);

	switch (p.type)
	{
	case(Brush::Type::line):
		drawLines(p);
		break;
	case (Brush::Type::dot):
		drawDots(p);
		break;
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0); WriteDebugLog(34);
	glUseProgram(0); WriteDebugLog(65);
	glErrorCheck();
}

void OpenGLOscilloscope::drawLines(const Brush & p)
{
	glEnableVertexAttribArray(aIdxLocation_line); WriteDebugLog(51);
	glEnableVertexAttribArray(aStartLocation_line); WriteDebugLog(54);
	glEnableVertexAttribArray(aEndLocation_line); WriteDebugLog(56);

	ScopedPointer<OpenGLShaderProgram> & shader = lineShader;
	shader->use(); WriteDebugLog(37);

	shader->setUniform("nSamples", (GLfloat)numSamples); // why convert to float if inside shader it is declared as int?
	shader->setUniform("LenFactor", LenFactor);
	shader->setUniform("ScreenSize", fWidth, fHeight);
	shader->setUniform("ScaleFactor", xScale, yScale);

	p.passVariablesToShaderProgram(lineShader);

	glVertexAttribPointer(aIdxLocation_line, 1, GL_INT, false, sizeof(VertexData), (void*)offsetof(VertexData, index)); WriteDebugLog(52);
	glVertexAttribPointer(aStartLocation_line, 2, GL_FLOAT, false, sizeof(VertexData), (void*)offsetof(VertexData, vertices2p)); WriteDebugLog(55);
	glVertexAttribPointer(aEndLocation_line, 2, GL_FLOAT, false, sizeof(VertexData), (void*)(sizeof(VertexData) * 6 + offsetof(VertexData, vertices2p))); WriteDebugLog(57);

	glErrorCheck();

	const int vertex_size = verticesList.size();

	// mLine1DWrapVertexOffset

	enum : int { vertices_per_point = 6 };

	if (!is_paused && vertex_size > vertices_per_point)
	{
		glDrawArrays(GL_TRIANGLES, 0, vertex_size - vertices_per_point); WriteDebugLog(59);
	}
	glErrorCheck();

	glDisableVertexAttribArray(aIdxLocation_line); WriteDebugLog(60);
	glDisableVertexAttribArray(aStartLocation_line); WriteDebugLog(61);
	glDisableVertexAttribArray(aEndLocation_line); WriteDebugLog(62);
}

void OpenGLOscilloscope::drawDots(const Brush & p)
{
	glEnableVertexAttribArray(aIdxLocation_dot); WriteDebugLog(51);
	glEnableVertexAttribArray(aStartLocation_dot); WriteDebugLog(54);

	glVertexAttribPointer(aStartLocation_dot, 2, GL_FLOAT, false, sizeof(VertexData), (void*)offsetof(VertexData, vertices2p)); WriteDebugLog(79);
	glVertexAttribPointer(aIdxLocation_dot, 1, GL_INT, false, sizeof(VertexData), (void*)offsetof(VertexData, index)); WriteDebugLog(82);
	glErrorCheck();

	ScopedPointer<OpenGLShaderProgram> & shader = dotShader;
	shader->use(); WriteDebugLog(66);

	shader->setUniform("nSamples", (GLfloat)numSamples); // why convert to float if inside shader it is declared as int?
	shader->setUniform("ScreenSize", fWidth, fHeight);
	shader->setUniform("ScaleFactor", xScale, yScale);

	p.passVariablesToShaderProgram(dotShader);

	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID); WriteDebugLog(77);

	if (!verticesList.empty())
	{
		glDrawArrays(GL_TRIANGLES, 0, verticesList.size()); WriteDebugLog(87);
	}

	WriteDebugLog(87);

	glErrorCheck();

	glDisableVertexAttribArray(aIdxLocation_dot); WriteDebugLog(60);
	glDisableVertexAttribArray(aStartLocation_dot); WriteDebugLog(61);
}

void OpenGLOscilloscope::prepareForRender(OpenGLCanvas* GLCanvas)
{
	// Upload color map to GL if it has changed since last render
	uploadColorMap();

	glActiveTexture(GL_TEXTURE0); WriteDebugLog(93);
	glBindTexture(GL_TEXTURE_1D, colormap_texture_id); WriteDebugLog(94);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); WriteDebugLog(95); // required: stop texture wrapping
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); WriteDebugLog(96); // required: scale texture with linear sampling
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); WriteDebugLog(97); // doesn't seem to be required
	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA_FORMAT, mColormapColors.size(), 0, GL_RGBA, GL_FLOAT, mColormapColors.data()); WriteDebugLog(98); // setup memory

	if (applyAlphaBlend)
	{
		// first, multiply all the rgb's by their alpha value and store that in a temporary texture
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GLCanvas->textureIDs[GLCanvas->currentTexture], 0); WriteDebugLog(99);
		glClearColor(0, 0, 0, 0); WriteDebugLog(100);
		glClear(GL_COLOR_BUFFER_BIT); WriteDebugLog(101);
		glActiveTexture(GL_TEXTURE0); WriteDebugLog(102);
		glBindTexture(GL_TEXTURE_2D, GLCanvas->textureIDs[(GLCanvas->currentTexture + 1) & 1]); glErrorCheck();

		GLCanvas->basicShader->use(); WriteDebugLog(103);
		GLCanvas->basicShader->setUniform("tex", 0); WriteDebugLog(104);
		GLCanvas->basicShader->setUniform("blendMode", 2); WriteDebugLog(105); // blend mode multiplies by alpha
		GLCanvas->renderFullScreenQuad(GLCanvas->basicShader, "position", "texCoord"); WriteDebugLog(106);
		GLCanvas->currentTexture ^= 1; // swap which texture is being read from

		applyAlphaBlend = false; WriteDebugLog(107);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GLCanvas->textureIDs[2], 0); WriteDebugLog(108);
	}
}
