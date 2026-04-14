#include "OpenGLCanvas.h"

#include <map>
#include <vector>

using namespace se;
using namespace jura;
using namespace rosic;
using namespace juce;

const Font OpenGLCanvas::fps_font{ "Courier New", 24, Font::bold };

OpenGLCanvas::OpenGLCanvas(PrettyScopeModule * scopeToUse, ScopeAudioBuffer * audioBufferToUse, vector<Brush*> * ArrayOfBrushesToUse) 
	:	prettyScopeModule(scopeToUse)
	, audioBuffer(audioBufferToUse)
	, brushes(ArrayOfBrushesToUse)
{
	logChecks = vector<bool>(10000, true);

  if(prettyScopeModule != nullptr)
  {
    connectParameters();
    prettyScopeModule->getColorMapPointer()->addChangeListener(this);
    updateColorMapFromScope();
  }
}

OpenGLCanvas::~OpenGLCanvas()
{
  //if(prettyScopeModule != nullptr) // why this check? it's supposed to be not null anyway (Robin)
  //{
    prettyScopeModule->getColorMapPointer()->removeChangeListener(this);
    disconnectParameters();
 // }
}

void OpenGLCanvas::connectParameters()
{
	auto m = prettyScopeModule;

	AutoPauseMode = m->getParameterByName("AutoPauseMode");
	ShiftX = m->getParameterByName("ShiftX");
	ShiftY = m->getParameterByName("ShiftY");
	ScaleX = m->getParameterByName("ShiftX");
	ScaleY = m->getParameterByName("ShiftY");
	OneDimensional = m->getParameterByName("OneDimensional");
	OneDimOffset = m->getParameterByName("OneDimOffset");
	OneDimZoom = m->getParameterByName("OneDimZoom");
	Zoom = m->getParameterByName("Zoom");
	TailFadeFactor = m->getParameterByName("TailFade");

	paramlookup[m->getParameterByName("Heatmap")] = [this](float v) { setEnableHeatmap(v >= 0.5); };
	paramlookup[m->getParameterByName("BlendMode")] = [this](float v) { openglOscilloscope.setBlendMode(int(v)); };
	paramlookup[m->getParameterByName("Framerate")] = [this](float v) { openglOscilloscope.setFramerate(v > 0.5); };
	paramlookup[m->getParameterByName("DecaySpeed")] = [this](float v) { openglOscilloscope.decaySpeed = (float)curve(v, 0.9) * 100; };
	paramlookup[m->getParameterByName("LenFactor")] = [this](float v) { openglOscilloscope.LenFactor = v; };
	paramlookup[m->getParameterByName("FreezePoints")] = [this](float v) { setFreezePoints(v > 0.5); };
	paramlookup[m->getParameterByName("ShowFPS")] = [this](float v) { setShowFPS(v > 0.5); };
	paramlookup[m->getParameterByName("Pause")] = [this](float v) { setPause(v > 0.5); };

	for (auto& p : paramlookup)
		p.first->registerParameterObserver(this);

	// triggers a call to parameterChanged for all the now connected parameters in order to initially
	// set up the display
	m->notifyParameterObservers(true);
}

void OpenGLCanvas::disconnectParameters()
{
	for (auto& p : paramlookup)
		p.first->deRegisterParameterObserver(this);
}

void OpenGLCanvas::parameterChanged(Parameter * p)
{
	paramlookup[p](p->getValue());
}

void OpenGLCanvas::changeListenerCallback(ChangeBroadcaster* source)
{
	// At the moment, the only ChangeBroadcaster we listen to is the ColorMap inside the PrettyScope
	// object - when this callback is called, it means the colormap has changed, so we need to update
	// it in the graphics code:
	updateColorMapFromScope();
}

void OpenGLCanvas::updateShaders()
{
	openglOscilloscope.updateShaders(shaderPath);
}

