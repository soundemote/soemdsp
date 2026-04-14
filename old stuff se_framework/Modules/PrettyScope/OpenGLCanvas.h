#pragma once

#pragma warning(disable: 4267) // size_t to GLsizei
#pragma warning(disable: 4244) // size_t to to int

using namespace juce;

using std::vector;
using std::map;
using std::function;

void bind_textures(const GLenum(&targets)[2], const GLuint(&textures)[2]);

class JUCE_API OpenGLCanvas
	: public MainContentComponent
	, public jura::ParameterObserver
	, public ChangeListener
{

public:

	/** Constructor. You need to pass a pointer to the scope object, so we can use it to pull
	datapoints from it. */
	OpenGLCanvas(PrettyScopeModule* scopeToUse, ScopeAudioBuffer* audioBufferToUse, vector<Brush*>* ArrayOfBrushesToUse);

	virtual ~OpenGLCanvas();

	/* Parameter Handling */
	map<jura::Parameter *, function<void(float)>> paramlookup;
	void initializeParamLookup();
	bool initializeParamLookup_was_called = false;
	void connectParameters();
	void disconnectParameters();
	virtual void parameterChanged(jura::Parameter* p) override;

	/* convenient pointers to some parameters */
	jura::Parameter* AutoPauseMode;
	jura::Parameter* ShiftX;
	jura::Parameter* ShiftY;
	jura::Parameter* ScaleX;
	jura::Parameter* ScaleY;
	jura::Parameter* OneDimensional;
	jura::Parameter* OneDimOffset;
	jura::Parameter* OneDimZoom;
	jura::Parameter* Zoom;
	jura::Parameter* TailFadeFactor;

	///** Overrides the callback inherited from StateWatcher. */
	//virtual void stateDirtyFlagChanged(StateManager* stateManager) override;

	/** Gets called when the ColorMap was changed. Inside the callback, we update the colormap used
	by the garphics code. */
	virtual void changeListenerCallback(ChangeBroadcaster* source) override;

	/** Overrides inherited render method from MainContentComponent to pull the data from the scope and render it. */
	virtual void render() override;
	void addDataFromBuffersAndRenderBrushes();

	/** For painting debug FPS text */
	void paint(Graphics& g) override;

	void setSizeAndBounds(int x, int y, int w, int h)
	{
		setBounds(x, y, w, h);
		fWidth = (float)getWidth();
		fHeight = (float)getHeight();
		fMaxSize = std::max<float>(fWidth, fHeight);
		fMinSize = std::min<float>(fWidth, fHeight);
		fRatio = fMaxSize/fMinSize;
	}

	/* Canvas Mouse Handling */
	double
		mouseWheelX = 0,
		mouseWheelY = 0,
		mouseWheelDelta;
	double
		fMaxSize,
		fMinSize,
		fRatio,
		fWidth,
		fHeight;
	double
		init_ShiftX = 0,
		init_ShiftY = 0,
		init_1DOffset = 0,
		init_1DZoom = 0;
	int
		oldDragDistanceX = 0,
		oldDragDistanceY = 0;
	double
		dragValueX = 0,
		dragValueY = 0,
		mouseX = 0,
		mouseY = 0;
	bool
		hideGUI = false,
		mouseWheelWasMoved = false,
		isMouseInUse = false;

	// Function to set 
	virtual void handleAutoPause();
	virtual void handleScreenshot();
	void handleSwapIntervalChanged();
	void handleTextureResizeAndClear();

	virtual void mouseDoubleClick(const MouseEvent & event) override;
	virtual void mouseWheelMove(const MouseEvent & e, const MouseWheelDetails & wheel) override;
	virtual void mouseDrag(const MouseEvent & e) override;
	virtual void mouseUp(const MouseEvent & e) override;
	virtual void mouseDown(const MouseEvent & e) override;

	/* Keeps track of log points that have been written so that a log message is only written once */
	vector<bool> logChecks;

	void setEnableHeatmap(bool v) {	doHeatmap = v;	}
	void setFreezePoints(bool v) { doFreezePoints = v; }
	void setShowFPS(bool v) { doShowFPS = v; }
	void setPause(bool v) { doPause = v; }
	// if < 1, renders a number of points based on framerate
	void setScreenshotPath(String v) { screenshotPath = v; }
	void setShaderPath(String v) { shaderPath = v; }

	/* Stupid handling for callback-like functionality */
	bool isSavingScreenshot = false;
	bool userClickedClearCanvas = false;

	void updateShaders();

protected:

	String screenshotPath;
	String shaderPath;

	bool
		doHeatmap = false,
		doFreezePoints = false,
		doShowFPS = false,
		doPause = false;

	/** Retrieves the current color map from the scope and updates the color-array used in the
	graphics  code accordingly. */
	void updateColorMapFromScope();
	//virtual bool keyPressed(const KeyPress &key, Component *originatingComponent) override;
	void SaveScreenShot(String directory, String filename);

	/* Debug String */
	struct DebugString
	{
		String string;
		float x, y;
		Font font;
		Colour color, strokeColor;
		PathStrokeType strokeType = PathStrokeType(0);
		DebugString() {}
		DebugString(String string, float x, float y, Font font, Colour color, Colour strokeColor, PathStrokeType strokeType) :
			string(string), x(x), y(y), font(font), color(color), strokeColor(strokeColor), strokeType(strokeType)
		{
		}
	};

	vector<DebugString> debugStrings;

	void ClearDebugStrings();

	static const Font fps_font;

	void AddDebugString(
		String string,
		float x = 0,
		float y = 24,
		Font font = Font("Arial", 24, Font::plain),
		Colour color = Colours::white,
		Colour strokeColor = Colours::black,
		PathStrokeType strokeType = PathStrokeType(3, PathStrokeType::curved, PathStrokeType::rounded)
	);

	void RenderDebugText(Graphics& gfx);

	int64 prevFrameTicks = Time::getHighResolutionTicks();
	double frameRate = 0;

	ScopeAudioBuffer * audioBuffer = nullptr;
	PrettyScopeModule * prettyScopeModule = nullptr;
	vector<Brush*> * brushes = nullptr;
};
