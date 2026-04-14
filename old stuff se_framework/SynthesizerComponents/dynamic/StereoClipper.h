#pragma once

class GainCompensatedSoftClipper
{
public:

	GainCompensatedSoftClipper()
	{
		clipAmountGraph.addNode(0.0, 2.000);
		clipAmountGraph.addNode(0.1, 1.000);
		clipAmountGraph.addNode(1.0, 0.001);

		gainCompensationGraph.addNode(2.000, 1.00);
		gainCompensationGraph.addNode(1.000, 1.00);
		gainCompensationGraph.addNode(0.001, 1000);
	}

	~GainCompensatedSoftClipper() = default;

	// 0 to 1
	void setClipAmount(double v)
	{
		clipAmount = v;
		updateClipLevel();
	}

	// max output is -1 to +1
	double getSample(double v)
	{
		return std::clamp(clipper.getValue(v) * gainCompensation, -1.0, +1.0);
	}

protected:

	void updateClipLevel()
	{
		clipLevel = clipAmountGraph.getValue(clipAmount);
		gainCompensation = std::max(1.0, 1.0 / clipLevel);
		clipper.setWidth(clipLevel * 2);
	}

	RAPT::rsScaledAndShiftedSigmoid<double> clipper;
	RAPT::rsNodeBasedFunction<double> clipAmountGraph;
	RAPT::rsNodeBasedFunction<double> gainCompensationGraph;

	double clipAmount = 1.0;
	double clipLevel = 1.5;
	double gainCompensation = 1.0;
};


class Clipper
{
public:

	enum ClipMode { Hard, Soft };

	Clipper()
	{
		updateClipLevel();
	}

	~Clipper() = default;	

	void setClipMode(int v);

	void setClipLevel(double v);	

	double getSample(double v);

protected:

	void updateClipLevel();

	RAPT::rsScaledAndShiftedSigmoid<double> clipper;
	ClipMode clipMode = ClipMode::Hard;
	double clipLevel = 1.0;
};


class StereoClipper
{
public:

	enum ClipMode { Hard, Soft };

	StereoClipper();

	void setClipMode(int v);

	void setClipLevel(double v);	

	void getSample(double * left, double * right);

protected:

	void updateClipLevel();

	std::array<RAPT::rsScaledAndShiftedSigmoid<double>, 2> softClipper;
	ClipMode clipMode = ClipMode::Hard;
	double clipLevel = 1.0;
};

