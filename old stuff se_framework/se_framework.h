/*******************************************************************************
 The block below describes the properties of this module, and is read by
 the Projucer to automatically generate project code that uses it.
 For details about the syntax and how to create or use a module, see the
 JUCE Module Format.txt file.


 BEGIN_JUCE_MODULE_DECLARATION

  ID:               se_framework
  vendor:           Soundemote
  version:          0.0.1
  name:             se_framework
  description:      framework for Soundemote software products
  website:          http://www.soundemote.com
  license:          for internal use only

	dependencies:     juce_core, juce_audio_basics, juce_graphics, juce_gui_basics,
						juce_audio_formats, juce_audio_processors, jura_framework,
						juce_opengl, jura_processors, elan_juce_helpers, rapt
  OSXFrameworks:
  iOSFrameworks:

 END_JUCE_MODULE_DECLARATION

*******************************************************************************/

#ifndef JucePlugin_Manufacturer
#define JucePlugin_Manufacturer "Soundemote"
#endif
#ifndef JucePlugin_ManufacturerWebsite
#define JucePlugin_ManufacturerWebsite "www.soundemote.com"
#endif
#ifndef JucePlugin_VersionString
#define JucePlugin_VersionString "1.0.0"
#endif

#ifndef SE_FRAMEWORK_H_INCLUDED
#define SE_FRAMEWORK_H_INCLUDED

#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_opengl/juce_opengl.h>
#include <jura_framework/jura_framework.h>
#include <jura_processors/jura_processors.h>

#include "licensing/se_PaywareAudioPlugIn.h"

/*
GUIDE TO VALUE RANGES

        Phase: 0 to TAU wrapped or -TAU to +TAU
        PhaseValue: -1 to 1 or 0 to 1 depending on if unipolar or bipolar, wrapped

        Value: Arbitrary, generally -1 to +1.
        Amplitude: signal or value of 0 to 1, -1 to 1, or -1 to 0, where negatives means inversion

        Degrees: 0 to 360 wrapped or -360 to 360 wrapped
        DegreesOfPhase: Degrees (0 to 360) converted to phase (0 to TAU)

        Sin/Cos params: If a function has _sin/_cos for paramers, use RAPT::elan::sinCos(input_TAU_range, &sin, &cos) to get coefficients
*/

#include "basic.h"

#include "GUIComponents/RectangleHelper.h"
#include "GUIComponents/TempBox.h"
#include "GUIComponents/Menu.h"
#include "GUIComponents/ParameterInfo.h"
#include "GUIComponents/FilmstripPainter.h"
#include "GUIComponents/VectorSlider.h"
#include "GUIComponents/ResourceBase.h"
#include "GUIComponents/ArcPainter.h"

#include "GUIComponents/MouseKeyboard.h"
#include "GUIComponents/Background.h"
#include "GUIComponents/SVGKnob.h"
#include "GUIComponents/TrianglePainter.h"

#include "SynthesizerComponents/utility/ParamSmoother.h"
#include "SynthesizerComponents/utility/PreciseTimer.h"

#include "SynthesizerComponents/utility/InternalModSrc.h"
#include "SynthesizerComponents/utility/MidiModSrc.h"
#include "SynthesizerComponents/utility/Oscilloscope.h"

#include "PluginComponents/myparams.h"
#include "SynthesizerComponents/MonoSynth.h"

#include "SynthesizerComponents/dynamic/StereoClipper.h"

#include "PluginComponents/MidiMaster.h"
#include "PluginComponents/BasicMidiPlugin.h"
#include "PluginComponents/BasicEditor.h"

#include "SynthesizerComponents/oscillator/NoiseGenerator.h"
#include "SynthesizerComponents/oscillator/waveshapes.h"
#include "SynthesizerComponents/oscillator/Phasor.h"
#include "SynthesizerComponents/oscillator/PolyBLEP.h"
#include "SynthesizerComponents/oscillator/XoxosOscillator.h"
#include "SynthesizerComponents/oscillator/BasicOscillator.h"

#include "SynthesizerComponents/envelope/LinearAttack.h"
#include "SynthesizerComponents/envelope/LinearADSR.h"
#include "SynthesizerComponents/envelope/rosic_EnvelopeGenerator_elanEdit.h"

#include "SynthesizerComponents/filter/StereoTwoPoleBandpass.h"

#include "JerobeamAlgorithms/MaxMSPHelper.h"
#include "JerobeamAlgorithms/JerobeamBlubb.h"
#include "JerobeamAlgorithms/JerobeamBoing.h"
#include "JerobeamAlgorithms/JerobeamKeplerBouwkamp.h"
#include "JerobeamAlgorithms/JerobeamMushroom.h"
#include "JerobeamAlgorithms/JerobeamNyquistShannon.h"
#include "JerobeamAlgorithms/JerobeamRadar.h"
#include "JerobeamAlgorithms/JerobeamSpiral.h"
#include "JerobeamAlgorithms/JerobeamTorus.h"
#include "JerobeamAlgorithms/JerobeamWirdoSpiral.h"

#include "SynthesizerComponents/filter/Equalizer.h"

#include "Modules/Chaosfly/ChaosflyCore.h"
#include "Modules/Chaosfly/ChaosflyModule.h"
#include "Modules/Chaosfly/ChaosflyEditor.h"

#include "Modules/XoxosOscillatorPlugin/XoxosOscillatorCore.h"
//#include "Modules/XoxosOscillatorPlugin/XoxosOscillatorModule.h"
//#include "Modules/XoxosOscillatorPlugin/XoxosOscillatorEditor.h"

#include "Modules/BasicEnvelopePlugin/BasicEnvelopeModule.h"
#include "Modules/BasicEnvelopePlugin/BasicEnvelopeEditor.h"

#include "Modules/BasicOscillatorPlugin/BasicOscillatorCore.h"
#include "Modules/BasicOscillatorPlugin/BasicOscillatorModule.h"
//#include "Modules/BasicOscillatorPlugin/BasicOscillatorEditor.h"

#include "Modules/MidiModSourcePlugin/MidiModSourceModule.h"
//#include "Modules/MidiModSourcePlugin/MidiModSourceEditor.h"

#include "Modules/Torus Generator/JerobeamTorusCore.h"
//#include "Modules/Torus Generator/JerobeamTorusModule.h"
//#include "Modules/Torus Generator/JerobeamTorusEditor.h"

#include "Modules/OMS/JerobeamRadarCore.h"
//#include "Modules/OMS/OMSModule.h"
//#include "Modules/OMS/OMSEditor.h"

#include "Modules/FlowerChildFilter/FlowerChildFilterCore.h"
//#include "Modules/FlowerChildFilter/FlowerChildFilterModule.h"
//#include "Modules/FlowerChildFilter/FlowerChildFilterEditor.h"

#endif
