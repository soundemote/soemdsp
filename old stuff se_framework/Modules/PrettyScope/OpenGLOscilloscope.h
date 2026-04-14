#pragma once

using std::vector;

class OpenGLCanvas;

class OpenGLOscilloscope
{
public:
	class TestData;

	OpenGLOscilloscope(OpenGLContext & context);

	/* Set zoom, offset, and scale for rendering 1D view
		@zoom: waveform horizontal zoom
		@offset: waveform horizontal scroll position
		@scale: waveform vertical zoom / height
	This is applied for all data points, and can be changed before each time data points are given if needed. */
	void set1DTransformation(double zoom, double offset, double scale);

	// Before calling prepareForRender, this function sets audio buffer and data points to be drawn
	void setXYDataPoints(const ScopeAudioBuffer & audioBuffer, const vector<float> & data_x, const vector<float> & data_y);

	// Call this as many times as desired per frame
	void render(OpenGLCanvas* GLCanvas, const vector<Brush*> & brushesToRender);

	// Setup colormap
	void setColorMap(const vector<RGBA> & cm);

	// Needed to correct colormap texture for proper color interpolation
	float getColorMapCorrection() const { return mColorMapCorrection; }

	/* At the moment, framerate is only 1 or 0, true or false, 0 to turn off VSync
	 or 1 to turn on VSync. If VSync is on, refresh rate is synced to monitor refresh
	 rat. */
	void setFramerate(double v);

	void startup();

	void resize(int w, int h);

	void setBlendMode(int newMode);
	void clearColor(float r, float g, float b, float a);

	void resetLine1D_Data();

	void updateShaders(String folderPathToShaders);

	vector<bool> logChecks;

	// Swap interval change needs to be handled
	bool swapIntervalChanged = false;

	// parameters that need quick access from OpenLGCanvas
	int blendMode = 0;
	float decaySpeed = 5.f; // 0 is no pixel decay
	GLuint colormap_texture_id = 0;
	int framerate = 0;
	bool is_paused = false;
	float LenFactor = 0.0f;

private:
	void setSize(int w, int h);

	// screen size stuff
	float fWidth, fHeight, fMaxSize, fMinSize, xScale, yScale, fRatio;

	// TODO Use SOA (structure of arrays) rather than AOS (array of structures)
	// Better for performance + allow plotting debug data
	struct VertexData
	{
		float vertices2p[2];
		int index;
	};
	
	bool applyAlphaBlend = false;	

	// This is called in the render function before rendering
	void prepareForRender(OpenGLCanvas* GLCanvas);

	vector<VertexData> verticesList;

	//ScopedPointer<TestData> testData;
	// There's nothing to stop you recreating the shader on the fly when doing development.
	void createAndLinkShader();

	// called by render conditionally
	void renderBrush(const Brush & p);
	// called by rener conditionally
	void renderBrushesAlphaBlend(OpenGLCanvas* GLCanvas, const vector<Brush*> & brushesToRender);
	// called by renderBrush and equivalent functions conditionally
	void drawLines(const Brush & p);
	// called by renderBrush and equivalent functions conditionally
	void drawDots(const Brush & p);

	GLint aIdxLocation_line = -1;
	GLint aStartLocation_line;
	GLint aEndLocation_line;

	GLint aIdxLocation_dot = -1;
	GLint aStartLocation_dot;

	/** Sets buffer incrementalIndex to be 0,1,2,3,4....  */
	void copyVertexDataToGPU();

	GLuint vertexBufferID = -1;

	size_t numSamples{ 0 };
	size_t numPoints{ 0 };

	double
		zoom_1d = 1,
		offset_1d = 0,
		scale_1d = 1;

	int width = 0;
	int height = 0;

	// Used to correct openGL texture blending so that colormap texture is interpolated as expected
	float mColorMapCorrection = 1.0f;
	// Color map stored, upload on render if dirty
	vector<RGBA> mColormapColors{ vector<RGBA>(1024, {1.0,1.0,1.0,1.0}) };
	// When colormap is set, this will be called to handle the update, but the colormap still needs to be uploaded
	void updateColorMap();
	// Upload will happen in the render callback
	void uploadColorMap();
	// Color map needs to be uploaded to GL when true
	bool mColorMapDirty = true;

	struct Point2D { float x, y; };

	Point2D mLastBlockVertex ={ 1e6f, 0.0f };

	const int GL_RGBA_FORMAT = GL_RGBA32F;

	ScopedPointer<OpenGLShaderProgram> lineShader, dotShader, linePlusDotShader;
	OpenGLContext & context;

	friend class MainContentComponent;
};

class MainContentComponent : public OpenGLAppComponent
{
public:
	MainContentComponent();

	~MainContentComponent() override
	{
		shutdownOpenGL();
	}

	void renderFullScreenQuad(OpenGLShaderProgram* shaderProgram, const char* positionName, const char* texCoordName);

	void updateTextureDimensions();
	void clearCanvas();

	void initialise() final;

	void shutdown() final {}

	// Gets called every frame
	void render() override {}

	void paint(Graphics& g) override {}

	void resized() final;

	vector<bool> logChecks;
	int width = 600;	// holds initial and resized width and helps set width for other components
	int height = 600; // holds initial and resized height and helps set height for other components

	GLuint frameBufferID = -1;
	GLuint fullScreenQuadBufferID = -1;

	static const unsigned TEXTURE_COUNT = 4;
	GLuint textureIDs[TEXTURE_COUNT];

	ScopedPointer<OpenGLShaderProgram> basicShader, combineShader, colormapShader;
	unsigned currentTexture = 0; // switches between 0 and 1 for the texture being painted to

	CriticalSection openGLCriticalSection;
	bool failedToStart = false;

	static void debugCallback(GLenum source,
		GLenum type,
		GLuint id,
		GLenum severity,
		GLsizei length,
		const char *message,
		void *userParam)
	{
		jassertfalse;
	}

protected:
	bool isResizing = false;

	OpenGLOscilloscope openglOscilloscope;

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainContentComponent)
};
