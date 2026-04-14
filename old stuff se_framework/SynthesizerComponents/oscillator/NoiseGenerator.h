#pragma once

namespace elan
{
class NoiseGenerator
{
public:

	NoiseGenerator() = default;
	~NoiseGenerator() = default;

	void setSeed(int v);

	void setSeedAndReset(int v)
	{
		setSeed(v);
		reset();
	}

	void reset();

	double getSampleBipolar();
	double getSampleUnpiolar();

protected:

	int seed = 39047;
	unsigned long state = 0;
};

class RandomGeneratorClocked
{

public:
	RandomGeneratorClocked()
	{
		clockdivider.triggerFunction = [this]()
		{
			noiseValue = noise.getSampleUnpiolar();
			return;
		};
	}
	~RandomGeneratorClocked() = default;

	void setSeed(int v)
	{
		noise.setSeed(v);
	}

	void reset()
	{
		noise.reset();
		clockdivider.reset();
	}

	void trigger()
	{
		clockdivider.trigger();
	}

	double getSample()
	{
		return noiseValue;
	}

	void setClockDivision(int v)
	{
		clockdivider.setDivision(v);
	}

protected:

	double noiseValue = 0;

	elan::NoiseGenerator noise;
	ClockDivider clockdivider;
};

class RandomGeneratorTimed
{
public:

	RandomGeneratorTimed()
	{
		timer.triggerFunction = [this]() { sampledNoiseValue = noiseValue; };
	}

	~RandomGeneratorTimed() = default;

	void setSampleRate(double v)
	{
		timer.setSampleRate(v);
		smoother.setSampleRate(v);
	}
	
	void reset()
	{
		noise.reset();
		timer.reset();
		smoother.setInternalValue(sampledNoiseValue);
	}

	void triggerNewSample()
	{
		timer.reset();		
	}

	void increment()
	{
		timer.incrememtIfRunning();
		noiseValue = noise.getSampleBipolar();
		smoothedNoiseValue = smoother.getSample(sampledNoiseValue);
	}

	double getNoiseValue() { return noiseValue; }
	double getSampledNoiseValue() { return sampledNoiseValue; }
	double getSmoothedNoiseValue() { return smoothedNoiseValue; }
	double getSmoothedNoiseValueUnipolar() { return bipolarToUnipolar(smoothedNoiseValue); }

	void setSeed(int v)
	{
		noise.setSeed(v);
	}

	void setFrequency(double v)
	{
		setSmoothingFrequency(v);
		setSamplingFrequency(v);
	}

	void setSmoothingFrequency(double v)
	{
		smoother.setSmoothingTime(1.0 / v);
	}

	void setSamplingFrequency(double v)
	{
		timer.setFrequency(v);
	}

protected:
	double noiseValue = 0;
	double smoothedNoiseValue = 0;
	double sampledNoiseValue = 0;

	elan::NoiseGenerator noise;
	LinearSmoother smoother;
	PreciseTimer timer;
};


	/** Implements a discrete time version of the Lorentz system defined by the system of
	differential equations: dx/dt = sigma*(y-x), dy/dt = x*(rho-z) - y, dz/dt = x*y - beta*z where
	sigma, rho, beta are the parameters of the system



	\todo use LaTeX markup in the comment
	\todo write more sophisticated state iteration functions based on runge/kutta or something

	*/

	class LorentzSystem
	{

	public:

		/** \name Construction/Destruction */

		/** Constructor. */
		LorentzSystem();

		/** \name Setup */

		/** Sets the sample-rate. */
		void setSampleRate(double newSampleRate);

		/** Sets up a kind of pseudo-frequency that determines how fast the system will run. */
		void setPseudoFrequency(double newPseudoFrequency);

		/** Sets the internal state of the system consisting of the 3 state variables x, y, z. */
		void setState(double x, double y, double z)
		{
			this->x = x;
			this->y = y;
			this->z = z;
		}

		void setSigma(double sigma)
		{
			this->sigma = sigma;
		}

		void setRho(double rho)
		{
			this->rho = rho;
		}

		void setBeta(double beta)
		{
			this->beta = beta;
		}

		/** \name Inquiry */

		void getState(double *x, double *y, double *z)
		{
			*x = this->x;
			*y = this->y;
			*z = this->z;
		}

		/** \name Processing */

		/** Iterates the internal state one step forward. */
		void iterateState()
		{
			double dx = sigma * (y - x);
			double dy = x * (rho - z) - y;
			double dz = x * y - beta * z;
			x += h * dx;
			y += h * dy;
			z += h * dz;
		}

		void reset()
		{
			x = 0.5;
			y = 0.0;
			z = 0.0;
		}


	protected:

		void updateStepSize()
		{
			const double c = 1.0;
			// \todo tweak this such that pseudoFrequency indeed coincides with perceived frequency

			h = c * pseudoFrequency / sampleRate;
		}

		double x, y, z;                        // state variables
		double sigma, rho, beta, h;            // internal parameters
		double sampleRate, pseudoFrequency;    // user parameters

	};

} //namespace elan
