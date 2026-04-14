#include "PrettyScopeModule.h"

#include <map>
#include <vector>

using namespace se;
using namespace jura;
using namespace rosic;
using namespace juce;

juce::String valuetoFPS(double v) { return v > 0.5 ? "inf" : "VSYNC"; }
juce::String valuetoDecaySpeed(double v) { return v <= 0 ? "FROZEN" : juce::String(v, 3); }
juce::String valuetoPPF(double v) { return v < 1 ? "AUTO" : juce::String(v).upToFirstOccurrenceOf(".", false, false); } // for some reason juce::String(v, 0) doesn't work.

PrettyScopeModule::PrettyScopeModule(CriticalSection *lockToUse, jura::MetaParameterManager * metaManagerToUse)
	: BasicModule(lockToUse, metaManagerToUse)
	, audioBuffer(lockToUse)
	, modulationManager(lockToUse)
{
  ScopedLock scopedLock(*lock);

	midiMaster.addMidiSlave(this);

	setModulationManager(&modulationManager);
	modulationManager.setMetaParameterManager(metaParamManager);

	setModuleTypeName("PrettyScope");

	// setup brushes
	brushes = vector<Brush*>{ &line1, &dot1, &line2, &dot2 };
	
  juce::ColourGradient g;
  g.addColour(0.00, Colour(  0,   2,  48));
	g.addColour(0.30, Colour(101, 000, 173));
  g.addColour(0.50, Colour(255, 000, 254));
	g.addColour(1.00, Colour(255, 199, 229));
  colorMap.setFromColourGradient(g);

	frameRateSmoother.setSmootherType(ParamSmoother::type::EXPONENTIAL);
	frameRateSmoother.setSmoothingTime(.01);

	createParameters();
}

