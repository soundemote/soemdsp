// clang-format off
#include <soemdsp/utility/operations_vector.hpp>

#include <soemdsp/oscillator/Supersaw.hpp>
#include <soemdsp/oscillator/Hypersaw.hpp>
#include <soemdsp/delay/Reverb.hpp>

#include <soemdsp/envelope/PluckEnvelope.hpp>
#include <soemdsp/random/FlexibleRandomWalk.hpp>
#include <soemdsp/modulator/VibratoGenerator.hpp>
#include <soemdsp/modulator/WowAndFlutter.hpp>

#include <soemdsp/additive/HarmonicManager.hpp>
#include <soemdsp/additive/additive.hpp>
#include <soemdsp/additive/harmonic_algorithms.hpp>
#include <soemdsp/additive/harmonic_profiles.hpp>
#include <soemdsp/additive/layer.hpp>
#include <soemdsp/additive/layer/waveform.hpp>
#include <soemdsp/delay/ModulatedDelay.hpp>

#include <soemdsp/dynamics/EarProtector.hpp>
#include <soemdsp/dynamics/EnvelopeFollower.hpp>
#include <soemdsp/dynamics/SilenceDetector.hpp>
#include <soemdsp/dynamics/SoftClipper.hpp>
#include <soemdsp/envelope/DigitalCurveEnvelope.hpp>
#include <soemdsp/envelope/ExponentialEnvelope.hpp>
#include <soemdsp/envelope/LinearEnvelope.hpp>

#include <soemdsp/filter/MultiStageFilter.hpp>
#include <soemdsp/filter/OnePoleFilter.hpp>
#include <soemdsp/filter/Smoother.hpp>
#include <soemdsp/filter/Superlove.hpp>
#include <soemdsp/modulator/Attractor.hpp>
#include <soemdsp/modulator/Parabol.hpp>
#include <soemdsp/musical/BeatDivision.hpp>
#include <soemdsp/musical/Transport.hpp>
#include <soemdsp/oscillator/DSFOscillator.hpp>
#include <soemdsp/oscillator/DistortionOscillator.hpp>
#include <soemdsp/oscillator/Ellipsoid.hpp>
#include <soemdsp/oscillator/PolyBLEP.hpp>
#include <soemdsp/oscillator/SineWavetable.hpp>
#include <soemdsp/plugin/DirtyUpdater.hpp>
#include <soemdsp/plugin/Parameter.hpp>
#include <soemdsp/plugin/ParameterPrototype.hpp>
#include <soemdsp/plugin/SmootherManager.hpp>
#include <soemdsp/plugin/VoiceManager.hpp>
#include <soemdsp/random/FractalBrownianMotion.hpp>
#include <soemdsp/random/NoiseGenerator.hpp>
#include <soemdsp/random/Random.hpp>
#include <soemdsp/random/oracle.hpp>
#include <soemdsp/utility/EnumArray.hpp>

#include <soemdsp/utility/MaxMSPHelper.hpp>
#include <soemdsp/utility/StringIterator.hpp>
#include <soemdsp/utility/curve_functions.hpp>
#include <soemdsp/utility/operations_area.hpp>
#include <soemdsp/utility/operations_char.hpp>
#include <soemdsp/utility/operations_string.hpp>

#include <soemdsp/utility/Cache.hpp>
#include <soemdsp/utility/Graph.hpp>
#include <soemdsp/utility/Hash.hpp>

#include <soemdsp/timer/Counter.hpp>
#include <soemdsp/timer/DelayedTrigger.hpp>
#include <soemdsp/timer/SampleAndHold.hpp>
#include <soemdsp/timer/SampleAndHoldSmoothed.hpp>
#include <soemdsp/timer/Timer.hpp>

#include <soemdsp/meta.hpp>
#include <soemdsp/sehelper.hpp>
#include <soemdsp/semath.hpp>
#include <soemdsp/semidi.hpp>
#include <soemdsp/Wire.hpp>
#include <soemdsp/SampleRate.hpp>
#include <soemdsp/Phasor.hpp>
// clang-format on
#include <iostream>
#include <string>
#include <filesystem>
//Industrial ANSI constants
namespace ansi {
constexpr const char* reset = "\033[0m";
constexpr const char* cyan  = "\033[0;36m";
constexpr const char* green = "\033[0;32m";
} //namespace ansi

void printExecutionPath() {
    // get the current directory
    auto path = std::filesystem::current_path();
    
    // print it
    std::cout << "[SYSTEM] I am executing from: " << path << std::endl;
}

int main() {
    //Startup success message
    std::cout << ansi::cyan << "soemdsp "
              << ansi::green << "successfully compiled."
              << ansi::reset << std::endl;
    printExecutionPath();
    std::cout << ansi::cyan << "press any key to exit"
              << ansi::reset << std::endl;

    std::cin.get();
    return 0;
}