void OpenGLCanvas::updateColorMapFromScope()
{
	ColourGradient grad = prettyScopeModule->getColorMapPointer()->getAsColourGradient();

	int N = 1024;  // number of colors
	vector<RGBA> map(N);
	double s = 1.0 / (N-1);
	for (int i = 0; i < N; i++)
	{
		Colour c = grad.getColourAtPosition(s*i);
		map[i].r = c.getFloatRed();
		map[i].g = c.getFloatGreen();
		map[i].b = c.getFloatBlue();
		map[i].a = 1.f;
	}

	openglOscilloscope.setColorMap(map);
}

void bind_textures(const GLenum(&targets)[2], const GLuint(&textures)[2])
{

#if !GL_LOADER_GLAD
	if (glBindTextures)
	{
		// [Lorcan] glBindTextures is available since GL 4.4
		// https://www.khronos.org/opengl/wiki/GLAPI/glBindTextures
		glBindTextures(0, 2, textures);
	}
	else
#endif // !GL_LOADER_GLAD
	{
		// DBG("Error: glBindTextures null");
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(targets[0], textures[0]);

		glActiveTexture(GL_TEXTURE0 + 1);
		glBindTexture(targets[1], textures[1]);
	}
}

void OpenGLCanvas::handleAutoPause()
{
	openglOscilloscope.is_paused =
		(audioBuffer->getIsInputSilence() && AutoPauseMode->getValue() == 1.0) ||
		((!isMouseInUse && !mouseWheelWasMoved) && AutoPauseMode->getValue() == 2.0) ||
		doPause;

	mouseWheelWasMoved = false;
}

void OpenGLCanvas::paint(Graphics& gfx)
{
	ScopedLock lock(openGLCriticalSection);
	RenderDebugText(gfx);
}

void OpenGLCanvas::SaveScreenShot(String directory, String filename)
{
	// create and get a pointer to an uninitialized image
	juce::Image image(juce::Image::RGB, width, height, false);
	juce::Image::BitmapData bitmapData(image, juce::Image::BitmapData::writeOnly);

	// read pixels from the framebuffer into a buffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0); glErrorCheck();
	for (int y = 0; y < height; ++y)
		glReadPixels(0, y, width, 1, GL_BGR, GL_UNSIGNED_BYTE, bitmapData.getLinePointer(height - y - 1)); glErrorCheck();

	juce::File outDirectory = directory;

	// create directory if it doesn't exist
	if (!outDirectory.exists()) 
		if (outDirectory.createDirectory().failed())
			return;

	std::unique_ptr<juce::FileOutputStream> fileStream(outDirectory.getChildFile(filename).createOutputStream());

	if (fileStream->failedToOpen())
		return;

	// can also use GIF or JPEG
	juce::PNGImageFormat png; 

	// write the image in png format
	png.writeImageToStream(image, *fileStream); 
}

void OpenGLCanvas::ClearDebugStrings()
{
	debugStrings.clear();
}

void OpenGLCanvas::AddDebugString(String string, float x, float y, Font font, Colour color, Colour strokeColor, PathStrokeType strokeType)
{
	debugStrings.emplace_back(DebugString(string, x, y, font, color, strokeColor, strokeType));
}

void OpenGLCanvas::RenderDebugText(Graphics& gfx)
{
	for (DebugString debugString : debugStrings)
	{
		GlyphArrangement glyph;
		Path textPath;
		glyph.addLineOfText(debugString.font, debugString.string, debugString.x, debugString.y);
		glyph.createPath(textPath);
		gfx.setColour(debugString.strokeColor); gfx.strokePath(textPath, debugString.strokeType);
		gfx.setColour(debugString.color); gfx.fillPath(textPath);
	}
}

void OpenGLCanvas::mouseDoubleClick(const MouseEvent & event)
{
	if (event.mods.isAnyModifierKeyDown())
		return;

	hideGUI = !hideGUI;

	Component* parent = getParentComponent();
	if (parent != nullptr)
	{
		parent->resized();
		//parent->setOpaque(true);
		//if (hideGUI)
		//	parent->addToDesktop(ComponentPeer::StyleFlags::windowHasMaximiseButton);
		//Desktop::getInstance().setKioskModeComponent(hideGUI ? parent : nullptr);
	}
}

