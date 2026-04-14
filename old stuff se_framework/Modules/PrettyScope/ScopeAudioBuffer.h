#pragma once

#pragma warning(disable: 4267) // size_t to GLsizei
#pragma warning(disable: 4244) // size_t to to int

using namespace juce;

class ScopeAudioBuffer
{
public:

	ScopeAudioBuffer(CriticalSection * lockToUse);

	virtual ~ScopeAudioBuffer() = default;

	/* Must be called per frame!!! */
	void updatePointsToRender();

	/** Returns the start-index in our circular buffer and the number of points to render and then
	resets internal counter variables for the next call. This is to be called periodically from
	the graphics renderer. */
	void getRenderInfo(int * startIndex, int * numPoints);

	/** If you want to render the most recent "numPoints" samples, this functions returns the index
	in the circular buffer, where you should start reading samples. This can be used alternativly to
	getRenderInfo (the latter doesn't assume a constant number of points to be rendered but instead
	assumes that you want to render all samples that have been stored since the last render). */
	int getRenderStart(int numPoints);

	/** This metho must be called in an audio processBlock thread */
	virtual void processBlock(double **inOutBuffer, int numChannels, int numSamples);

	/** This method must be called in an audio per-sample thread. */
	virtual void processSampleFrame(double * x, double * y);

	void clearCircularBuffers();

	// newSize must be a power of 2
	void setCircularBufferSize(int newSize);

	// circular buffers:
	std::vector<float> bufX, bufY, bufY2;

	void setPointsPerFrame(int v);
	int getNumPointsToRender() const;
	int getPointStartToRender() const;
	void updateStartIndex();

	unsigned int bufferSize; // must be power of 2
	unsigned int bufferMask; // must be one less than bufferSize
	unsigned int bufIndex;
	unsigned int numPointsToRender;

	int numPtsMax = 8192;

	bool getIsInputSilence() { return isInputSilence; }

	// Transformation
	void setZoom(double v);
	void setRotationX(double v);
	void setRotationY(double v);
	void setRotationZ(double v);
	void setScaleX(double v);
	void setScaleY(double v);
	void setShiftX(double v);
	void setShiftY(double v);

	void doXYTransformation(double x, double y, double * out_x, double * out_y);

	// Do X Y Z transformation
	RAPT::rsRotationXYZ<double> rotator3D;

	CriticalSection * lock2 = nullptr;

protected:

	bool isInputSilence = true;

	int
		numPoints = 0,
		start = 0,
		pointsPerFrame = 2048;

	double
		zoom = 1,
		scaleX = 1,
		scaleY = 1,
		shiftX = 0,
		shiftY = 0;
};
