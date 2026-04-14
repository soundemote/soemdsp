#pragma once

class SmootherBase
{
public:
	virtual void setSampleRate(double v) { sampleRate = v; }

	double getSample(double v)
	{
		setTargetValue(v);
		return inc();
	}

	double inc()
	{
		isSmoothing = needsSmoothing();

		if (!isSmoothing || isPaused)
			return currentValue;

		currentValue = getSampleInternal();

		if (fabs(currentValue - targetValue) <= 1.e-6)
			setInternalValue(targetValue);

		return currentValue;
	}

	virtual void setSmoothingTime(double v) { smoothingTime = v; }

	virtual void setInternalValue(double v) 
	{
		currentValue = targetValue = v; 
		isSmoothing = false;
	}

	virtual void setTargetValue(double v) { targetValue = v; }

	virtual void setIsPaushed(bool v) { isPaused = v; }

	virtual double getCurrentValue() { return currentValue; }

	virtual bool needsSmoothing() { return std::abs(currentValue - targetValue) > 1.e-6; }

protected:
	virtual double getSampleInternal() { return currentValue; }

	double currentValue = 0;
	double targetValue = 0;
	double sampleRate = 44100;
	double smoothingTime = .012;
	bool isPaused = false;
	bool isSmoothing = true;

	friend class ParamSmoother;
};

class LinearSmoother : public SmootherBase
{
public:

	void setSampleRate(double v) override
	{
		SmootherBase::setSampleRate(v);

		updateNumSamplesToTarget();
		updateIncrement();
	}

	void setSmoothingTime(double v) override
	{
		SmootherBase::setSmoothingTime(v);

		updateNumSamplesToTarget();
		updateIncrement();
	}

	void setTargetValue(double v) override
	{
		SmootherBase::setTargetValue(v);

		isSmoothing = needsSmoothing();

		if (!isSmoothing)
			setInternalValue(targetValue);

		updateIncrement();
	}

	void setMaxIncAmt(double v)
	{
		maxIncAmt = v;
	}

protected:
	void updateIncrement()
	{
		increment = (targetValue - currentValue) / numSamplesToTarget;
	}

	void updateNumSamplesToTarget()
	{
		numSamplesToTarget = int(smoothingTime * sampleRate);
	}

	double getSampleInternal() override
	{
 		return currentValue += clamp(increment, -maxIncAmt, +maxIncAmt);
	}

	double increment = 0.0;
	double maxIncAmt = 1;
	int numSamplesToTarget = 0;
};

class ExponentialSmoother : public SmootherBase
{
public:

	void setSampleRate(double v) override
	{
		SmootherBase::setSampleRate(v);

		smoother.setTimeConstantAndSampleRate(smoothingTime, sampleRate);
	}

	void setSmoothingTime(double v) override
	{
		SmootherBase::setSmoothingTime(v);

		smoother.setTimeConstantAndSampleRate(smoothingTime, sampleRate);
	}

	void setTargetValue(double v) override
	{
		SmootherBase::setTargetValue(v);

		smoother.setTargetValue(targetValue);
	}

	void setInternalValue(double v)
	{
		SmootherBase::setInternalValue(v);

		smoother.setCurrentValue(targetValue);
	}

protected:
	double getSampleInternal() override
	{
		return smoother.getSample();
	}

	rosic::ExponentialSmoother smoother;
};

class GaussianSmoother : public SmootherBase
{
public:

	GaussianSmoother()
	{
		smoother.setOrder(2);
		smoother.setShapeParameter(1);
	}

	void setSampleRate(double v) override
	{
		SmootherBase::setSampleRate(v);

		smoother.setTimeConstantAndSampleRate(smoothingTime, v);
	}

	void setSmoothingTime(double v) override
	{
		SmootherBase::setSmoothingTime(v);

		smoother.setTimeConstantAndSampleRate(smoothingTime, sampleRate);
	}

	void setInternalValue(double v) override
	{
		SmootherBase::setInternalValue(v);

		smoother.setTimeConstantAndSampleRate(0, sampleRate);
		smoother.getSample(targetValue);
		smoother.setTimeConstantAndSampleRate(smoothingTime, sampleRate);
	}

	double getSampleInternal() override
	{
		return smoother.getSample(targetValue);
	}

	RAPT::rsSmoothingFilter<double, double> smoother;
};

class ParamSmoother : public SmootherBase
{
public:
	ParamSmoother(double initialValue = 0.0);

	void setSampleRate(double v) override
	{
		SmootherBase::setSampleRate(v);

		smoother->setSampleRate(v);
	}

	void setSmoothingTime(double v) override
	{
		SmootherBase::setSmoothingTime(v);

		smoother->setSmoothingTime(v);
	}

	void setInternalValue(double v) override
	{
		SmootherBase::setInternalValue(v);

		smoother->setInternalValue(v);
	}

	void setTargetValue(double v) override
	{
		SmootherBase::setTargetValue(v);

		smoother->setTargetValue(v);
	}

	double getSampleInternal() override
	{
		return smoother->getSampleInternal();
	}

	enum type { EXPONENTIAL, GAUSSIAN, LINEAR };

	void setSmootherType(type v) 
	{
		smootherType = v;
		switch (smootherType)
		{
		case type::EXPONENTIAL:
			smoother.reset(new ExponentialSmoother());
			break;
		case type::GAUSSIAN:
			smoother.reset(new GaussianSmoother());
			break;
		case type::LINEAR:
			smoother.reset(new LinearSmoother());
			break;
		}
	}

protected:
	std::unique_ptr<SmootherBase> smoother;

	type smootherType = LINEAR;
};

/* 
* Call cleanupSmoothers() in the per-block function, recommended after the per-sample calls.
* call inc() in the per-sample function.
*/
class SmootherManager
{
public:
	SmootherManager() = default;
	~SmootherManager() = default;

	void setSampleRate(int v);

	void cleanupSmoothers();

	void inc();

	void addSmoother(const String& id, std::function<void()> func, ParamSmoother::type smootherType = ParamSmoother::LINEAR, bool useDefaultSmoothingTime = true);

	void setTargetValue(const String& id, double v);

	void setSmoothingTime(const String& id, double v);

	void setDefaultSmoothingTime(double v);

	double getValue(const String& id);

	bool needsSmoothing();

	void runSetValueFunction(const String& id);

	class SmootherChild : public ParamSmoother
	{
	public:
		SmootherChild()
		{
			setSmootherType(ParamSmoother::type::LINEAR);
			setSmoothingTime(.012);
		}

		double inc()
		{
			ParamSmoother::inc();
			setValueFunction();
			return getCurrentValue();
		}

		std::function<void()> setValueFunction = nullptr;
	};

	std::map<String, std::unique_ptr<SmootherChild>> smoothers;

protected:
	vector<SmootherChild*> smoothersUsingDefaultTime;
	vector<SmootherChild*> activeSmoothers;
	double defaultSmoothingTime = 0.012;
};