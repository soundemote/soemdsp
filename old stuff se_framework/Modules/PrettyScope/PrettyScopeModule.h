#pragma once

#include <map>

#pragma warning(disable: 4267) // size_t to GLsizei
#pragma warning(disable: 4244) // size_t to int

using namespace juce;
using std::vector;

class JUCE_API PrettyScopeModule : public BasicModule, public MidiSlave
{
public:
  PrettyScopeModule(CriticalSection *lockToUse, jura::MetaParameterManager * metaManagerToUse);
	virtual ~PrettyScopeModule() = default;

	// plugin
	jura::AudioModuleEditor* createEditor(int type = 0) override;
	void createParameters() override;
	void setInitPatchValues() override;
	void setHelpTextAndLabels() override;

	virtual void setStateFromXml(const XmlElement& xmlState, const juce::String& stateName, bool markAsClean) override;
	virtual XmlElement* getStateAsXml(const juce::String& stateName, bool markAsClean) override;

	// host
	virtual void reset() override;
  virtual void setSampleRate(double newSampleRate) override;
	void setBeatsPerMinute(double v) override;
  virtual void processBlock(double **inOutBuffer, int numChannels, int numSamples) override;
	virtual void processSampleFrame(double* left, double* right) override;

	// midi
	void allNotesOff() override;

	virtual void handleMidiMessage(MidiMessage message) override
	{
		ScopedLock scopedLock(*lock);

		midiMaster.handleMidiMessage(message);
	}
	void triggerAttack();
	void triggerNoteOnLegato();
	void triggerNoteOffLegato();
	void triggerRelease();

	// inquiry:
	jura::LoadableColorMap* getColorMapPointer() { return &colorMap; }

	// brushes
	std::vector<Brush*> brushes;
	LineBrush line1, line2;
	DotBrush dot1, dot2;

	// Smmooth out framerate text numbers
	ParamSmoother frameRateSmoother;

	// for automation of clearing canvas
	bool doClearCanvas;
	bool canvasWasCleared;

	float tailFadeFactor = 1.0;

	// creates sawtooth scanning signal in 1D mode
	RAPT::rsScopeScreenScanner<double> screenScanner;

	ScopeAudioBuffer audioBuffer;

	ModulationManager modulationManager;
	MidiMaster midiMaster;

	vector<jura::rsSmoothableParameter*> parametersForSmoothing;

protected:

	ModulatableParameter2* syncModePar = nullptr;
	ModulatableParameter2* fxModePar = nullptr;
	ModulatableParameter2* oneDimensionalPar = nullptr;

  jura::LoadableColorMap colorMap;

  double sampleRate  = 44100;

	/** Returns the value of the screen scanner sawtooth wave used in 1D mode. You should pass an
	input signal that will be used for analysis in sync mode. */
	double getScannerSaw(double in);

  // transform matrix coefficients:
  double Axx, Axy, Ayx, Ayy;

  friend class OpenGLCanvas;
  friend class PrettyScopeEditor;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PrettyScopeModule)
};
