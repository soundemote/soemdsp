#pragma once

using namespace jura;

namespace OscilloscopeGradientsXml
{
extern const char*   AnalogGreen_xml;
const int            AnalogGreen_xmlSize = 90;

extern const char*   AnalogRed_xml;
const int            AnalogRed_xmlSize = 90;

extern const char*   Blackout_xml;
const int            Blackout_xmlSize = 90;

extern const char*   BlueCream_xml;
const int            BlueCream_xmlSize = 184;

extern const char*   FireIce_xml;
const int            FireIce_xmlSize = 160;

extern const char*   Hot_xml;
const int            Hot_xmlSize = 202;

extern const char*   LightBlue_xml;
const int            LightBlue_xmlSize = 90;

extern const char*   LightGreen_xml;
const int            LightGreen_xmlSize = 90;

extern const char*   PrettyPink_xml;
const int            PrettyPink_xmlSize = 109;

extern const char*   Rainbow_xml;
const int            Rainbow_xmlSize = 154;

extern const char*   Whiteout_xml;
const int            Whiteout_xmlSize = 90;

// Points to the start of a list of resource names.
extern const char* namedResourceList[];

// Number of elements in the namedResourceList array.
const int namedResourceListSize = 11;

// If you provide the name of one of the binary resource variables above, this function will
// return the corresponding data and its size (or a null pointer if the name isn't found).
const char* getNamedResource(const char* resourceNameUTF8, int& dataSizeInBytes) throw();
}

class JUCE_API OscilloscopeModule : public jura::ModulatableAudioModule, public jura::ImageUpdater, public ChangeBroadcaster
{
public:

  OscilloscopeModule(juce::CriticalSection * lockToUse,
		jura::MetaParameterManager * metaManagerToUse = nullptr,
		jura::ModulationManager * modManagerToUse = nullptr);

  virtual ~OscilloscopeModule();

  virtual void createParameters();

  /** Sets the desired pixel size for the display. This function will update the internal pixel 
  size of the buffer according to the passed values and the desired rescaling factor. " */
  void setDisplayPixelSize(int width, int height);
	void setZoom(float v) { zoom = v; }

	bool getIsBypassed() { return isBypassed; }

	void setSync(bool v) { buffer.setSyncMode(v); }
	void setNumCycles(int v) { buffer.screenScanner.setNumCyclesShown(v); }
	void setOneDimensional(bool v) { buffer.setOneDimensionalMode(v); }

  // inquiry functions:
  const ColorMap& getColorMap() { return colorMap; }
  LoadableColorMap* getColorMapPointer() { return &colorMap; }

  // overriden from AudioModule baseclass:
  virtual AudioModuleEditor *createEditor(int type = 0) override;
	double getSample(double v)
	{
		double signal = v * 0.5;
		processStereoFrame(&signal, &signal);
		return v;
	}
  virtual void processBlock(double **inOutBuffer, int numChannels, int numSamples) override;
  virtual void processStereoFrame(double *left, double *right) override;
  virtual void setSampleRate(double newSampleRate) override; 
  virtual void reset() override;
  virtual void setStateFromXml(const XmlElement& xmlState, const juce::String& stateName, 
    bool markAsClean) override;
  virtual XmlElement* getStateAsXml(const juce::String& stateName, bool markAsClean) override;

	struct preset
	{
		String name;
		String xml;
	};

	vector<preset> Presets
	{
		{ "AnalogGreen", OscilloscopeGradientsXml::AnalogGreen_xml },
		{ "AnalogRed", OscilloscopeGradientsXml::AnalogRed_xml },
		{ "Hot", OscilloscopeGradientsXml::Hot_xml },
		{ "BlueCream", OscilloscopeGradientsXml::BlueCream_xml },
		{ "PrettyPink", OscilloscopeGradientsXml::PrettyPink_xml },
		{ "LightBlue", OscilloscopeGradientsXml::LightBlue_xml },
		{ "LightGreen", OscilloscopeGradientsXml::LightGreen_xml },
		{ "Whiteout", OscilloscopeGradientsXml::Whiteout_xml },
		{ "Blackout", OscilloscopeGradientsXml::Blackout_xml },
		{ "Rainbow", OscilloscopeGradientsXml::Rainbow_xml },
		{ "FireIce", OscilloscopeGradientsXml::FireIce_xml },
	};

