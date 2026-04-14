/*
Deterministic seed-based noise generator.
*/

class NoiseGenerator
{
	NoiseGenerator() {};

	double value = 0;

	void setSeed(int v)
	{
		seed = v;
	}

	void setSeedAndReset(int v)
	{
		setSeed(v);
		reset();
	}

	void reset()
	{
		state = seed;
	}

	double getSampleBipolar()
	{
		//state = (1664525 * state + 1013904223) & 4294967295;
		//return value = 2.0 / 4294967296.0 * state - 1;
		return rand(-1, 1);
	}

	double getSampleUnipolar()
	{
		//state = (1664525 * state + 1013904223) & 4294967295;
		//return value = 1.0 / 4294967296.0 * state;
		return rand(0, 1);
	}

	protected int seed = 39047;
	protected uint64 state = 0;
};
