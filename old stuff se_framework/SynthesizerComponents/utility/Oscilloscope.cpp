#include "Oscilloscope.h"

using namespace jura;

using jura::Parameter;

OscilloscopeModule::OscilloscopeModule(juce::CriticalSection * lockToUse,
	jura::MetaParameterManager * metaManagerToUse,
	jura::ModulationManager * modManagerToUse) : jura::ModulatableAudioModule(lockToUse, metaManagerToUse, modManagerToUse)
{
  ScopedLock scopedLock(*lock);

  setModuleTypeName("Scope");

  pixelScale = 1.0;
	displayWidth = 100;
  displayHeight = 100;
  bypassPixelDecay = false;
  buffer.setMaxSizeWithoutReAllocation(1000, 1000);
  updateBufferSize();

  createParameters();
  reset();
}

OscilloscopeModule::~OscilloscopeModule()
{
}

void OscilloscopeModule::createParameters()
{
  ScopedLock scopedLock(*lock);	

  Parameter* p;

	p = new ModulatableParameter2("Bypass", 0, 1, 0, Parameter::scalings::LINEAR);
	p->setValueChangeCallback([this](double v)
	{
		isBypassed = v > 0.5;
		sendChangeMessage();
	});
	p->setSaveAndRecall(false);
	addObservedParameter(p);

	auto mp = new ModulatableParameter2("Zoom", -12, 12, 0, Parameter::LINEAR);
	mp->setValueChangeCallback([this](double v) { setZoom((float)elan::dbToAmp(v)); });
	addObservedParameter(mp);

  mp = new ModulatableParameter2("Brightness", .001, 100, 3.4, Parameter::EXPONENTIAL);
	//mp->setValueChangeCallback([this](double v) { buffer.setBrightness((float)v); });
	addObservedParameter(mp);

  p = new ModulatableParameter2("AfterGlow", .001, 50, 0.02, Parameter::EXPONENTIAL);
	p->setValueChangeCallback([this](double v)
	{
		buffer.setDecayTime(v);
		bypassPixelDecay = v >= 50;
	});
	addObservedParameter(p);

  p = new Parameter("PixelSpread", 0, 1, .5, Parameter::LINEAR);
  p->setValueChangeCallback([this](double v) { buffer.setPixelSpread((float)v);	});
	addObservedParameter(p);

  p = new Parameter("PixelScale", 1, 8, 1, Parameter::EXPONENTIAL);
  p->setValueChangeCallback([this](double v)
	{  
		jassert(v >= 1.0);
		pixelScale = v;
		updateBufferSize();
	});
	addObservedParameter(p);

  p = new Parameter("LineDensity", 0, 1, 1, Parameter::LINEAR);
  p->setValueChangeCallback([this](double v) { buffer.setLineDensity((float)v); });
	addObservedParameter(p);

  p = new Parameter("DotLimit", 1, 500, 2, Parameter::EXPONENTIAL, 1);
	p->setValueChangeCallback([this](double v) { buffer.setLineDensityLimit((int)v); });
	addObservedParameter(p);

  p = new Parameter("FrameRate", 1, 100, 60, Parameter::EXPONENTIAL);
  p->setValueChangeCallback([this](double v)
	{
		buffer.setFrameRate(v);
		updateRepaintInterval();
	});
	addObservedParameter(p);

  p = new Parameter("AntiAlias", 0, 1, 1, Parameter::BOOLEAN);
	p->setValueChangeCallback([this](double v) { buffer.setAntiAlias(v >= 0.5); });
	addObservedParameter(p);

  p = new ModulatableParameter2("OneDimensional", 0, 1, 0, Parameter::BOOLEAN);
	p->setValueChangeCallback([this](double v) { buffer.setOneDimensionalMode(v >= 0.5); sendChangeMessage(); });
	p->setValue(1, true, true);
	p->setValue(0, true, true);
	addObservedParameter(p);

  p = new Parameter("ScanFrequency", 0.1, 10, 1, Parameter::EXPONENTIAL);
	p->setValueChangeCallback([this](double v) { buffer.setScanningFrequency(v); });
	addObservedParameter(p);

  p = new Parameter("NumCycles", 1, 10, 2, Parameter::LINEAR);
  p->setValueChangeCallback([this](double v) { buffer.screenScanner.setNumCyclesShown((int)v); });
	addObservedParameter(p);

  p = new Parameter("Sync", 0, 1, 1, Parameter::BOOLEAN);
	p->setValueChangeCallback([this](double v) { buffer.setSyncMode(v >= 0.5); });
	addObservedParameter(p);

	p = new Parameter("GradientSelection", 0, 1, 0, Parameter::STRING);

	for (auto & obj : Presets)
		p->addStringValue("Color: " + obj.name);

	p->setValueChangeCallback([this](double v)
	{
		juce::XmlDocument myDocument(Presets[(size_t)v].xml);
		std::unique_ptr<XmlElement> mainElement(myDocument.getDocumentElement());

		if (mainElement == nullptr)
		{
			jassertfalse;
			String error = myDocument.getLastParseError();
		}
		else
		{
			colorMap.setStateFromXml(*mainElement, "ColorMap", false);
			sendChangeMessage(); // get the new backgroundImage color for Editor
		}
	});

	addObservedParameter(p);
}