void PrettyScopeModule::createParameters()
{
	ModulatableParameter2* p;

	p = new ModulatableParameter2("ParameterSmoothing", 1.e-6, 10, .06, Parameter::EXPONENTIAL, 0);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v)
		{
			for (auto& param : parametersForSmoothing)
				param->setSmoothingTime(v * 1000);

			if (smoothingManager != nullptr)
				for (auto& obj : smoothingManager->usedSmoothers)
					obj->setTimeConstantAndSampleRate(v * 1000, sampleRate);
		});
	p->setDefaultModParameters
	(
		-1.e+100,
		+1.e+100,
		p->getMinValue(),
		p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		1
	);

	p = new ModulatableParameter2("Clear", 0, 1, 0, Parameter::LINEAR, 1);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v)
		{
		/* While parClear is more than or equal to 0.5, doClearCanvas will be set to true. However, we only want
		to clear the canvas once while this is true, so we have another bool canvasWasCleared which will be set
		to true when OpenGLCanvas does clear and we only set it back to false if canvas was clearned AND if
		value is below 0.5. Then doClearCanvas becomes false and canvas will not be cleared until value is again
		more than or equal to 0.5.*/
		doClearCanvas = v >= 0.5;
		canvasWasCleared = canvasWasCleared && doClearCanvas;
		});
	p->setDefaultModParameters
	(
		-1.e+100,
		+1.e+100,
		-p->getMinValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		jmin<double>(1, (int)std::abs(p->getMaxValue() - p->getMinValue()) / 5)
	);

	p = new ModulatableParameter2("FXMode", 0, 1, 0, Parameter::LINEAR, 1);
	addObservedParameter(p);
	fxModePar = p;
	p->setDefaultModParameters
	(
		-1.e+100,
		+1.e+100,
		-p->getMinValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		jmin<double>(1, (int)std::abs(p->getMaxValue() - p->getMinValue()) / 5)
	);

	p = new ModulatableParameter2("Pause", 0, 1, 0, Parameter::LINEAR, 1);
	addObservedParameter(p);
	p->setDefaultModParameters
	(
		-1.e+100,
		+1.e+100,
		-p->getMinValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		jmin<double>(1, (int)std::abs(p->getMaxValue() - p->getMinValue()) / 5)
	);

	p = new ModulatableParameter2("AutoPauseMode", 0, 2, 0, Parameter::STRING, 1);
	addObservedParameter(p);
	p->addStringValue("AutoPause: None");
	p->addStringValue("AutoPause: Silence");
	p->addStringValue("AutoPause: Mouse");
	p->setDefaultModParameters
	(
		-1.e+100,
		+1.e+100,
		-p->getMinValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		jmin<double>(1, (int)std::abs(p->getMaxValue() - p->getMinValue()) / 5)
	);

	p = new ModulatableParameter2("FreezePoints", 0, 1, 0, Parameter::LINEAR, 1);
	addObservedParameter(p);
	p->setDefaultModParameters
	(
		-1.e+100,
		+1.e+100,
		-p->getMinValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		jmin<double>(1, (int)std::abs(p->getMaxValue() - p->getMinValue()) / 5)
	);

	p = new ModulatableParameter2("ShowFPS", 0, 1, 0, Parameter::LINEAR, 1);
	addObservedParameter(p);
	p->setDefaultModParameters
	(
		-1.e+100,
		+1.e+100,
		-p->getMinValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		jmin<double>(1, (int)std::abs(p->getMaxValue() - p->getMinValue()) / 5)
	);

	p = new ModulatableParameter2("Heatmap", 0, 1, 0, Parameter::LINEAR, 1);
	addObservedParameter(p);
	p->setDefaultModParameters
	(
		-1.e+100,
		+1.e+100,
		-p->getMinValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		jmin<double>(1, (int)std::abs(p->getMaxValue() - p->getMinValue()) / 5)
	);

	p = new ModulatableParameter2("BlendMode", 0, 4, 0, Parameter::STRING, 1);
	addObservedParameter(p);
	p->addStringValue("Blend Mode: Laser");
	p->addStringValue("Blend Mode: LED");
	p->addStringValue("Blend Mode: Light");
	p->addStringValue("Blend Mode: Paint");
	p->addStringValue("Blend Mode: Solid");
	p->setDefaultModParameters
	(
		-1.e+100,
		+1.e+100,
		-p->getMinValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		jmin<double>(1, (int)std::abs(p->getMaxValue() - p->getMinValue()) / 5)
	);

	p = new ModulatableParameter2("DotColorMode", 0, 1, 0, Parameter::STRING, 1);
	addObservedParameter(p);
	p->addStringValue("Color: R G B");
	p->addStringValue("Color: H S L");
	p->setValueChangeCallback([this](double v)
		{
		dot1.setColorMode((ColorMem::ColorMode)(int)v);
		dot2.setColorMode((ColorMem::ColorMode)(int)v);
		});
	p->setDefaultModParameters
	(
		-1.e+100,
		+1.e+100,
		-p->getMinValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		jmin<double>(1, (int)std::abs(p->getMaxValue() - p->getMinValue()) / 5)
	);

	p = new ModulatableParameter2("LineColorMode", 0, 1, 0, Parameter::STRING, 1);
	addObservedParameter(p);
	p->addStringValue("Color: R G B");
	p->addStringValue("Color: H S L");
	p->addStringValue("Color: RdGnBl");
	p->addStringValue("Color: PrttyGrl");
	p->setValueChangeCallback([this](double v)
		{
			line1.setColorMode((ColorMem::ColorMode)(int)v);
			line2.setColorMode((ColorMem::ColorMode)(int)v);
		});
	p->setDefaultModParameters
	(
		-1.e+100,
		+1.e+100,
		-p->getMinValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		jmin<double>(1, (int)std::abs(p->getMaxValue() - p->getMinValue()) / 5)
	);

	p = new ModulatableParameter2("Framerate", 0, 1, 0);
	addObservedParameter(p);
	p->setDefaultModParameters
	(
		-1.e+100,
		+1.e+100,
		-p->getMinValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		jmin<double>(1, (int)std::abs(p->getMaxValue() - p->getMinValue()) / 5)
	);

	p = new ModulatableParameter2("DecaySpeed", 0, 1, 0);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setDefaultModParameters
	(
		-1.e+100,
		+1.e+100,
		-p->getMinValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		jmin<double>(1, (int)std::abs(p->getMaxValue() - p->getMinValue()) / 5)
	);

	p = new ModulatableParameter2("PointsPerFrame", 0.1, 8192, 2048, Parameter::EXPONENTIAL, 1);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { audioBuffer.setPointsPerFrame(v); });
	p->setDefaultModParameters
	(
		-1.e+100,
		+1.e+100,
		p->getMinValue(),
		p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		1
	);

	p = new ModulatableParameter2("TailFade", 0, 10, 0);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { tailFadeFactor = v; });
	p->setDefaultModParameters
	(
		-1.e+100,
		+1.e+100,
		-p->getMinValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		p->getMaxValue() / 10.0
	);

	p = new ModulatableParameter2("Dots", 0, 1, 1, Parameter::LINEAR, 1);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { dot1.doDraw = dot2.doDraw = v >= 0.5 ? true : false; });
	p->setDefaultModParameters
	(
		-1.e+100,
		+1.e+100,
		-p->getMinValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		jmin<double>(1, (int)std::abs(p->getMaxValue() - p->getMinValue()) / 5)
	);

	p = new ModulatableParameter2("DotBrightness", 0.0001, 1, 0.05, Parameter::EXPONENTIAL);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { dot1.intensity = dot2.intensity = v; });
	p->setDefaultModParameters
	(
		-1.e+100,
		+1.e+100,
		p->getMinValue(),
		p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		1
	);

	p = new ModulatableParameter2("DotSize", 0, 1, 0.1);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { dot1.size = dot2.size = v; });
	p->setDefaultModParameters
	(
		-1.e+100,
		+1.e+100,
		-p->getMinValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		jmin<double>(1, (int)std::abs(p->getMaxValue() - p->getMinValue()) / 5)
	);

	p = new ModulatableParameter2("DotBlur", 0, 1, .2);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { dot1.blur = dot2.blur = v; });
	p->setDefaultModParameters
	(
		-1.e+100,
		+1.e+100,
		-p->getMinValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		p->getMaxValue() / 10.0
	);

	p = new ModulatableParameter2("DotProfile", 0, 2, 0, Parameter::STRING, 1);
	addObservedParameter(p);
	p->addStringValue("Gaussian");
	p->addStringValue("Pointy");
	p->addStringValue("Flat");
	p->setValueChangeCallback([this](double v) { dot1.profile = dot2.profile = (int)v; });
	p->setDefaultModParameters
	(
		-1.e+100,
		+1.e+100,
		-p->getMinValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		jmin<double>(1, (int)std::abs(p->getMaxValue() - p->getMinValue()) / 5)
	);

	p = new ModulatableParameter2("DrawOrder", 0, 1, 1);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v)
		{
			if (v)
			{
				brushes[0] = &dot1;
				brushes[1] = &line1;
				brushes[2] = &dot2;
				brushes[3] = &line2;
			}
			else
			{
				brushes[0] = &line1;
				brushes[1] = &dot1;
				brushes[2] = &line2;
				brushes[3] = &dot2;
			}
		});
	p->setDefaultModParameters
	(
		-1.e+100,
		+1.e+100,
		-p->getMinValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		jmin<double>(1, (int)std::abs(p->getMaxValue() - p->getMinValue()) / 5)
	);

	p = new ModulatableParameter2("Lines", 0, 1, 1);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { line1.doDraw = line2.doDraw = v >= 0.5 ? true : false; });
	p->setDefaultModParameters
	(
		-1.e+100,
		+1.e+100,
		-p->getMinValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		jmin<double>(1, (int)std::abs(p->getMaxValue() - p->getMinValue()) / 5)
	);

	p = new ModulatableParameter2("LineBrightness", 0.0001, 1, 0.23, Parameter::EXPONENTIAL);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { line1.intensity = line2.intensity = v; });
	p->setDefaultModParameters
	(
		-1.e+100,
		+1.e+100,
		p->getMinValue(),
		p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		1
	);

	p = new ModulatableParameter2("LineSize", 0, 1, 0.02);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { line1.size = line2.size = v; });
	p->setDefaultModParameters
	(
		-1.e+100,
		+1.e+100,
		-p->getMinValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		p->getMaxValue() / 10.0
	);

	p = new ModulatableParameter2("LineBlur", 0, 1, 0.2);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { line1.blur = line2.blur = v; });
	p->setDefaultModParameters
	(
		-1.e+100,
		+1.e+100,
		-p->getMinValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		p->getMaxValue() / 10.0
	);

	p = new ModulatableParameter2("LenFactor", 0, 1, 1);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setDefaultModParameters
	(
		-1.e+100,
		+1.e+100,
		-p->getMinValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		p->getMaxValue() / 10.0
	);

	p = new ModulatableParameter2("LineProfile", 0, 2, 0, Parameter::STRING);
	addObservedParameter(p);
	p->addStringValue("Gaussian");
	p->addStringValue("Pointy");
	p->addStringValue("Flat");
	p->setValueChangeCallback([this](double v) { line1.profile = line2.profile = (int)v; });
	p->setDefaultModParameters
	(
		-1.e+100,
		+1.e+100,
		-p->getMinValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		jmin<double>(1, (int)std::abs(p->getMaxValue() - p->getMinValue()) / 5)
	);

	p = new ModulatableParameter2("DotColorR", 0, 1, 1);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { dot1.setRed(v); });
	p->setDefaultModParameters
	(
		-1.e+100,
		+1.e+100,
		-p->getMinValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		p->getMaxValue() / 10.0
	);

	p = new ModulatableParameter2("DotColorG", 0, 1, 1);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { dot1.setGreen(v); });
	p->setDefaultModParameters
	(
		-1.e+100,
		+1.e+100,
		-p->getMinValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		p->getMaxValue() / 10.0
	);

	p = new ModulatableParameter2("DotColorB", 0, 1, 1);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { dot1.setBlue(v); });
	p->setDefaultModParameters
	(
		-1.e+100,
		+1.e+100,
		-p->getMinValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		p->getMaxValue() / 10.0
	);

	p = new ModulatableParameter2("DotColorH", 0, 360, 0);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { dot1.setHue(v / 360.0); });
	p->setDefaultModParameters
	(
		-1.e+100,
		+1.e+100,
		-p->getMinValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		p->getMaxValue() / 10.0
	);

	p = new ModulatableParameter2("DotColorS", 0, 1, 1);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { dot1.setSaturation(v); });
	p->setDefaultModParameters
	(
		-1.e+100,
		+1.e+100,
		-p->getMinValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		p->getMaxValue() / 10.0
	);

	p = new ModulatableParameter2("DotColorL", 0, 1, 1);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { dot1.setLightness(v); });
	p->setDefaultModParameters
	(
		-1.e+100,
		+1.e+100,
		-p->getMinValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		p->getMaxValue() / 10.0
	);

	p = new ModulatableParameter2("LineColorR", 0, 1, 1);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { line1.setRed(v); });
	p->setDefaultModParameters
	(
		-1.e+100,
		+1.e+100,
		-p->getMinValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		p->getMaxValue() / 10.0
	);
	p = new ModulatableParameter2("LineColorG", 0, 1, 1);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { line1.setGreen(v); });
	p->setDefaultModParameters
	(
		-1.e+100,
		+1.e+100,
		-p->getMinValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		p->getMaxValue() / 10.0
	);
	p = new ModulatableParameter2("LineColorB", 0, 1, 1);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { line1.setBlue(v); });
	p->setDefaultModParameters
	(
		-1.e+100,
		+1.e+100,
		-p->getMinValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		p->getMaxValue() / 10.0
	);

	p = new ModulatableParameter2("LineColorH", 0, 360, 0);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { line1.setHue(v / 360.0); });
	p->setDefaultModParameters
	(
		-1.e+100,
		+1.e+100,
		-p->getMinValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		p->getMaxValue() / 10.0
	);

	p = new ModulatableParameter2("LineColorS", 0, 1, 1);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { line1.setSaturation(v); });
	p->setDefaultModParameters
	(
		-1.e+100,
		+1.e+100,
		-p->getMinValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		p->getMaxValue() / 10.0
	);

	p = new ModulatableParameter2("LineColorL", 0, 1, 1);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { line1.setLightness(v); });
	p->setDefaultModParameters
	(
		-1.e+100,
		+1.e+100,
		-p->getMinValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		p->getMaxValue() / 10.0
	);

	p = new ModulatableParameter2("DotColorR2", 0, 1, 1);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { dot2.setRed(v); });
	p->setDefaultModParameters
	(
		-1.e+100,
		+1.e+100,
		-p->getMinValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		p->getMaxValue() / 10.0
	);

	p = new ModulatableParameter2("DotColorG2", 0, 1, 1);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { dot2.setGreen(v); });
	p->setDefaultModParameters
	(
		-1.e+100,
		+1.e+100,
		-p->getMinValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		p->getMaxValue() / 10.0
	);

	p = new ModulatableParameter2("DotColorB2", 0, 1, 1);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { dot2.setBlue(v); });
	p->setDefaultModParameters
	(
		-1.e+100,
		+1.e+100,
		-p->getMinValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		p->getMaxValue() / 10.0
	);

	p = new ModulatableParameter2("DotColorH2", 0, 360, 0);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { dot2.setHue(v / 360.0); });
	p->setDefaultModParameters
	(
		-1.e+100,
		+1.e+100,
		-p->getMinValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		p->getMaxValue() / 10.0
	);

	p = new ModulatableParameter2("DotColorS2", 0, 1, 1);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { dot2.setSaturation(v); });
	p->setDefaultModParameters
	(
		-1.e+100,
		+1.e+100,
		-p->getMinValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		p->getMaxValue() / 10.0
	);

	p = new ModulatableParameter2("DotColorL2", 0, 1, 1);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { dot2.setLightness(v); });
	p->setDefaultModParameters
	(
		-1.e+100,
		+1.e+100,
		-p->getMinValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		p->getMaxValue() / 10.0
	);

	p = new ModulatableParameter2("LineColorR2", 0, 1, 1);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { line2.setRed(v); });
	p->setDefaultModParameters
	(
		-1.e+100,
		+1.e+100,
		-p->getMinValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		p->getMaxValue() / 10.0
	);

	p = new ModulatableParameter2("LineColorG2", 0, 1, 1);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { line2.setGreen(v); });
	p->setDefaultModParameters
	(
		-1.e+100,
		+1.e+100,
		-p->getMinValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		p->getMaxValue() / 10.0
	);

	p = new ModulatableParameter2("LineColorB2", 0, 1, 1);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) {line2.setBlue(v); });
	p->setDefaultModParameters
	(
		-1.e+100,
		+1.e+100,
		-p->getMinValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		p->getMaxValue() / 10.0
	);
 
	p = new ModulatableParameter2("LineColorH2", 0, 360, 0);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { line2.setHue(v / 360.0); });
	p->setDefaultModParameters
	(
		-1.e+100,
		+1.e+100,
		-p->getMinValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		p->getMaxValue() / 10.0
	);

	p = new ModulatableParameter2("LineColorS2", 0, 1, 1);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { line2.setSaturation(v); });
	p->setDefaultModParameters
	(
		-1.e+100,
		+1.e+100,
		-p->getMinValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		p->getMaxValue() / 10.0
	);

	p = new ModulatableParameter2("LineColorL2", 0, 1, 1);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { line2.setLightness(v); });
	p->setDefaultModParameters
	(
		-1.e+100,
		+1.e+100,
		-p->getMinValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		p->getMaxValue() / 10.0
	);

	p = new ModulatableParameter2("LenColorIntensity", 0.001, 100, 25.0);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { line1.lenColorIntensity = line2.lenColorIntensity = v; });
	p->setDefaultModParameters
	(
		-1.e+100,
		+1.e+100,
		-p->getMinValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		jmin<double>(1, (int)std::abs(p->getMaxValue() - p->getMinValue()) / 5)
	);

	p = new ModulatableParameter2("Zoom", 0.000001, 8, 1, Parameter::EXPONENTIAL);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { audioBuffer.setZoom(v); });
	p->setDefaultModParameters
	(
		-1.e+100,
		+1.e+100,
		p->getMinValue(),
		p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		1
	);

	p = new ModulatableParameter2("RotationX", -360, +360, 0, Parameter::LINEAR_BIPOLAR);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { audioBuffer.setRotationX(v / 360.0); });
	p->setDefaultModParameters
	(
		-1.e+100,
		+1.e+100,
		-p->getMinValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		p->getMaxValue() / 10.0
	);

	p = new ModulatableParameter2("RotationY", -360, +360, 0, Parameter::LINEAR_BIPOLAR);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { audioBuffer.setRotationY(v / 360.0); });
	p->setDefaultModParameters
	(
		-1.e+100,
		+1.e+100,
		-p->getMinValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		p->getMaxValue() / 10.0
	);


	p = new ModulatableParameter2("RotationZ", -360, +360, 0, Parameter::LINEAR_BIPOLAR);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { audioBuffer.setRotationZ(v / 360.0); });
	p->setDefaultModParameters
	(
		-1.e+100,
		+1.e+100,
		-p->getMinValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		p->getMaxValue() / 10.0
	);

	p = new ModulatableParameter2("ScaleX", -8, 8, 1, Parameter::LINEAR_BIPOLAR);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { audioBuffer.setScaleX(v); });
	p->setDefaultModParameters
	(
		-1.e+100,
		+1.e+100,
		-p->getMinValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		p->getMaxValue() / 10.0
	);


	p = new ModulatableParameter2("ScaleY", -8, 8, 1, Parameter::LINEAR_BIPOLAR);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { audioBuffer.setScaleY(v); });
	p->setDefaultModParameters
	(
		-1.e+100,
		+1.e+100,
		-p->getMinValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		p->getMaxValue() / 10.0
	);

	p = new ModulatableParameter2("ShiftX", -1, 1, 0, Parameter::LINEAR_BIPOLAR);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { audioBuffer.setShiftX(v); });
	p->setDefaultModParameters
	(
		-1.e+100,
		+1.e+100,
		-p->getMinValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		p->getMaxValue() / 10.0
	);

	p = new ModulatableParameter2("ShiftY", -1, 1, 0, Parameter::LINEAR_BIPOLAR);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setValueChangeCallback([this](double v) { audioBuffer.setShiftY(v); });
	p->setDefaultModParameters
	(
		-1.e+100,
		+1.e+100,
		-p->getMinValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		p->getMaxValue() / 10.0
	);

	p = new ModulatableParameter2("OneDimZoom", 0, 8., 1);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setDefaultModParameters
	(
		-1.e+100,
		+1.e+100,
		-p->getMinValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		p->getMaxValue() / 10.0
	);

	p = new ModulatableParameter2("OneDimOffset", -1, 1, 0, Parameter::LINEAR_BIPOLAR);
	addObservedParameter(p);
	parametersForSmoothing.push_back(p);
	p->setDefaultModParameters
	(
		-1.e+100,
		+1.e+100,
		-p->getMinValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		p->getMaxValue() / 10.0
	);

	p = new ModulatableParameter2("OneDimensional", 0, 1, 0, Parameter::LINEAR, 1);
	addObservedParameter(p);
	oneDimensionalPar = p;
	p->setDefaultModParameters
	(
		-1.e+100,
		+1.e+100,
		-p->getMinValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		jmin(1, (int)std::abs(p->getMaxValue() - p->getMinValue()) / 5)
	);

	p = new ModulatableParameter2("ScanFrequency", 0.01, 3000, 25, Parameter::EXPONENTIAL);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { screenScanner.setScanFreqNoSync(v); });
	p->setDefaultModParameters
	(
		-1.e+100,
		+1.e+100,
		p->getMinValue(),
		p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		1
	);

	p = new ModulatableParameter2("SyncMode", 0, 1, 0, Parameter::STRING);
	addObservedParameter(p);
	syncModePar = p;
	p->addStringValue("Sync Mode: Off");
	p->addStringValue("Sync Mode: Left");
	p->addStringValue("Sync Mode: Right");
	p->addStringValue("Sync Mode: L + R");
	p->setValueChangeCallback([this](double v) { screenScanner.setSync(v > 0.0); });
	p->setDefaultModParameters
	(
		-1.e+100,
		+1.e+100,
		-p->getMinValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		p->getMaxValue() / 10.0
	);

	p = new ModulatableParameter2("SyncNumCycles", 1, 100, 3, Parameter::LINEAR, 1);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { screenScanner.setNumCyclesShown(v); });
	p->setDefaultModParameters
	(
		-1.e+100,
		+1.e+100,
		-p->getMinValue(),
		+p->getMaxValue(),
		jura::ModulationConnection::modModes::ABSOLUTE,
		p->getMaxValue() / 10.0
	);

	//Framerate.saveAndRecall = false;
	//Pause.saveAndRecall = false;
	//FreezePoints.saveAndRecall = false;
	//ShowFPS.saveAndRecall = false;	
	//parClear.saveAndRecall = false;

	//ParameterSmoothing.stringConvertFunc = &valueToStringTotal5;
	//Framerate.stringConvertFunc = &valuetoFPS;
	//DecaySpeed.stringConvertFunc = &valuetoDecaySpeed;
	//PointsPerFrame.stringConvertFunc = &valuetoPPF;
	//SyncNumCycles.stringConvertFunc = &valueToString0;
	//TailFade.stringConvertFunc = &valueToString3;

	//DotBrightness.stringConvertFunc = &valueToString3;
	//DotSize.stringConvertFunc = &valueToString3;
	//DotBlur.stringConvertFunc = &valueToString3;

	//LineBrightness.stringConvertFunc = &valueToString3;
	//LineSize.stringConvertFunc = &valueToString3;
	//LineBlur.stringConvertFunc = &valueToString3;

	//LenFactor.stringConvertFunc = &valueToString3;

	//DotColorH.stringConvertFunc = &valueToString1;
	//DotColorH2.stringConvertFunc = &valueToString1;

	//LineColorH.stringConvertFunc = &valueToString1;
	//LineColorH2.stringConvertFunc = &valueToString1;
}

