#pragma once

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