void OscilloscopeModule::setDisplayPixelSize(int width, int height)
{
  ScopedLock scopedLock(*lock);
  displayWidth  = width;
  displayHeight = height; 
  updateBufferSize();
}

AudioModuleEditor* OscilloscopeModule::createEditor(int)
{
  return new OscilloscopeEditor(this);
}

void OscilloscopeModule::processBlock(double **inOutBuffer, int numChannels, int numSamples)
{
  jassert(numChannels == 2);
  for(int n = 0; n < numSamples; n++)
    processStereoFrame(&inOutBuffer[0][n], &inOutBuffer[1][n]);
}

void OscilloscopeModule::processStereoFrame(double *left, double *right)
{
	if (isBypassed)
		return;

	float l = float(*left) * zoom;
	float r = float(*right) * zoom;

  buffer.processSampleFrame(l, r);
  repaintCounter++;
  if(repaintCounter > repaintIntervalInSamples)
  {
    updateScopeImage();
    sendImageUpdateNotification(&image);
    repaintCounter = 0;
  }
}

void OscilloscopeModule::setSampleRate(double newSampleRate)
{
  ScopedLock scopedLock(*lock);
	buffer.setSampleRate(newSampleRate);
  updateRepaintInterval();
}

void OscilloscopeModule::reset()
{
  ScopedLock scopedLock(*lock);
	buffer.reset();
  repaintCounter = 0;
}

void OscilloscopeModule::setStateFromXml(const XmlElement& xmlState, const juce::String& stateName,
  bool markAsClean)
{
  AudioModule::setStateFromXml(xmlState, stateName, markAsClean);
 auto xmlColorMap = xmlState.getChildByName("ColorMap");
  if(xmlColorMap != nullptr)
    colorMap.setFromXml(*xmlColorMap);
}

XmlElement* OscilloscopeModule::getStateAsXml(const juce::String& stateName, bool markAsClean)
{
  XmlElement* xml = AudioModule::getStateAsXml(stateName, markAsClean);
  XmlElement* colorMapXml = colorMap.getAsXml();
  xml->addChildElement(colorMapXml);
  return xml;
}

void OscilloscopeModule::updateBufferSize()
{
  ScopedLock scopedLock(*lock);
  int w = (int) ::round(displayWidth  / pixelScale);
  int h = (int) ::round(displayHeight / pixelScale);
  w = jmax(w, 1);
  h = jmax(h, 1);

  jassert(w <= buffer.getImage()->getMaxWidth());
  jassert(h <= buffer.getImage()->getMaxHeight());

	buffer.setSize(w, h);
  image = juce::Image(juce::Image::ARGB, w, h, false);
}

void OscilloscopeModule::updateScopeImage()
{
	normalizedDataToImage(buffer.getImage()->getPixelPointer(0, 0), image, colorMap);
	if (!bypassPixelDecay)
		buffer.applyPixelDecay();
}

void OscilloscopeModule::updateRepaintInterval()
{
  repaintIntervalInSamples = 
    (int) ::round(buffer.getSampleRate() / buffer.getFrameRate());
  repaintCounter = 0;
}

//=================================================================================================

ScopeDisplay::ScopeDisplay(OscilloscopeModule * newPhaseScopeToEdit)
{
	phaseScope = newPhaseScopeToEdit;
	phaseScope->addImageUpdateListener(this);
	addChangeListener(this);
}

ScopeDisplay::~ScopeDisplay()
{
	phaseScope->removeImageUpdateListener(this);
}

