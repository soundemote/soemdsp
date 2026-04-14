#ifdef  SE_FRAMEWORK_H_INCLUDED
 /* When you add this cpp file to your project, you mustn't include it in a file where you've
    already included any other headers - just put it inside a file on its own, possibly with your config
    flags preceding it, but don't include anything else. That also includes avoiding any automatic prefix
    header files that the compiler may be using.
 */
 #error "Incorrect use of JUCE cpp file"
#endif

#include "se_framework.h"

// todo: clean up directory structure (an the adapt KeyGenerator project - should use the
// se_framework juce module also instead of including the keygenerator itself):

#include "licensing/keygen/PrimeNumbers.cpp"
#include "licensing/keygen/RandomNumberGenerator01.cpp"
#include "licensing/keygen/RandomNumberGenerator02.cpp"
#include "licensing/keygen/KeyGenerator.cpp"
#include "licensing/se_PaywareAudioPlugIn.cpp"

#include "basic.cpp"

#include "GUIComponents/RectangleHelper.cpp"
#include "GUIComponents/TempBox.cpp"
#include "GUIComponents/Menu.cpp"
#include "GUIComponents/ParameterInfo.cpp"
#include "GUIComponents/FilmstripPainter.cpp"
#include "GUIComponents/VectorSlider.cpp"
#include "GUIComponents/ResourceBase.cpp"
#include "GUIComponents/ArcPainter.cpp"

#include "GUIComponents/MouseKeyboard.cpp"
#include "GUIComponents/Background.cpp"
#include "GUIComponents/SVGKnob.cpp"
#include "GUIComponents/TrianglePainter.cpp"

#include "SynthesizerComponents/utility/ParamSmoother.cpp"
#include "SynthesizerComponents/utility/PreciseTimer.cpp"

#include "SynthesizerComponents/utility/InternalModSrc.cpp"
#include "SynthesizerComponents/utility/MidiModSrc.cpp"
#include "SynthesizerComponents/utility/Oscilloscope.cpp"

#include "PluginComponents/myparams.cpp"
#include "SynthesizerComponents/MonoSynth.cpp"

#include "SynthesizerComponents/dynamic/StereoClipper.cpp"

#include "PluginComponents/MidiMaster.cpp"
#include "PluginComponents/BasicMidiPlugin.cpp"
#include "PluginComponents/BasicEditor.cpp"

#include "SynthesizerComponents/oscillator/NoiseGenerator.cpp"
#include "SynthesizerComponents/oscillator/Phasor.cpp"
#include "SynthesizerComponents/oscillator/waveshapes.cpp"
#include "SynthesizerComponents/oscillator/PolyBlep.cpp"
#include "SynthesizerComponents/oscillator/XoxosOscillator.cpp"
#include "SynthesizerComponents/oscillator/BasicOscillator.cpp"

#include "SynthesizerComponents/envelope/LinearAttack.cpp"
#include "SynthesizerComponents/envelope/LinearADSR.cpp"
#include "SynthesizerComponents/envelope/rosic_EnvelopeGenerator_elanEdit.h"

#include "SynthesizerComponents/filter/StereoTwoPoleBandpass.cpp"

#include "JerobeamAlgorithms/MaxMSPHelper.cpp"
#include "JerobeamAlgorithms/JerobeamBlubb.cpp"
#include "JerobeamAlgorithms/JerobeamBoing.cpp"
#include "JerobeamAlgorithms/JerobeamKeplerBouwkamp.cpp"
#include "JerobeamAlgorithms/JerobeamMushroom.cpp"
#include "JerobeamAlgorithms/JerobeamNyquistShannon.cpp"
#include "JerobeamAlgorithms/JerobeamRadar.cpp"
#include "JerobeamAlgorithms/JerobeamSpiral.cpp"
#include "JerobeamAlgorithms/JerobeamTorus.cpp"
#include "JerobeamAlgorithms/JerobeamWirdoSpiral.cpp"

#include "SynthesizerComponents/filter/Equalizer.cpp"

#include "Modules/Chaosfly/ChaosflyCore.cpp"
#include "Modules/Chaosfly/ChaosflyModule.cpp"
#include "Modules/Chaosfly/ChaosflyEditor.cpp"

#include "Modules/XoxosOscillatorPlugin/XoxosOscillatorCore.cpp"
//#include "Modules/XoxosOscillatorPlugin/XoxosOscillatorModule.cpp"
//#include "Modules/XoxosOscillatorPlugin/XoxosOscillatorEditor.cpp"

#include "Modules/BasicEnvelopePlugin/BasicEnvelopeModule.cpp"
#include "Modules/BasicEnvelopePlugin/BasicEnvelopeEditor.cpp"

#include "Modules/BasicOscillatorPlugin/BasicOscillatorCore.cpp"
#include "Modules/BasicOscillatorPlugin/BasicOscillatorModule.cpp"
#include "Modules/BasicOscillatorPlugin/BasicOscillatorEditor.cpp"

#include "Modules/MidiModSourcePlugin/MidiModSourceModule.cpp"
#include "Modules/MidiModSourcePlugin/MidiModSourceEditor.cpp"

#include "Modules/Torus Generator/JerobeamTorusCore.cpp"
//#include "Modules/Torus Generator/JerobeamTorusModule.cpp"
//#include "Modules/Torus Generator/JerobeamTorusEditor.cpp"

#include "Modules/OMS/JerobeamRadarCore.cpp"
//#include "Modules/OMS/OMSModule.cpp"
//#include "Modules/OMS/OMSEditor.cpp"

#include "Modules/FlowerChildFilter/FlowerChildFilterCore.cpp"
//#include "Modules/FlowerChildFilter/FlowerChildFilterModule.cpp"
//#include "Modules/FlowerChildFilter/FlowerChildFilterEditor.cpp"
