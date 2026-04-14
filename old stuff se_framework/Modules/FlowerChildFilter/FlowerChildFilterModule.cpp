#include "FlowerChildFilterModule.h"

FlowerChildFilterModule::FlowerChildFilterModule(CriticalSection* lockToUse, jura::MetaParameterManager* metaManagerToUse)
	: BasicModule(lockToUse, metaManagerToUse)
	, modulationManager(lockToUse)
{
	ScopedLock scopedLock(*lock);

	setModuleTypeName("FlowerChildFilter");

	jura::Parameter::setStoreDefaultValuesToXml(true);

	MIDIMASTER.addMidiSlave(this, MidiMaster::AddFor::MonoNoteCallbacks);
	MIDIMASTER.addMidiSlave(this, MidiMaster::AddFor::VelocityChange);

	setSmoothingManager(&smootyMan);

	setModulationManager(&modulationManager);
	modulationManager.setMetaParameterManager(metaParamManager);

	midiVelocityModSrc.setValuePtr(&getMidiState().velocity);
	registerModulationSource(&midiVelocityModSrc);

	freqAdsrModSrc.setValuePtr(&fMod);
	registerModulationSource(&freqAdsrModSrc);

	ampAdsrModSrc.setValuePtr(&aMod);
	registerModulationSource(&ampAdsrModSrc);

	addChildAudioModule(oscilloscopeModule = new OscilloscopeModule(lock, metaParamManager, &modulationManager));
	oscilloscopeModule->getParameterByName("OneDimensional")->setValue(1.0, true, true);
	oscilloscopeModule->getParameterByName("Zoom")->setValue(0.0, true, true);
	oscilloscopeModule->getParameterByName("DotLimit")->setValue(1.0, true, true);
	oscilloscopeModule->getParameterByName("NumCycles")->setValue(4.0, true, true);	

	ModulatableParameter2 * p;

	p = new ModulatableParameter2("ParamSmoothing", 0.001, 1, .01, Parameter::EXPONENTIAL);
	addObservedParameter(p);
	p->setSaveAndRecall(false);
	p->setValueChangeCallback([this](double v)
		{
			for (auto& param : parametersForSmoothing)
				param->setSmoothingTime(v * 1000);

			for (auto& obj : smoothingManager->usedSmoothers)
				obj->setTimeConstantAndSampleRate(v * 1000, sampleRate);
		});
	p->setDefaultModParameters
	(
		0,
		2,
		-2,
		+2,
		jura::ModulationConnection::modModes::ABSOLUTE,
		.5
	);

	p = new ModulatableParameter2("Oversampling", 1, 8, 2, Parameter::LINEAR, 1);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { FlowerChildFilter.setOversampling((int)v); });
	p->setDefaultModParameters
	(
		p->getMinValue(),
		p->getMaxValue(),
		-p->getMaxValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		3
	);

	p = new ModulatableParameter2("ChannelMode", 0, 1, 1, Parameter::BOOLEAN);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { FlowerChildFilter.setNumChannels(1 + int(v)); });
	p->setDefaultModParameters
	(
		p->getMinValue(),
		p->getMaxValue(),
		-2,
		+2,
		jura::ModulationConnection::modModes::ABSOLUTE,
		1
	);

	p = new ModulatableParameter2("Cutoff", 0, 1, 1, Parameter::LINEAR);
	parametersForSmoothing.push_back(p);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { FlowerChildFilter.setFrequency(v); });
	p->setDefaultModParameters
	(
		p->getMinValue(),
		p->getMaxValue(),
		-2,
		+2,
		jura::ModulationConnection::modModes::ABSOLUTE,
		.2
	);

	p = new ModulatableParameter2("Resonance", 0, 1, .2, Parameter::LINEAR);
	parametersForSmoothing.push_back(p);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { FlowerChildFilter.setResonance(v); });
	p->setDefaultModParameters
	(
		p->getMinValue(),
		p->getMaxValue(),
		-2,
		+2,
		jura::ModulationConnection::modModes::ABSOLUTE,
		.2
	);

	p = new ModulatableParameter2("Chaos", 0, 1, 0, Parameter::LINEAR);
	parametersForSmoothing.push_back(p);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { FlowerChildFilter.setChaosAmount(v); });
	p->setDefaultModParameters
	(
		p->getMinValue(),
		p->getMaxValue(),
		-2,
		+2,
		jura::ModulationConnection::modModes::ABSOLUTE,
		.2
	);

	p = new ModulatableParameter2("Chaos2", 0, 1, 0, Parameter::LINEAR);
	parametersForSmoothing.push_back(p);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { FlowerChildFilter.setChaosAmount(v); });
	p->setDefaultModParameters
	(
		p->getMinValue(),
		p->getMaxValue(),
		-2,
		+2,
		jura::ModulationConnection::modModes::ABSOLUTE,
		.2
	);

	p = new ModulatableParameter2("Algorithm", 0, FlowerChildFilter.algorithmStrings.size() - 1, 1, Parameter::STRING);
	for (const auto& s : FlowerChildFilter.algorithmStrings)
		p->addStringValue(s);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v)
		{
			FlowerChildFilter.setAlgorithm((FMDFilterStereoWrapper::Algorithm)(int)v);
			ColourGradient g;
			switch ((int)v)
			{
			case 0: default:
				oscilloscopeModule->getParameterByName("GradientSelection")->setValue(8, true, true);
				getParameterByName("Chaos")->callValueChangeCallbacks(getParameterByName("Chaos")->getValue());
				break;
			case 1:
				oscilloscopeModule->getParameterByName("GradientSelection")->setValue(3, true, true);
				getParameterByName("Chaos")->callValueChangeCallbacks(getParameterByName("Chaos")->getValue());
				break;
			case 2:
				oscilloscopeModule->getParameterByName("GradientSelection")->setValue(2, true, true);
				getParameterByName("Chaos")->callValueChangeCallbacks(getParameterByName("Chaos")->getValue());
			case 3:
					oscilloscopeModule->getParameterByName("GradientSelection")->setValue(0, true, true); // so that old value != new value
					g.addColour(0.0, Colour::fromRGB(0, 0, 0));
					g.addColour(0.7, Colour::fromRGB(210, 128, 0));
					g.addColour(1.0, Colour::fromRGB(255, 230, 165));
					oscilloscopeModule->getColorMapPointer()->setFromColourGradient(g);
					getParameterByName("Chaos2")->callValueChangeCallbacks(getParameterByName("Chaos2")->getValue());
					break;
			case 4:
				oscilloscopeModule->getParameterByName("GradientSelection")->setValue(0, true, true); // so that old value != new value
				g.addColour(1.0f - 1.00f, Colour::fromRGB(  0,   0,   0));
				g.addColour(1.0f - 0.89f, Colour::fromRGB( 91,  91,  91));
				g.addColour(1.0f - 0.62f, Colour::fromRGB( 37, 142,  87));
				g.addColour(1.0f - 0.37f, Colour::fromRGB(182, 167, 255));
				g.addColour(1.0f - 0.19f, Colour::fromRGB(250, 186, 255));
				g.addColour(1.0f - 0.00f, Colour::fromRGB(255, 255, 255));
				oscilloscopeModule->getColorMapPointer()->setFromColourGradient(g);
				getParameterByName("Chaos2")->callValueChangeCallbacks(getParameterByName("Chaos2")->getValue());
				break;
			}

			/*
			banana: rgba(255,255,255,1) 0%, rgba(255,242,167,1) 1%, rgba(46,184,182,1) 26%, rgba(111,69,20,1) 69%, rgba(0,0,0,1) 100%
			pink green cyan: rgba(255,255,255,1) 0%, rgba(190,255,253,1) 5%, rgba(203,255,215,1) 13%, rgba(236,168,168,1) 47%, rgba(0,0,0,1) 100
			                 rgba(255,255,255,1) 0%, rgba(208,252,217,1) 15%, rgba(199,254,242,1) 21%, rgba(196,255,252,1) 36%, rgba(177,194,231,1) 55%, rgba(0,0,0,1) 100%
			*/
		});
	p->setDefaultModParameters
	(
		p->getMinValue(),
		p->getMaxValue(),
		-2,
		+2,
		jura::ModulationConnection::modModes::ABSOLUTE,
		1
	);

	p = new ModulatableParameter2("Input", 0, 10, .3, Parameter::LINEAR);
	parametersForSmoothing.push_back(p);
	addObservedParameter(p);
	p->setMapper(new jura::rsParameterMapperRational(p->getMinValue(), p->getMaxValue(), .8));
	p->setValueChangeCallback([this](double v) { FlowerChildFilter.setInputAmplitude(v); });
	p->setDefaultModParameters
	(
		p->getMinValue(),
		p->getMaxValue(),
		-p->getMaxValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		2
	);

	p = new ModulatableParameter2("Output", 0, 10, 1, Parameter::LINEAR);
	parametersForSmoothing.push_back(p);
	addObservedParameter(p);
	p->setMapper(new jura::rsParameterMapperRational(p->getMinValue(), p->getMaxValue(), .8));
	p->setValueChangeCallback([this](double v) { FlowerChildFilter.setOutputAmplitude(v); });
	p->setDefaultModParameters
	(
		p->getMinValue(),
		p->getMaxValue(),
		-p->getMaxValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		2
	);

	p = new ModulatableParameter2("CutoffEnvAttack", .001, 10, .001, Parameter::EXPONENTIAL);
	parametersForSmoothing.push_back(p);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { cutoffSlewLimiter.setAttackTime(v); });
	p->setDefaultModParameters
	(
		p->getMinValue(),
		p->getMaxValue(),
		-p->getMaxValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		5
	);

	p = new ModulatableParameter2("CutoffEnvHold", .001, 10, .001, Parameter::EXPONENTIAL);
	parametersForSmoothing.push_back(p);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { cutoffSlewLimiter.setHoldTime(v);	});
	p->setDefaultModParameters
	(
		p->getMinValue(),
		p->getMaxValue(),
		-p->getMaxValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		5
	);

	p = new ModulatableParameter2("CutoffEnvDecay", .001, 10, .001, Parameter::EXPONENTIAL);
	parametersForSmoothing.push_back(p);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { cutoffSlewLimiter.setDecayTime(v); });
	p->setDefaultModParameters
	(
		p->getMinValue(),
		p->getMaxValue(),
		-p->getMaxValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		5
	);

	p = new ModulatableParameter2("CutoffADSRAttack", .001, 10, .001, Parameter::EXPONENTIAL);
	parametersForSmoothing.push_back(p);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { cutoffADSR.setAttack(v); });
	p->setDefaultModParameters
	(
		p->getMinValue(),
		p->getMaxValue(),
		-p->getMaxValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		5
	);

	p = new ModulatableParameter2("CutoffADSRDecay", .001, 10, .001, Parameter::EXPONENTIAL);
	parametersForSmoothing.push_back(p);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { cutoffADSR.setDecay(v); });
	p->setDefaultModParameters
	(
		p->getMinValue(),
		p->getMaxValue(),
		-p->getMaxValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		5
	);

	p = new ModulatableParameter2("CutoffADSRSustain", 0, 1, .5, Parameter::LINEAR);
	parametersForSmoothing.push_back(p);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { cutoffADSR.setSustainLevel(v); });
	p->setDefaultModParameters
	(
		p->getMinValue(),
		p->getMaxValue(),
		-p->getMaxValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		5
	);

	p = new ModulatableParameter2("CutoffADSRRelease", .001, 10, .001, Parameter::EXPONENTIAL);
	parametersForSmoothing.push_back(p);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { cutoffADSR.setRelease(v); });
	p->setDefaultModParameters
	(
		p->getMinValue(),
		p->getMaxValue(),
		-p->getMaxValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		5
	);

	p = new ModulatableParameter2("CutoffADSRAttackCurve", -10, +10, 1, Parameter::LINEAR);
	parametersForSmoothing.push_back(p);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { cutoffADSR.setAttackCurve(v); });
	p->setDefaultModParameters
	(
		p->getMinValue(),
		p->getMaxValue(),
		-p->getMaxValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		1
	);

	p = new ModulatableParameter2("CutoffADSRDecayCurve", -10, +10, 1, Parameter::LINEAR);
	parametersForSmoothing.push_back(p);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { cutoffADSR.setDecayCurve(v); });
	p->setDefaultModParameters
	(
		p->getMinValue(),
		p->getMaxValue(),
		-p->getMaxValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		1
	);

	p = new ModulatableParameter2("CutoffADSRReleaseCurve", -10, +10, 1, Parameter::LINEAR);
	parametersForSmoothing.push_back(p);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { cutoffADSR.setReleaseCurve(v); });
	p->setDefaultModParameters
	(
		p->getMinValue(),
		p->getMaxValue(),
		-p->getMaxValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		1
	);


	p = new ModulatableParameter2("AmpEnvAttack", .001, 10, .001, Parameter::EXPONENTIAL);
	parametersForSmoothing.push_back(p);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { amplitudeSlewLimiter.setAttackTime(v); });
	p->setDefaultModParameters
	(
		p->getMinValue(),
		p->getMaxValue(),
		-p->getMaxValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		5
	);

	p = new ModulatableParameter2("AmpEnvHold", .001, 10, .001, Parameter::EXPONENTIAL);
	parametersForSmoothing.push_back(p);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { amplitudeSlewLimiter.setHoldTime(v); });
	p->setDefaultModParameters
	(
		p->getMinValue(),
		p->getMaxValue(),
		-p->getMaxValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		5
	);

	p = new ModulatableParameter2("AmpEnvDecay", .001, 10, .001, Parameter::EXPONENTIAL);
	parametersForSmoothing.push_back(p);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { amplitudeSlewLimiter.setDecayTime(v); });
	p->setDefaultModParameters
	(
		p->getMinValue(),
		p->getMaxValue(),
		-p->getMaxValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		5
	);

	p = new ModulatableParameter2("AmpADSRAttack", .001, 10, .001, Parameter::EXPONENTIAL);
	parametersForSmoothing.push_back(p);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { amplitudeADSR.setAttack(v); });
	p->setDefaultModParameters
	(
		p->getMinValue(),
		p->getMaxValue(),
		-p->getMaxValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		5
	);

	p = new ModulatableParameter2("AmpADSRDecay", .001, 10, .001, Parameter::EXPONENTIAL);
	parametersForSmoothing.push_back(p);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { amplitudeADSR.setDecay(v); });
	p->setDefaultModParameters
	(
		p->getMinValue(),
		p->getMaxValue(),
		-p->getMaxValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		5
	);

	p = new ModulatableParameter2("AmpADSRSustain", 0, 1, .5, Parameter::LINEAR);
	parametersForSmoothing.push_back(p);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { amplitudeADSR.setSustainLevel(v); });
	p->setDefaultModParameters
	(
		p->getMinValue(),
		p->getMaxValue(),
		-p->getMaxValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		5
	);

	p = new ModulatableParameter2("AmpADSRRelease", .001, 10, .001, Parameter::EXPONENTIAL);
	parametersForSmoothing.push_back(p);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { amplitudeADSR.setRelease(v); });
	p->setDefaultModParameters
	(
		p->getMinValue(),
		p->getMaxValue(),
		-p->getMaxValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		5
	);

	p = new ModulatableParameter2("AmpADSRAttackCurve", -10, +10, 1, Parameter::LINEAR);
	parametersForSmoothing.push_back(p);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { amplitudeADSR.setAttackCurve(v); });
	p->setDefaultModParameters
	(
		p->getMinValue(),
		p->getMaxValue(),
		-p->getMaxValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		1
	);

	p = new ModulatableParameter2("AmpADSRDecayCurve", -10, +10, 1, Parameter::LINEAR);
	parametersForSmoothing.push_back(p);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { amplitudeADSR.setDecayCurve(v); });
	p->setDefaultModParameters
	(
		p->getMinValue(),
		p->getMaxValue(),
		-p->getMaxValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		1
	);

	p = new ModulatableParameter2("AmpADSRReleaseCurve", -10, +10, 1, Parameter::LINEAR);
	parametersForSmoothing.push_back(p);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { amplitudeADSR.setReleaseCurve(v); });
	p->setDefaultModParameters
	(
		p->getMinValue(),
		p->getMaxValue(),
		-p->getMaxValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		1
	);

	p = new ModulatableParameter2("CutoffEnvMode", 0, 1, 0, Parameter::LINEAR);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v)
		{
			freqEnvMode = v;
			updateEnvMode();
		});
	p->setDefaultModParameters
	(
		p->getMinValue(),
		p->getMaxValue(),
		-p->getMaxValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		2
	);

	p = new ModulatableParameter2("CutoffAdsrTrigMode", 0, 1, 0, Parameter::LINEAR);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v)
		{
			freqAdsrTrigMode = v;
			updateEnvMode();
		});
	p->setDefaultModParameters
	(
		p->getMinValue(),
		p->getMaxValue(),
		-p->getMaxValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		2
	);

	p = new ModulatableParameter2("CutoffEnvSend", -10, 10, 0, Parameter::LINEAR);
	parametersForSmoothing.push_back(p);
	addObservedParameter(p);
	p->setMapper(new jura::rsParameterMapperSinh(p->getMinValue(), p->getMaxValue(), 4));
	p->setValueChangeCallback([this](double v)
		{
			freqModGain = v * 2;
			cutoffADSR.setAmplitudeScale(freqModGain);
		});
	p->setDefaultModParameters
	(
		p->getMinValue(),
		p->getMaxValue(),
		-p->getMaxValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		2
	);

	p = new ModulatableParameter2("AmpEnvMode", 0, 1, 0, Parameter::LINEAR);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v)
		{
			ampEnvMode = v;
			updateEnvMode();
		});
	p->setDefaultModParameters
	(
		p->getMinValue(),
		p->getMaxValue(),
		-p->getMaxValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		2
	);

	p = new ModulatableParameter2("AmpAdsrTrigMode", 0, 1, 0, Parameter::LINEAR);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v)
		{
			ampAdsrTrigMode = v;
			updateEnvMode();
		});
	p->setDefaultModParameters
	(
		p->getMinValue(),
		p->getMaxValue(),
		-p->getMaxValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		2
	);

	p = new ModulatableParameter2("AmpEnvSend", -10, 10, 0, Parameter::LINEAR);
	parametersForSmoothing.push_back(p);
	addObservedParameter(p);
	p->setMapper(new jura::rsParameterMapperSinh(p->getMinValue(), p->getMaxValue(), 4));
	p->setValueChangeCallback([this](double v)
		{
			ampModGain = v * 2;
			amplitudeADSR.setAmplitudeScale(ampModGain);
		});
	p->setDefaultModParameters
	(
		p->getMinValue(),
		p->getMaxValue(),
		-p->getMaxValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		2
	);

	p = new ModulatableParameter2("DryWet", 0, 1, 1, Parameter::LINEAR);
	parametersForSmoothing.push_back(p);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v)
		{
			FlowerChildFilter.setDryWet(v);
		});
	p->setDefaultModParameters
	(
		0,
		2,
		-2,
		+2,
		jura::ModulationConnection::modModes::ABSOLUTE,
		.5
	);

	DBG("Num parameters = " + String(getNumParameters()));

	//p = new ModulatableParameter2("AmpEnvSend", 0, 1, 0, Parameter::EXPONENTIAL);
	//parametersForSmoothing.push_back(p);
	//addObservedParameter(p);
	//p->setValueChangeCallback([this](double v)
	//{
	//	auto filter1 = dynamic_cast<FlowerChildRev1*>(FlowerChildFilter.getFMDFilterLeftPointer());
	//	auto filter2 = dynamic_cast<FlowerChildRev1*>(FlowerChildFilter.getFMDFilterRightPointer());

	//	if (filter1 != nullptr)
	//	{
	//		filter1->setSmootherFrequency(v);
	//		filter2->setSmootherFrequency(v);
	//	}
	//});

	//p = new ModulatableParameter2("DryWet", .001, 20000, 20000, Parameter::EXPONENTIAL);
	//parametersForSmoothing.push_back(p);
	//addObservedParameter(p);
	//p->setValueChangeCallback([this](double v)
	//{
	//	auto filter1 = dynamic_cast<FlowerChildRev1*>(FlowerChildFilter.getFMDFilterLeftPointer());
	//	auto filter2 = dynamic_cast<FlowerChildRev1*>(FlowerChildFilter.getFMDFilterRightPointer());

	//	if (filter1 != nullptr)
	//	{
	//		filter1->setDownsamplerFrequency(v);
	//		filter2->setDownsamplerFrequency(v);
	//	}
	//});
}

