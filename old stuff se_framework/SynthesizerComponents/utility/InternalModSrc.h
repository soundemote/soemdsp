#pragma once

class InternalModSrc : public jura::ModulationSource
{
public:
	InternalModSrc(String name, String displayName)
	{
		setModulationSourceName(name);
		setModulationSourceDisplayName(displayName);
		valuePtr = &dummy;
	}

	InternalModSrc(String name, String displayName, double* valuePtr_)
		: valuePtr(valuePtr_)
	{
		setModulationSourceName(name);
		setModulationSourceDisplayName(displayName);
	}

	void setValuePtr(double* ptr)
	{
		valuePtr = ptr;
	}

protected:

	virtual double renderModulation() override
	{
		return *valuePtr;
	}

	double* valuePtr;

	double dummy = 0;
};