void PrettyScopeModule::setInitPatchValues()
{
	//LineColorMode.setValue(1);
	//DotColorMode.setValue(1);
	//Dots.setValue(0);
	//DotBrightness.setValue(.1);
	//DotSize.setValue(.5);
	//DotBlur.setValue(.5);
	//DotProfile.setValue(2);
	//drawOrder.setValue(0);
	//DotColorR.setValue(.22);
	//DotColorG.setValue(0);
	//DotColorB.setValue(0);
	//LineColorR.setValue(0.4);
	//LineColorG.setValue(0.5);
	//LineColorB.setValue(1.0);
	//DotColorR2.setValue(.22);
	//DotColorG2.setValue(0);
	//DotColorB2.setValue(0);
	//LineColorR2.setValue(0.4);
	//LineColorG2.setValue(0.5);
	//LineColorB2.setValue(1);
	//DotColorH.setValue(264);
	//DotColorH2.setValue(218);
	//LineColorH.setValue(120);
	//LineColorH2.setValue(0.0);
	//LineColorS.setValue(.99);
	//SyncMode.setValue(1);
	//OneDimZoom.setValue(0.95);
}

void PrettyScopeModule::setHelpTextAndLabels()
{
	//Framerate.text = "VSYNC On/Off";
	//FreezePoints.text = "FreezePts";
	//drawOrder.text = "LinesOnTop";
	//lenColorIntensity.text = "Color Shift";
	//DotColorR.text = DotColorR2.text = LineColorR.text = LineColorR2.text = "Red";
	//DotColorG.text = DotColorG2.text = LineColorG.text = LineColorG2.text = "Grn";
	//DotColorB.text = DotColorB2.text = LineColorB.text = LineColorB2.text = "Blu";
	//DotColorH.text = DotColorH2.text = LineColorH.text = LineColorH2.text = "Hue";
	//DotColorS.text = DotColorS2.text = LineColorS.text = LineColorS2.text = "Sat";
	//DotColorL.text = DotColorL2.text = LineColorL.text = LineColorL2.text = "Lit";
	//Zoom.text = "Zoom X/Y";
	//OneDimZoom.text = "Zoom";
	//OneDimOffset.text = "Offset";
	//OneDimensional.text = "1D Mode On/Off";
	//SyncNumCycles.text = "# Cycles";
	//FXMode.text = "Audio Effects On/Off";
	//ParameterSmoothing.text = "Smoothing";
	//ShowFPS.text = "fps";
}