void ScopeDisplay::resized()
{
	auto b = getBoundsInParent();
	phaseScope->setDisplayPixelSize(getWidth(), getHeight());
}

void ScopeDisplay::paint(Graphics &g)
{
	auto b = getBoundsInParent();

	g.fillAll(Colours::red.withAlpha(0.3f));
	g.setImageResamplingQuality(Graphics::lowResamplingQuality);
	g.drawImage(phaseScope->image, getLocalBounds().toFloat());
}

void ScopeDisplay::imageWasUpdated(juce::Image*)
{
	sendChangeMessage();
	// We will receive the change message ourselves and when we do, we will trigger a repaint. We 
	// can't directly call repaint here because then we hit an assertion which says we should acquire
	// a MessageManagerLock - but when we do so, it becomes unresponsive.
}

void ScopeDisplay::changeListenerCallback(ChangeBroadcaster*)
{
	repaint();
}

//=================================================================================================

OscilloscopeEditor::OscilloscopeEditor(OscilloscopeModule* newOscilloscopeToEdit)
	: AudioModuleEditor(newOscilloscopeToEdit)
	, display(newOscilloscopeToEdit)
{
	ScopedLock scopedLock(*lock);
	scope = newOscilloscopeToEdit;
	widgetMargin = 150;

	scope->addChangeListener(this);
	addMouseListener(&display, true);

	addAndMakeVisible(display);
	createWidgets();

	int headerMargin = 26;  // this is the height we need for headline and preset-section
	int displaySize = 250;
	setSize(displaySize+widgetMargin, displaySize+headerMargin);

	setWidgetsToHideForMouseover(false);
}

OscilloscopeEditor::~OscilloscopeEditor()
{
	scope->removeChangeListener(this);
}

void OscilloscopeEditor::createWidgets()
{
	addWidget(&buttonBypass);
	buttonBypass.assignParameter(scope->getParameterByName("Bypass"));
	buttonBypass.setButtonText(buttonBypass.getParameterName());

  addWidget(&sliderBrightness);
	sliderBrightness.assignParameter( scope->getParameterByName("Brightness") );
	sliderBrightness.setStringConversionFunction(&valueToString3);
	sliderBrightness.setName(sliderBrightness.getParameterName());

  addWidget(&sliderAfterglow);
	sliderAfterglow.assignParameter( scope->getParameterByName("AfterGlow") );
	sliderAfterglow.setStringConversionFunction(&elan::afterGlowToString);

	addWidget(&sliderDotLimit);
	sliderDotLimit.assignParameter(scope->getParameterByName("DotLimit"));
	sliderDotLimit.setStringConversionFunction(&elan::dotLimitToString);

	addWidget(&sliderZoom);
	sliderZoom.assignParameter(scope->getParameterByName("Zoom"));
	sliderZoom.setStringConversionFunction(&elan::decibelsToStringWithUnit1);
	sliderZoom.setSliderName("Z");

  addWidget(&button1D);
	button1D.assignParameter( scope->getParameterByName("OneDimensional") );
	button1D.setButtonText("1D");

  addWidget(&sliderScanFreq);
	sliderScanFreq.assignParameter( scope->getParameterByName("ScanFrequency") );
	sliderScanFreq.setStringConversionFunction(&valueToString3);

  addWidget(&sliderNumCycles);
	sliderNumCycles.assignParameter( scope->getParameterByName("NumCycles") );
	sliderNumCycles.setStringConversionFunction(&valueToString0);

	addWidget(&comboGradientSelection);
	comboGradientSelection.assignParameter(scope->getParameterByName("GradientSelection"));
	comboGradientSelection.setDescriptionField(infoField);	
	
	sliderNumCycles.setName(sliderNumCycles.getParameterName());

	for (auto & obj : widgets)
		obj->setDescriptionField(infoField);

	buttonBypass.setDescription("Bypass oscilloscope to save CPU.");
	button1D.setDescription("Enable 1D waveform view");
	sliderNumCycles.setDescription("Number of cycles shown for 1D waveform view");
	comboGradientSelection.setDescription("Change the color gradient used for waveform display");
	sliderBrightness.setDescription("Brightness of waveform traces.");
	sliderAfterglow.setDescription("How long it takes for traces to decay in brightness.");
	sliderDotLimit.setDescription("Sets number of dots drawn per line segment. Keep low to save on CPU or set to 1 for \"dots only\" mode.");
	sliderZoom.setDescription("Zooms the oscilloscope in or out based on decibels");

	buttonBypass.setPainter(&buttonEnableColorRed);
	button1D.setPainter(&buttonEnableColorGreen);
}

