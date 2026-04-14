#pragma once

#include "JuceHeader.h"

using namespace elan;

class TriSawOscillator
{
public:
	TriSawOscillator() = default;
	virtual ~TriSawOscillator() = default;

	Phasor phasor;

	void setSampleRate(double v)
	{
		phasor.setSampleRate(v);
		recalculateAsymLimit();
	}
	void reset() { phasor.reset(); }

	double getSample()
	{
		double phase = phasor.getSample();

		triSawValue = [&]()
		{
			if (phase < h)
				return RAPT::rsTriSawOscillator<double>::shape(a0 + a1 * phase, atkbend, -0.5 * atksig); // upward section
			else
				return RAPT::rsTriSawOscillator<double>::shape(b0 + b1 * phase, -decbend, -0.5 * decsig);  // downward section
		}();

		return triSawValue;
	}

	void setFrequency(double v)
	{
		phasor.setFrequency(v);
		recalculateAsymLimit();
	}
	void setPhaseOffset(double v) { phasor.setPhaseOffset(v); }
	void setAsymmetrical(double v)
	{
		incomingAsym = v;
		recalculateAsym();
	}

	void setAttackBend(double v) { atkbend = v; }
	void setDecayBend(double v) { decbend = v; }
	void setAttackSigmoid(double v)
	{
		if (v < 0)
			v *= 2;
		atksig = v;
	}
	void setDecaySigmoid(double v)
	{
		if (v < 0)
			v *= 2;
		decsig = v;
	}

	void setBendSymmetrical(double v)
	{
		bendSymmetrical = v;
		recalculateBend();
	}
	void setBendOffset(double v)
	{
		bendOffset = v;
		recalculateBend();
	}

	void setSigmoidSymmetrical(double v)
	{
		sigmoidSymmetrical = v;
		recalculateSigmoid();
	}
	void setSigmoidOffset(double v)
	{
		sigmoidOffset = v;
		recalculateSigmoid();
	}

protected:

	void recalculateAsymLimit()
	{
		double inc = phasor.getIncrementAmount();
		asymLimit = std::max<double>(0.0, 1.0 - 2.0 * asymTransitionSamplesMin * inc);
		recalculateAsym();
	}
	void recalculateAsym()
	{
		limitedAsym = jlimit(incomingAsym, -asymLimit, +asymLimit);
		updateCoefficients();
	}

	void recalculateBend()
	{
		double absoluteValue = bendOffset;
		double target = 1;

		if (bendOffset < 0)
		{
			absoluteValue = -bendOffset;
			target = -target;
		}

		setAttackBend(juce::jmap(absoluteValue, bendSymmetrical, target));
		setDecayBend(juce::jmap(absoluteValue, -bendSymmetrical, target));
	}
	void recalculateSigmoid()
	{
		double absoluteValue = sigmoidOffset;
		double target = 1;

		if (sigmoidOffset < 0)
		{
			absoluteValue = -sigmoidOffset;
			target = -target;
		}

		setAttackSigmoid(juce::jmap(absoluteValue, sigmoidSymmetrical, target));
		setDecaySigmoid(juce::jmap(absoluteValue, sigmoidSymmetrical, -target));
	}

	const double asymTransitionSamplesMin = 1;

	double triSawValue = 0;

	// variables for asymmetrical shape control
	double atkbend = 0;
	double decbend = 0;
	double atksig = 0;
	double decsig = 0;

	// variables for symmetrical shape control
	double bendSymmetrical = 0;
	double bendOffset = 0;
	double sigmoidSymmetrical = 0;
	double sigmoidOffset = 0;

	double incomingAsym{};
	double limitedAsym{};
	double asymLimit{};

private:

	double h{};
	double a0{};
	double a1{};
	double b0{};
	double b1{};

	void updateCoefficients()
	{
		h = 0.5 * (limitedAsym + 1);
		a0 = -1;
		a1 = 2 / h;
		b0 = (1 + h) / (1 - h);
		b1 = -1 - b0;
	}
};