void OpenGLCanvas::mouseWheelMove(const MouseEvent & e, const MouseWheelDetails & wheel)
{
	mouseWheelWasMoved = true;

	double scale = 0.1;
	if (e.mods.isShiftDown())
		scale = 0.01;

	mouseWheelDelta = Zoom->getValue();

	mouseWheelDelta += wheel.deltaY > 0 ? scale : -scale;

	Zoom->setValue(mouseWheelDelta, true, true);
}

void OpenGLCanvas::mouseDrag(const MouseEvent & e)
{
	isMouseInUse = true;

	if (e.mods.isCommandDown())
		return;

	mouseX = (double)e.getPosition().getX();
	mouseY = (double)e.getPosition().getY();

	int newDragDistanceX = e.getDistanceFromDragStartX();
	int newDragDistanceY = e.getDistanceFromDragStartY();

	int dragDeltaX = newDragDistanceX - oldDragDistanceX;
	int dragDeltaY = newDragDistanceY - oldDragDistanceY;

	oldDragDistanceX = newDragDistanceX;
	oldDragDistanceY = newDragDistanceY;

	float scale = 2.0 / fMinSize;
	if (e.mods.isShiftDown()) // SHIFT+DRAG
		scale = 0.2 / fMinSize;

	dragValueX += scale*dragDeltaX;
	dragValueY += scale*dragDeltaY;

	if (e.mods.isLeftButtonDown()) // LEFT DRAG
	{
		ShiftX->setValue(+dragValueX + init_ShiftX, true, true);
		ShiftY->setValue(-dragValueY + init_ShiftY, true, true);
	}
	else if (e.mods.isRightButtonDown() && OneDimensional->getValue()) // RIGHT DRAG
	{
		OneDimOffset->setValue(dragValueX + init_1DOffset, true, true);
		OneDimZoom->setValue(dragValueY + init_1DZoom, true, true);
	}
}

void OpenGLCanvas::mouseUp(const MouseEvent & e)
{
	isMouseInUse = false;
	oldDragDistanceX = oldDragDistanceY = dragValueX = dragValueY = 0;
}

void OpenGLCanvas::mouseDown(const MouseEvent & e)
{
	isMouseInUse = true;

	mouseX = (double)e.getPosition().getX();
	mouseY = (double)e.getPosition().getY();

	if (e.mods.isLeftButtonDown()) // LEFT-CLICK
	{
		if (e.mods.isCommandDown()) // CTRL+LEFT-CLICK
		{
			mouseWheelY = 1;
			ShiftX->resetToDefaultValue(true, true);
			ShiftY->resetToDefaultValue(true, true);
			ScaleX->resetToDefaultValue(true, true);
			ScaleY->resetToDefaultValue(true, true);
			Zoom->resetToDefaultValue(true, true);
		}
		else if (e.mods.isAltDown()) // ALT+LEFT-CLICK
		{
			if (fMaxSize == fWidth)
			{
				ShiftX->setValue(+mouseX / fWidth * (fRatio*2.0) - fRatio, true, true);
				ShiftY->setValue(-mouseY / fHeight * 2.0 + 1.0, true, true);
			}
			else
			{
				ShiftX->setValue(+mouseX / fWidth * 2.0 - 1.0, true, true);
				ShiftY->setValue(-mouseY / fHeight * (fRatio*2.0) + fRatio, true, true);
			}
		}
	}
	else if (e.mods.isRightButtonDown() && OneDimensional->getValue()) // RIGHT-CLICK
	{
		if (e.mods.isCommandDown()) // CTRL+RIGHT-CLICK
		{
			OneDimOffset->resetToDefaultValue(true, true);
			OneDimZoom->resetToDefaultValue(true, true);
		}
		else if (e.mods.isAltDown()) // ALT+RIGHT-CLICK
		{
			OneDimOffset->setValue(+mouseX / fWidth*2.0 - 1.0, true, true);
			OneDimZoom->setValue(-mouseY / fHeight*2.0 + 1.0, true, true);
		}
	}

	init_ShiftX = ShiftX->getValue();
	init_ShiftY = ShiftY->getValue();
	init_1DOffset = OneDimOffset->getValue();
	init_1DZoom = OneDimZoom->getValue();
}