void OscilloscopeEditor::resized()
{
	int squareWidthHeight = jmin(getWidth(), getHeight());
	int x_offset = (getWidth() - squareWidthHeight)/2;
	display.setBounds(x_offset, 0, squareWidthHeight, squareWidthHeight);

	const int margin = 4;
	const int sliderHeight = 16;

	{ // top row
		int x = margin;
		int y = margin;
		int w = buttonBypass.getFont()->getTextPixelWidth(buttonBypass.getButtonText(), buttonBypass.getFont()->getDefaultKerning()) + margin * 2;
		int h = sliderHeight;

		buttonBypass.setBounds(x, y, w, h);

		x = buttonBypass.getRight() + margin;

		sliderZoom.setBounds(x, y, 64, h);

		x = sliderZoom.getRight() + margin;
		w = getWidth() - x - margin;

		comboGradientSelection.setBounds(x, y, w, h);
	}	

	{ // bottom rows
		int y = getHeight();

		y -= sliderHeight + margin;

		{ // 1d mode controls bottom first row going up
			int x = margin;			
			int w = getWidth()/2;
			int h = sliderHeight;			

			w = button1D.getFont()->getTextPixelWidth(button1D.getButtonText(), buttonBypass.getFont()->getDefaultKerning()) + margin * 2;
			button1D.setBounds(x, y, w, h);

			x = button1D.getRight() + margin;
			w = getWidth()/2 - x - margin;

			sliderNumCycles.setBounds(x, y, w, h);

			x = sliderNumCycles.getRight() + margin*2;
			w = getWidth()/2;

			sliderDotLimit.setBounds(x, y, w - margin*2, h);
		}

		y -= sliderHeight + margin;

		{ // brightness & glow bottom second row going up
			int x = margin;
			int w = getWidth()/2;
			int h = sliderHeight;

			sliderBrightness.setBounds(x, y, w - margin*2, h);

			x += w;

			sliderAfterglow.setBounds(x, y, w - margin*2, h);
		}
	}
}