void FlowerChildFilterModule::setStateFromXml(const juce::XmlElement & xmlState, const juce::String & stateName, bool markAsClean)
{
	double v = getParameterByName("ParamSmoothing")->getValue();

	auto state = xmlState;

	state.deleteAllChildElementsWithTagName("Scope");

	BasicModule::setStateFromXml(state, stateName, markAsClean);

	XmlElement* modXml = state.getChildByName("Modulations");
	if (modXml != nullptr)
		modulationManager.setStateFromXml(*modXml);
	else
		modulationManager.removeAllConnections();

	getParameterByName("ParamSmoothing")->setValue(v, true, true);
}

XmlElement* FlowerChildFilterModule::getStateAsXml(const juce::String& stateName, bool markAsClean)
{
	auto state = BasicModule::getStateAsXml(stateName, markAsClean);

	state->deleteAllChildElementsWithTagName("Scope");

	state->addChildElement(modulationManager.getStateAsXml());

	return state;
}

void FlowerChildFilterModule::setSampleRate(double v)
{
	sampleRate = v;

	FlowerChildFilter.setSampleRate(v);

	cutoffSlewLimiter.setSampleRate(v);
	amplitudeSlewLimiter.setSampleRate(v);
	cutoffADSR.setSampleRate(v);
	amplitudeADSR.setSampleRate(v);

	for (auto& obj : childModules)
		obj->setSampleRate(v);
}