void OpenGLCanvas::addDataFromBuffersAndRenderBrushes()
{
	for (auto & b : *brushes)
		b->lengthFade = TailFadeFactor->getValue() * audioBuffer->getNumPointsToRender();

	if (OneDimensional->getValue())
	{
		float scaleX = jmax<float>(1, fWidth / fHeight);
		float zoom   = OneDimZoom->getValue() * scaleX;

		openglOscilloscope.set1DTransformation(
			OneDimZoom->getValue() * scaleX,
			OneDimOffset->getValue() * (zoom + 1) * scaleX,
			jmax<float>(1, fHeight / fWidth)
		);
	}
	else
	{
		openglOscilloscope.set1DTransformation(1, 0, 1);
	}

	audioBuffer->updatePointsToRender();

	if (!doFreezePoints)
		openglOscilloscope.setXYDataPoints(*audioBuffer, (*audioBuffer).bufX, (*audioBuffer).bufY);

	openglOscilloscope.render(this, { (*brushes)[0], (*brushes)[1] });

	if (OneDimensional->getValue())
	{
		if (!doFreezePoints)
			openglOscilloscope.setXYDataPoints(*audioBuffer, (*audioBuffer).bufX, (*audioBuffer).bufY2);

		openglOscilloscope.render(this, { (*brushes)[2], (*brushes)[3] });
	}
}

void OpenGLCanvas::handleScreenshot()
{
	if (isSavingScreenshot == true)
	{
		SaveScreenShot(screenshotPath, Time::getCurrentTime().formatted("PrettyScope-%m-%d-%Y-%H_%M_%S.png"));
		isSavingScreenshot = false;
	}
}

void OpenGLCanvas::handleTextureResizeAndClear()
{
	if (isResizing == true)
		updateTextureDimensions();

	if (prettyScopeModule->doClearCanvas && !prettyScopeModule->canvasWasCleared)
	{
		prettyScopeModule->canvasWasCleared = true;
		clearCanvas();
	}

	if (userClickedClearCanvas)
	{
		userClickedClearCanvas = false;
		clearCanvas();
	}

	isResizing = false;
}

void OpenGLCanvas::handleSwapIntervalChanged()
{
	if (openglOscilloscope.swapIntervalChanged)
	{
		openGLContext.setSwapInterval(openglOscilloscope.framerate);
		openglOscilloscope.swapIntervalChanged = false;
	}
}

