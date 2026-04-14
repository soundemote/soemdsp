#include "ScopeAudioBuffer.h"

ScopeAudioBuffer::ScopeAudioBuffer(CriticalSection * lockToUse) : lock2(lockToUse)
{
	setCircularBufferSize(numPtsMax);
}

void ScopeAudioBuffer::clearCircularBuffers()
{
	std::fill(bufX.begin(), bufX.end(), 0);
	std::fill(bufY.begin(), bufY.end(), 0);
	std::fill(bufY2.begin(), bufY2.end(), 0);
	bufIndex = 0;
	numPointsToRender = 0;
}

void ScopeAudioBuffer::setCircularBufferSize(int newSize)
{
	ScopedLock scopedLock(*lock2);

	bufferSize = newSize;
	bufferMask = bufferSize-1;
	bufX.resize(bufferSize);
	bufY.resize(bufferSize);
	bufY2.resize(bufferSize);

	clearCircularBuffers();
}

int ScopeAudioBuffer::getRenderStart(int numPoints)
{
	ScopedLock scopedLock(*lock2);

	int startIndex = bufIndex - numPoints;
	while (startIndex < 0)
		startIndex += bufferSize;

	numPointsToRender = 0;

	return startIndex;
}

void ScopeAudioBuffer::updateStartIndex()
{
	bufIndex = (bufIndex+1) & bufferMask;
}

void ScopeAudioBuffer::getRenderInfo(int * startIndex, int * numPoints)
{
	ScopedLock scopedLock(*lock2);
	*numPoints  = numPointsToRender;
	*startIndex = getRenderStart(*numPoints);
}

void ScopeAudioBuffer::doXYTransformation(double x, double y, double * out_x, double * out_y)
{
	//double z = x*y*4;
	double z = 0;
	rotator3D.apply(&x, &y, &z);
	*out_x = x * zoom * scaleX + shiftX;
	*out_y = y * zoom * scaleY + shiftY;
}

void ScopeAudioBuffer::setPointsPerFrame(int v) 
{
	pointsPerFrame = v;
}

void ScopeAudioBuffer::setRotationX(double v)
{
	rotator3D.setAngleX(v * TAU);
}
void ScopeAudioBuffer::setRotationY(double v)
{
	rotator3D.setAngleY(v * TAU);
}
void ScopeAudioBuffer::setRotationZ(double v)
{
	rotator3D.setAngleZ(v * TAU);
}

void ScopeAudioBuffer::setZoom(double v)
{
	zoom = v;
}

void ScopeAudioBuffer::setScaleX(double v)
{
	scaleX = v;
}

void ScopeAudioBuffer::setScaleY(double v)
{
	scaleY = v;
}

void ScopeAudioBuffer::setShiftX(double v)
{
	shiftX = v;
}

void ScopeAudioBuffer::setShiftY(double v)
{
	shiftY = v;
}

int ScopeAudioBuffer::getNumPointsToRender() const
{
	return numPoints;
}

int ScopeAudioBuffer::getPointStartToRender() const
{
	return start;
}

void ScopeAudioBuffer::updatePointsToRender()
{
	numPoints = pointsPerFrame;
	start = 0;

	// This would always render the samples that have been stored in the circular buffer since last render...
	if (numPoints >= 1)
		start = getRenderStart(numPoints);
	// ...alternatively, we may render a fixed number of samples:		
	else
		getRenderInfo(&start, &numPoints);
}

void ScopeAudioBuffer::processBlock(double **inOutBuffer, int numChannels, int numSamples)
{
	numPointsToRender += numSamples;

	if (numPointsToRender > bufferSize)
		numPointsToRender = bufferSize;
}

void ScopeAudioBuffer::processSampleFrame(double * x, double * y)
{
	isInputSilence = *x+*y == 0.0;

	doXYTransformation(*x, *y, x, y);
}
