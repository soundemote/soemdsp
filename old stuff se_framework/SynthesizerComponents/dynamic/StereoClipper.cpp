#include "StereoClipper.h"

void Clipper::setClipMode(int v)
{
	clipMode = (ClipMode)v;
	updateClipLevel();
}

void Clipper::setClipLevel(double v)
{
	clipLevel = v;
	updateClipLevel();
}

void Clipper::updateClipLevel()
{
	switch (clipMode)
	{
	case ClipMode::Hard:
		break;
	case ClipMode::Soft:
		clipper.setWidth(clipLevel * 2);
		break;
	}
}

double Clipper::getSample(double v)
{
	switch (clipMode)
	{
	case ClipMode::Hard:
		return std::clamp(v, -clipLevel, +clipLevel);
	case ClipMode::Soft:
		return clipper.getValue(v);
	}
	return {};
}


StereoClipper::StereoClipper()
{
	softClipper[0].setSigmoid(&RAPT::rsNormalizedSigmoids<double>::softClipHexic);
	softClipper[1].setSigmoid(&RAPT::rsNormalizedSigmoids<double>::softClipHexic);
	updateClipLevel();
}

void StereoClipper::setClipMode(int v) 
{ 
	clipMode = (ClipMode)v; 
	updateClipLevel(); 
}

void StereoClipper::setClipLevel(double v) 
{
	clipLevel = v; 
	updateClipLevel();
}

void StereoClipper::updateClipLevel()
{
	switch (clipMode) // REPLACE WTIH GENERAL CLIPPER
	{
	case ClipMode::Hard:
		break;
	case ClipMode::Soft:
		softClipper[0].setWidth(clipLevel*2);
		softClipper[1].setWidth(clipLevel*2);
	}
}

void StereoClipper::getSample(double * left, double * right)
{
	switch (clipMode) // REPLACE WTIH GENERAL CLIPPER
	{
	case ClipMode::Hard:
		*left = std::clamp(*left, -clipLevel, clipLevel);
		*right = std::clamp(*right, -clipLevel, clipLevel);
		break;
	case ClipMode::Soft:
		*left = softClipper[0].getValue(*left);
		*right = softClipper[1].getValue(*right);
		break;
	}
}