void PrettyScopeModule::setSampleRate(double v)
{
	midiMaster.setSampleRate(v);

	sampleRate = v;
	screenScanner.setSampleRate(v);
	frameRateSmoother.setSampleRate(v);

	for (auto & obj : childModules)
		obj->setSampleRate(v);
}

void PrettyScopeModule::allNotesOff()
{
	midiMaster.allNotesOff();
}

void PrettyScopeModule::triggerAttack()
{
}

void PrettyScopeModule::triggerNoteOnLegato()
{
}

void PrettyScopeModule::triggerNoteOffLegato()
{
}

void PrettyScopeModule::triggerRelease()
{
}

void PrettyScopeModule::setBeatsPerMinute(double v)
{
	midiMaster.setBPM(v);
}


void PrettyScopeModule::setStateFromXml(const XmlElement& xmlState, const juce::String& stateName,
  bool markAsClean)
{
  BasicModule::setStateFromXml(xmlState, stateName, markAsClean);

  XmlElement* xmlColorMap = xmlState.getChildByName("ColorMap");
  if(xmlColorMap != nullptr)
    colorMap.setFromXml(*xmlColorMap);
}

XmlElement* PrettyScopeModule::getStateAsXml(const juce::String& stateName, bool markAsClean)
{
  XmlElement* xml = BasicModule::getStateAsXml(stateName, markAsClean);

  XmlElement* colorMapXml = colorMap.getAsXml();
  xml->addChildElement(colorMapXml);
  return xml;
}

