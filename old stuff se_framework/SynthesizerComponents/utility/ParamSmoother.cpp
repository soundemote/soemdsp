#include "ParamSmoother.h"

ParamSmoother::ParamSmoother(double initialValue)
{
	setSmootherType(type::LINEAR);
	smoother->setInternalValue(initialValue);
}

void SmootherManager::addSmoother(const String& id, std::function<void()> func, ParamSmoother::type smootherType, bool useDefaultSmoothingTime)
{
	//jassert(id == smoothers.size()); // add the variables in order of id starting from 0;

	smoothers[id] = std::make_unique<SmootherChild>();
	smoothers[id]->setValueFunction = func;
	smoothers[id]->setSmootherType(smootherType);
	smoothers[id]->setSmoothingTime(defaultSmoothingTime);

	if (useDefaultSmoothingTime)
		smoothersUsingDefaultTime.push_back(smoothers[id].get());
}

void SmootherManager::runSetValueFunction(const String& id)
{
	smoothers[id]->setValueFunction();
}

void SmootherManager::setTargetValue(const String& id, double v)
{
	smoothers[id]->setTargetValue(v);

	if (!VectorHelper::contains(activeSmoothers, smoothers[id].get()))
		activeSmoothers.push_back(smoothers[id].get());
}

void SmootherManager::setSmoothingTime(const String& id, double v)
{
	smoothers[id]->setSmoothingTime(v);
}

void SmootherManager::setDefaultSmoothingTime(double v)
{
	if (defaultSmoothingTime == v)
		return;

	defaultSmoothingTime = v;

	for (auto& s : smoothersUsingDefaultTime)
		s->setSmoothingTime(v);
}

double SmootherManager::getValue(const String& id)
{
	return smoothers[id]->getCurrentValue();
}

bool SmootherManager::needsSmoothing()
{
	return activeSmoothers.size() != 0;
}

void SmootherManager::cleanupSmoothers()
{
	activeSmoothers.erase(std::remove_if(activeSmoothers.begin(), activeSmoothers.end(), [](SmootherChild* s) { return !s->needsSmoothing(); }), activeSmoothers.end());
}

void SmootherManager::inc()
{
	for (auto& s : activeSmoothers)
		s->inc();
}

void SmootherManager::setSampleRate(int v)
{
	for (auto& s : smoothers)
		s.second->setSampleRate(v);
}