class XoxosOscillator
{
public:
	void setFrequency(double v)
	{
		frequency = v;
		calculateRotationSpeed();
	}

	void setSampleRate(double v)
	{
		sampleRate = v;
		calculateRotationSpeed();
	}

	void calculateRotationSpeed()
	{
		double rotationSpeed = (frequency * 4) / sampleRate * PI_z_2 + PI_z_2;
		elan::sinCos(rotationSpeed, &w0, &w1);
	}

	// 0 to 1
	void setAngle(double v) { elan::sinCos(v * PI, &m0, &m1); }

	// 0 to inf
	void setScale(double v) { scale = v; }

	// -1 to +1
	void setOffset(double v) { offset = v; }

	double getSample();

	void reset() {
		s0 = 0;
		s1 = 1;
		w0 = 0;
		w1 = 1;
		m0 = 0;
		m1 = 1;
	}

protected:
	double 
		s0{  },
		s1{ 1 },
		w0{  },
		w1{ 1 },
		m0{  },
		m1{ 1 };

	double
		sampleRate = 44100,
		frequency = 1,
		scale = 0,
		offset = 1;
};

class IanFritzOneEyeChaotica
{
public:
	void setSampleRate(double v)
	{
		sampleRate = v;

		calculateStepSize();

		x_filter.setSampleRate(v);
		y_filter.setSampleRate(v);
		z_filter.setSampleRate(v);

		x_filter.setMode(RAPT::rsOnePoleFilter<double, double>::modes::LOWPASS_BLT);
		y_filter.setMode(RAPT::rsOnePoleFilter<double, double>::modes::LOWPASS_BLT);
		z_filter.setMode(RAPT::rsOnePoleFilter<double, double>::modes::LOWPASS_BLT);
	}

	void calculateStepSize()
	{
		h = stepSize / sampleRate;
	}

	void getSample(double* outL, double* outR)
	{
		double w, x = x1, y = y1, z = z1;

		double dx, dy, dz;
		dx = a * (y - d);
		dy = a * (z - A * m(y));
		w = c * (x + e * n(y));
		dz = -a * (b * z + w);

		/* probably best to put individual speed on each output */
		x1 += h * jlimit(dx, -10.0, +10.0);
		y1 += h * clamp(dy, -10.0, +10.0);
		z1 += h * clamp(dz, -10.0, +10.0);

		//x1 = x_filter.getSample(x1);
		//y1 = y_filter.getSample(y1);
		//z1 = z_filter.getSample(z1);

		rotate3D(x1, y1, z1, xRotSin, xRotCos, yRotSin, yRotCos, outL, outR);
	}

	void setStepSize(double v) { stepSize = v; calculateStepSize(); }
	void setRate(double v) { a = v;/*/sampleRate; */ }
	void setGain(double v) { c = v; }
	void setNLDrive(double v) { e = v; }
	void set2D(double v) { A = v; }
	void setDamping(double v) { b = v; }
	void setOffset(double v) { d = v; }
	void setQ(double v) { q = v; }
	void setXRotation(double v) { elan::sinCos(v * TAU, &xRotSin, &xRotCos); }
	void setYRotation(double v) { elan::sinCos(v * TAU, &yRotSin, &yRotCos); }

	RAPT::rsOnePoleFilter<double, double>x_filter, y_filter, z_filter;

protected:
	double
		stepSize = 0.1,
		sampleRate = 44100;

	double m(double y)
	{
		if (y > 2 * q)
			return y - 2 * q;

		return 0;
	}

	double n(double y)
	{
		if (y < -q)
			return y + q;

		return 0;
	}

	// rotation
	double
		xRotSin = 0,
		xRotCos = 1,
		yRotSin = 0,
		yRotCos = 1;

	// input
	double
		a = 0,
		c = 0,
		e = 0,
		A = 0,
		b = 0,
		d = 0,
		q = 0;

	// internal
	double
		h = 0.01;

	// output
	double
		x1 = 0,
		y1 = 0,
		z1 = 0;
};