namespace OscilloscopeGradientsXml
{

//================== AnalogGreen.xml ==================
static const unsigned char temp_binary_data_0[] =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n"
"\r\n"
"<ColorMap \r\n"
"FF000000=\"0.0\"\r\n"
"FF00FF00=\"1.0\"\r\n"
"/>\r\n";

const char* AnalogGreen_xml = (const char*)temp_binary_data_0;

//================== AnalogRed.xml ==================
static const unsigned char temp_binary_data_1[] =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n"
"\r\n"
"<ColorMap \r\n"
"FF000000=\"0.0\"\r\n"
"FFFF0000=\"1.0\"\r\n"
"/>\r\n";

const char* AnalogRed_xml = (const char*)temp_binary_data_1;

//================== Blackout.xml ==================
static const unsigned char temp_binary_data_2[] =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n"
"\r\n"
"<ColorMap \r\n"
"FF000000=\"0.0\"\r\n"
"FFFFFFFF=\"1.0\"\r\n"
"/>\r\n";

const char* Blackout_xml = (const char*)temp_binary_data_2;

//================== BlueCream.xml ==================
static const unsigned char temp_binary_data_3[] =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n"
"\r\n"
"<ColorMap \r\n"
"FF000000=\"0.000\"\r\n"
"FF004A8A=\"0.167\"\r\n"
"FF0093B9=\"0.333\"\r\n"
"FF5FB59C=\"0.500\"\r\n"
"FFB3DC77=\"0.667\"\r\n"
"FFF3FA82=\"0.833\"\r\n"
"FFFFFFFF=\"1.000\"\r\n"
"/>\r\n";

const char* BlueCream_xml = (const char*)temp_binary_data_3;

//================== FireIce.xml ==================
static const unsigned char temp_binary_data_4[] =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n"
"\r\n"
"<ColorMap \r\n"
"FF000000=\"0\"\r\n"
"FFFFFF00=\"0.167\"\r\n"
"FFFF0000=\"0.333\"\r\n"
"FF0000FF=\"0.667\"\r\n"
"FF00FFFF=\"0.833\"\r\n"
"FFFFFFFF=\"1.0\"\r\n"
"/>\r\n";

const char* FireIce_xml = (const char*)temp_binary_data_4;

//================== Hot.xml ==================
static const unsigned char temp_binary_data_5[] =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n"
"\r\n"
"<ColorMap \r\n"
"ff000000=\"0.000\"\r\n"
"ff7f0000=\"0.143\"\r\n"
"ffb30000=\"0.286\"\r\n"
"ffe63300=\"0.429\"\r\n"
"fff8782b=\"0.571\"\r\n"
"ffffc080=\"0.714\"\r\n"
"ffffe6cc=\"0.857\"\r\n"
"ffffffff=\"1.000\"\r\n"
"/>\r\n";

const char* Hot_xml = (const char*)temp_binary_data_5;

//================== LightBlue.xml ==================
static const unsigned char temp_binary_data_6[] =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n"
"\r\n"
"<ColorMap \r\n"
"FFFFFFFF=\"0.0\"\r\n"
"FF0000FF=\"1.0\"\r\n"
"/>\r\n";

const char* LightBlue_xml = (const char*)temp_binary_data_6;

//================== LightGreen.xml ==================
static const unsigned char temp_binary_data_7[] =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n"
"\r\n"
"<ColorMap \r\n"
"FFEDFFFC=\"0.0\"\r\n"
"FF11C311=\"1.0\"\r\n"
"/>\r\n";

const char* LightGreen_xml = (const char*)temp_binary_data_7;

//================== PrettyPink.xml ==================
static const unsigned char temp_binary_data_8[] =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n"
"\r\n"
"<ColorMap ff000230=\"0\" ff6500ad=\"0.3\" ffff00c8=\"0.5\" ffffc7e5=\"1\"/>";

const char* PrettyPink_xml = (const char*)temp_binary_data_8;

//================== Rainbow.xml ==================
static const unsigned char temp_binary_data_9[] =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n"
"\r\n"
"<ColorMap \r\n"
"FF000000=\"0.0\"\r\n"
"FF0000FF=\"0.2\"\r\n"
"FF00FFFF=\"0.4\"\r\n"
"FF00FF00=\"0.6\"\r\n"
"FFFFFF00=\"0.8\"\r\n"
"FFFF0000=\"1.0\"\r\n"
"/>\r\n";

const char* Rainbow_xml = (const char*)temp_binary_data_9;

//================== Whiteout.xml ==================
static const unsigned char temp_binary_data_10[] =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n"
"\r\n"
"<ColorMap \r\n"
"FFFFFFFF=\"0.0\"\r\n"
"FF000000=\"1.0\"\r\n"
"/>\r\n";

const char* Whiteout_xml = (const char*)temp_binary_data_10;


const char* getNamedResource(const char*, int&) throw();
const char* getNamedResource(const char* resourceNameUTF8, int& numBytes) throw()
{
	unsigned int hash = 0;
	if (resourceNameUTF8 != 0)
		while (*resourceNameUTF8 != 0)
			hash = 31 * hash + (unsigned int)*resourceNameUTF8++;

	switch (hash)
	{
	case 0x47fa91ab:  numBytes = 90; return AnalogGreen_xml;
	case 0x1048f839:  numBytes = 90; return AnalogRed_xml;
	case 0xdbbaeee7:  numBytes = 90; return Blackout_xml;
	case 0xc4f04380:  numBytes = 184; return BlueCream_xml;
	case 0xaf4b592d:  numBytes = 160; return FireIce_xml;
	case 0xa4ba3d05:  numBytes = 202; return Hot_xml;
	case 0x3230c5c8:  numBytes = 90; return LightBlue_xml;
	case 0x1ae9a485:  numBytes = 90; return LightGreen_xml;
	case 0xfc911884:  numBytes = 109; return PrettyPink_xml;
	case 0xb82e75ee:  numBytes = 154; return Rainbow_xml;
	case 0x354a8f7d:  numBytes = 90; return Whiteout_xml;
	default: break;
	}

	numBytes = 0;
	return 0;
}

const char* namedResourceList[] =
{
	"AnalogGreen_xml",
	"AnalogRed_xml",
	"Blackout_xml",
	"BlueCream_xml",
	"FireIce_xml",
	"Hot_xml",
	"LightBlue_xml",
	"LightGreen_xml",
	"PrettyPink_xml",
	"Rainbow_xml",
	"Whiteout_xml"
};

}