void OpenGLCanvas::render()
{
	ScopedLock lock(openGLCriticalSection); // prevent changes to size during rendering

	if (failedToStart || prettyScopeModule == nullptr)
		return;

	jassert(OpenGLHelpers::isContextActive()); WriteDebugLog(0);

	ClearDebugStrings();

	// call AddDebugString here any number of times to add debug text to the screen
	prettyScopeModule->frameRateSmoother.setTargetValue(frameRate);

	if (doShowFPS) // showing FPS counter significantly lowers framerate
		AddDebugString(String(prettyScopeModule->frameRateSmoother.getCurrentValue(), 2) +" FPS");
	
	handleScreenshot();

	handleTextureResizeAndClear();

	// set up rendering to a texture
	glBindFramebuffer(GL_FRAMEBUFFER, frameBufferID); WriteDebugLog(1);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textureIDs[2], 0); WriteDebugLog(2);

	const auto scale = openGLContext.getRenderingScale();
	glViewport(0, 0, width * scale, height * scale); WriteDebugLog(3);
	//
	// SCENE RENDERING-----------------------------------------------
	//
	openglOscilloscope.clearColor(0, 0, 0, 0);
	
	handleAutoPause();

	//prettyScopeModule->Zoom.smoother.setIsPaused(openglOscilloscope.is_paused);
	//prettyScopeModule->ShiftX.smoother.setIsPaused(openglOscilloscope.is_paused);
	//prettyScopeModule->ShiftY.smoother.setIsPaused(openglOscilloscope.is_paused);
	//prettyScopeModule->ScaleX.smoother.setIsPaused(openglOscilloscope.is_paused);
	//prettyScopeModule->ScaleY.smoother.setIsPaused(openglOscilloscope.is_paused);
	//prettyScopeModule->ShearX.smoother.setIsPaused(openglOscilloscope.is_paused);
	//prettyScopeModule->ShearY.smoother.setIsPaused(openglOscilloscope.is_paused);
	//prettyScopeModule->Rotation.smoother.setIsPaused(openglOscilloscope.is_paused);

	addDataFromBuffersAndRenderBrushes();

	//
	// combine old screen buffer with new screen buffer -- source code in Shaders\Combine.frag
	//
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textureIDs[currentTexture], 0); WriteDebugLog(4);
	const GLuint textures[] ={ textureIDs[(currentTexture + 1) & 1], textureIDs[2] }; WriteDebugLog(5);
	const GLenum targets[] ={ GL_TEXTURE_2D, GL_TEXTURE_2D };
	bind_textures(targets, textures);
	WriteDebugLog(6);

	const float decaySpeed = openglOscilloscope.is_paused ? 0.0f : openglOscilloscope.decaySpeed/50.0f;

	combineShader->use(); WriteDebugLog(7);
	combineShader->setUniform("oldPixel", 0); WriteDebugLog(8);
	combineShader->setUniform("newPixel", 1); WriteDebugLog(9);
	combineShader->setUniform("decaySpeed", decaySpeed); WriteDebugLog(10);
	combineShader->setUniform("blendMode", openglOscilloscope.blendMode); WriteDebugLog(11);
	renderFullScreenQuad(combineShader, "position", "texCoord"); WriteDebugLog(12);

	glBindFramebuffer(GL_FRAMEBUFFER, 0); glErrorCheck(); WriteDebugLog(13);
	glClearColor(0, 0, 0, 0); WriteDebugLog(14);
	glClear(GL_COLOR_BUFFER_BIT); WriteDebugLog(15);

	if (doHeatmap)
	{
		const GLuint textures[] ={ textureIDs[currentTexture], openglOscilloscope.colormap_texture_id };
		const GLenum targets[] ={ GL_TEXTURE_2D, GL_TEXTURE_1D };
		bind_textures(targets, textures); WriteDebugLog(16);

		colormapShader->use(); WriteDebugLog(17);
		colormapShader->setUniform("originalColor", 0); WriteDebugLog(18);
		colormapShader->setUniform("colorMap", 1); WriteDebugLog(19);
		colormapShader->setUniform("blendMode", openglOscilloscope.blendMode); WriteDebugLog(20);
		colormapShader->setUniform("colorMapCorrection", openglOscilloscope.getColorMapCorrection()); WriteDebugLog(21);
		renderFullScreenQuad(colormapShader, "position", "texCoord"); WriteDebugLog(22);
	}
	else
	{
		// render resulting combination -- very basic shaders
		glActiveTexture(GL_TEXTURE0); WriteDebugLog(23);
		glBindTexture(GL_TEXTURE_2D, textureIDs[currentTexture]); WriteDebugLog(24);

		basicShader->use(); WriteDebugLog(25);
		basicShader->setUniform("tex", 0); WriteDebugLog(26);
		basicShader->setUniform("blendMode", openglOscilloscope.blendMode); WriteDebugLog(27);
		renderFullScreenQuad(basicShader, "position", "texCoord"); WriteDebugLog(28);
	}

	currentTexture ^= 1; // toggling between 1 and 0

	handleSwapIntervalChanged();

	repaint();

	const float frameTime = Time::highResolutionTicksToSeconds(Time::getHighResolutionTicks() - prevFrameTicks);
	frameRate = 1.0 / frameTime;
	prevFrameTicks = Time::getHighResolutionTicks();
}