void FlowerChildFilterModule::processBlock(double** inOutBuffer, int numChannels, int numSamples)
{
	const bool doApplySmoothing = smoothingManager->needsSmoothing();

	const bool doApplyModulations = modulationManager.getNumConnections() != 0;

	double left = 0, right = 0;
	for (int n = 0; n < numSamples; n++)
	{
		if (doApplySmoothing)
			smoothingManager->updateSmoothedValues();

		if (inOutBuffer[0]) left = inOutBuffer[0][n];
		if (inOutBuffer[1]) right = inOutBuffer[1][n];

		double monoSignalClipped = clip((left + right) * 0.5, -1.0, 1.0);

		switch (cutoffEnvelopeType)
		{
		case SLEW_LIMITER: default:			
			fMod = cutoffSlewLimiter.getSample(monoSignalClipped) * freqModGain;
			break;
		case ADSR_ALWAYS:
		case ADSR_LEGATO:
			fMod = cutoffADSR.getSample();
		}

		switch (amplitudeEnvelopeType)
		{
		case SLEW_LIMITER: default:
			aMod = amplitudeSlewLimiter.getSample(monoSignalClipped) * ampModGain;
			break;
		case ADSR_ALWAYS:
		case ADSR_LEGATO:
			aMod = amplitudeADSR.getSample() * 0.5;
		}

		if (doApplyModulations)
			modulationManager.applyModulations();

		FlowerChildFilter.setFrequencyMod(fMod);
		FlowerChildFilter.setAmplitudeMod(aMod);
		FlowerChildFilter.getSampleStereo(left, right, &left, &right);

		oscilloscopeModule->getSample(clip(left * 2.4, -2.4, +2.4));

		if (inOutBuffer[0]) inOutBuffer[0][n] = left;
		if (inOutBuffer[1]) inOutBuffer[1][n] = right;
	}
}
