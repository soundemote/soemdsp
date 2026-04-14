#include "myparams.h"

#include "JuceHeader.h"

#undef RELATIVE
#undef ABSOLUTE

void myparams::init(int type, String strID, double mini, double maxi, double def, int scaling)
{
	jassert(mini != maxi);
	jassert(def <= maxi && def >= mini);
	this->type = type;
	this->strID = convertToAcceptableXMLVariable(strID);
	this->text = strID;
	this->mini = mini;
	this->maxi = maxi;
	this->def = def;
	this->scaling = (jura::Parameter::scalings)scaling;

	managerPtr->addParameters(this);
}

void myparams::initButton(String strID, int def)
{
	shouldBeSmoothed = false;
	init(BUTTON, strID, 0, 1, def, jura::Parameter::scalings::BOOLEAN);
}

void myparams::initClickButton(String strID)
{
	shouldBeSmoothed = false;
	init(CLICKBUTTON, strID, 0, 1, 0, jura::Parameter::scalings::BOOLEAN);
}

void myparams::initSlider(String strID, double mini, double maxi, double def)
{
	// defaults
	modDepthMin = -maxi;
	modDepthMax = +maxi;	

	init(SLIDER, strID, mini, maxi, def, scaling);
	
	initializeParameter();

	jassert(modDepthMin < modDepthMax);
}

void myparams::initKnob(String strID, double mini, double maxi, double def)
{
	// defaults
	modDepthMin = mini;
	modDepthMax = maxi;

	init(KNOB, strID, mini, maxi, def, scaling);

	initializeParameter();

	jassert(modDepthMin < modDepthMax);
}

void myparams::initCombobox(String strID, int def, vector<String> list)
{
	maxi = (double)jmax<size_t>(1, list.size()-1);

	menuItems = list;

	shouldBeSmoothed = false;

	init(Type::COMBOBOX, strID, 0, maxi, def, jura::Parameter::scalings::STRING);
}

void myparams::addMenuItems(const vector<String> & list)
{
	menuItems.insert(menuItems.end(), list.begin(), list.end());
	maxi = double(menuItems.size()-1);
}

void myparams::instantiateParameter()
{
	switch (type)
	{
	default:
	case Type::BUTTON:
		ptr = new jura::ModulatableParameter2(strID, mini, maxi, def, scaling, interval);
		break;
	case Type::COMBOBOX:
		ptr = new jura::ModulatableParameter2(strID, mini, maxi, def, scaling, interval);
		for (auto s : menuItems)
			ptr->addStringValue(s);
		break;
	case Type::KNOB:
	case Type::SLIDER:
		if (mapper != nullptr)
			scaling = jura::Parameter::scalings::CUSTOM;

		ptr = new jura::ModulatableParameter2(strID, mini, maxi, def, scaling, interval);
		ptr->setSmoothingTime(0);

		if (mapper != nullptr)
			ptr->setMapper(mapper);

		ptr->setDefaultModParameters(modLimitMin, modLimitMax, modDepthMin, modDepthMax, modMode, defaultModDepth);
		break;
	}

	ptr->setSaveAndRecall(saveAndRecall);

	// set smoothing defaults
	if (!shouldBeSmoothed)
		ptr->setSmoothingTime(0);
	else
		ptr->setSmoothingTime(10);
}

void myparams::setBounds(int x, int y, int w, int h)
{
	widget->setBounds(x, y, w, h);
}

void myparams::setCallback(std::function<void(double)> cb)
{
	// you forgot to call ParamManager::instantiateParameters(this)	or an init function for this object
	jassert(ptr != nullptr);
	ptr->setValueChangeCallback(cb);
}

void ParamManager::addParameters(vector<myparams*> list)
{
	ParamList.insert(ParamList.end(), list.begin(), list.end());
}

void ParamManager::addParameters(myparams * ptr)
{
	ParamList.push_back(ptr);
}

void ParamManager::setGlobalSmoothingAmount(double v)
{
	for (auto & p : paramsThatShouldBeSmoothed)
			p->ptr->setSmoothingTime(v*1000);
}