double PrettyScopeModule::getScannerSaw(double in)
{
  return 2 * screenScanner.getSample(in) - 1;
}

void PrettyScopeModule::reset()
{
	AudioModuleWithMidiIn::reset();
	screenScanner.reset();
}

void PrettyScopeModule::processSampleFrame(double* x, double* y)
{
	const bool fxmode = fxModePar->getValue() != 0.0;
	const bool onedimensional = oneDimensionalPar->getValue() != 0.0;

	double xt = *x;
	double yt = *y;

	audioBuffer.processSampleFrame(&xt, &yt);

	if (fxmode)
	{
		xt = clamp(xt, -1.0, +1.0);
		yt = clamp(yt, -1.0, +1.0);
	}

	int ss = 0;
	if (onedimensional)
	{
		// for pitch analysis
		switch ((int)syncModePar->getValue())
		{
		case 0: ss = getScannerSaw(*x + *y); break;
		case 1: ss = getScannerSaw(*y); break;
		case 2: ss = getScannerSaw(*x); break;
		case 3: ss = getScannerSaw(*x + *y); break;
		}

		// store transformed point in circular buffer:
		audioBuffer.bufX[audioBuffer.bufIndex] = (float)ss;
		audioBuffer.bufY[audioBuffer.bufIndex] = (float)xt;
		audioBuffer.bufY2[audioBuffer.bufIndex] = (float)yt;
	}
	else
	{
		// store transformed point in circular buffer:
		audioBuffer.bufX[audioBuffer.bufIndex] = (float)xt;
		audioBuffer.bufY[audioBuffer.bufIndex] = (float)yt;
	}

	*x = xt;
	*y = yt;

	audioBuffer.updateStartIndex();
	frameRateSmoother.inc();
	midiMaster.incrementPitchGlide();
}

void PrettyScopeModule::processBlock(double **inOutBuffer, int numChannels, int numSamples)
{
	ScopedLock scopedLock(*lock);

	const bool doApplySmoothing = smoothingManager->needsSmoothing();

	const bool doApplyModulations = modulationManager.getNumConnections() != 0;

	if (BasicModule::isDemoTimedOut())
	{
		BasicModule::processBlockDemo(inOutBuffer, numChannels, numSamples);
		return;
	}

	const bool fxmode = fxModePar->getValue() != 0.0;

	for (int n = 0; n < numSamples; n++)
	{
		if (doApplySmoothing)
			smoothingManager->updateSmoothedValues();

		if (doApplyModulations)
			modulationManager.applyModulations();

		// retrieve input point:
		double x = inOutBuffer[0] ? inOutBuffer[0][n] : 0.0;
		double y = inOutBuffer[1] ? inOutBuffer[1][n] : 0.0;

		processSampleFrame(&x, &y);

		if (fxmode)
		{
			if (inOutBuffer[0]) { inOutBuffer[0][n] = x; }
			if (inOutBuffer[1]) { inOutBuffer[1][n] = y; }
		}
	}

	audioBuffer.processBlock(inOutBuffer, numChannels, numSamples);
}