	// this object is reponsible for drawing the incoming data onto a virtual screen:
	RAPT::rsPhaseScopeBuffer<double, float, double> buffer;

protected:

  /** Updates the size of the internal buffer according to the settings of displayWidth, 
  displayHeight and pixelScale. */
  void updateBufferSize();

  /** Updates the image for the scope picture, i.e. writes/converts the content of the 
  phaseScopeBuffer member into the image member. */
  void updateScopeImage();

  /** Updates our repaint triggering interval - needs to be called whenever frame rate or sample
  rate changes. It's the number of samples between two frames. */
  void updateRepaintInterval();

  double pixelScale = 1; // scale factor between internal and external pixel sizes of the image
  int displayWidth = 100; // display width in pixels
  int displayHeight = 100; // display height in pixels

	bool bypassPixelDecay = false;

  int repaintIntervalInSamples = 0;
  int repaintCounter = 0;

  juce::Image image;
  jura::LoadableColorMap colorMap;

	bool isBypassed = false;
	float zoom = 1;

  friend class ScopeDisplay;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OscilloscopeModule)
};

//=================================================================================================

class JUCE_API ScopeDisplay : public Component, public ImageUpdateListener,	public ChangeListener, public ChangeBroadcaster
{

public:

	ScopeDisplay(OscilloscopeModule * newPhaseScopeToEdit);
	virtual ~ScopeDisplay();

	virtual void resized() override;
	virtual void paint(Graphics &g)	override;
	virtual void imageWasUpdated(juce::Image* image) override;
	virtual void changeListenerCallback(ChangeBroadcaster *source) override;

protected:

	OscilloscopeModule * phaseScope = nullptr;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ScopeDisplay)
};

//=================================================================================================

class JUCE_API OscilloscopeEditor : public jura::AudioModuleEditor
{

public:

  OscilloscopeEditor(OscilloscopeModule* newOscilloscopeToEdit);
	virtual ~OscilloscopeEditor();

	void paint(Graphics& g) override
	{
		g.setColour(backgroundColor);
		g.fillRect(getLocalBounds());
	}

	void paintOverChildren(Graphics&) override {}
  void createWidgets();
  void resized() override;

	Colour backgroundColor{ 0,0,0 };

	void changeListenerCallback(ChangeBroadcaster* /*source*/) override
	{
		/*
		wait a minute, shouldn't the individual parameter, not the module, be a change broadcaster?
		otherwise, we have to update EVERYTHING for anything sending a change message.
		*/
		backgroundColor = scope->getColorMapPointer()->getAsColourGradient().getColourAtPosition(0);

		display.setVisible(!scope->getIsBypassed());

		if (doHideAllControls)
			sliderNumCycles.setVisible(false);
		else
			sliderNumCycles.setVisible(scope->getParameterByName("OneDimensional")->getValue() > 0.5);

		repaint();
	}

	void setWidgetsToHideForMouseover(bool isVisible)
	{
		bool setvisible = isVisible && !doHideAllControls;

		sliderBrightness.setVisible(setvisible);
		sliderAfterglow.setVisible(setvisible);
		sliderDotLimit.setVisible(setvisible);
		sliderZoom.setVisible(setvisible);
		button1D.setVisible(setvisible);
		comboGradientSelection.setVisible(setvisible);
		sliderScanFreq.setVisible(setvisible);
		sliderNumCycles.setVisible(setvisible);

		if (doHideAllControls)
			sliderNumCycles.setVisible(false);
		else
			sliderNumCycles.setVisible(scope->getParameterByName("OneDimensional")->getValue() > 0.5);

		buttonBypass.setVisible(isVisible);
	}

	void setHideAllControls(bool v)
	{
		doHideAllControls = v;
	}

protected:

  OscilloscopeModule * scope = nullptr;
	ScopeDisplay display;
  int widgetMargin = 0;
	bool doHideAllControls = false;

	rsModulatableSlider sliderBrightness, sliderAfterglow, sliderDotLimit, sliderZoom;

	ElanModulatableButton button1D, buttonSync, buttonBypass;

	ElanModulatableComboBox comboGradientSelection;

	rsModulatableSlider sliderScanFreq, sliderNumCycles;


  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OscilloscopeEditor)
};
