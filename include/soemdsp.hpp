#pragma once

/*
update repository using Git Bash at library's git location:
git switch main
git add .
git commit -m "commit message"
git push
*/

// General library support
#include "soemdsp/SampleRate.hpp"
#include "soemdsp/Wire.hpp"
#include "soemdsp/sehelper.hpp"
#include "soemdsp/semath.hpp"
#include "soemdsp/semidi.hpp"
#include "soemdsp/curve_functions.hpp"
#include "soemdsp/operations_area.hpp"
#include "soemdsp/operations_char.hpp"
#include "soemdsp/operations_string.hpp"
#include "soemdsp/operations_vector.hpp"
#include "soemdsp/EnumArray.hpp"

// Miscellaneous
#include "soemdsp/utility/BeatDivision.hpp"
#include "soemdsp/utility/Graph.hpp"
#include "soemdsp/utility/Hash.hpp"
#include "soemdsp/utility/NoiseGenerator.hpp"
#include "soemdsp/utility/StringIterator.hpp"
#include "soemdsp/utility/Transport.hpp"

// Additive synthesis
#include "soemdsp/additive/additive.hpp"

// Time based effects and processing
#include "soemdsp/delay/ModulatedDelay.hpp"
#include "soemdsp/delay/Reverb.hpp"

// Amplitude based effects and processing
#include "soemdsp/dynamics/EarProtector.hpp"
#include "soemdsp/dynamics/EnvelopeFollower.hpp"
#include "soemdsp/dynamics/SilenceDetector.hpp"
#include "soemdsp/dynamics/SoftClipper.hpp"

// Frequency based processing
#include "soemdsp/filter/OnePoleFilter.hpp"
#include "soemdsp/filter/Smoother.hpp"
#include "soemdsp/filter/MultiStageFilter.hpp"

// Random number generators
#include "soemdsp/random/FlexibleRandomWalk.hpp"
#include "soemdsp/random/FractalBrownianMotion.hpp"
#include "soemdsp/random/Random.hpp"

// Generators that are meant to control parameters and not suited for audio
#include "soemdsp/modulator/ExponentialEnvelope.hpp"
#include "soemdsp/modulator/LinearEnvelope.hpp"
#include "soemdsp/modulator/Parabol.hpp"
#include "soemdsp/modulator/PluckEnvelope.hpp"
#include "soemdsp/modulator/VibratoGenerator.hpp"
#include "soemdsp/modulator/WowAndFlutter.hpp"

// Generators that are antialiased and meant for audio
#include "soemdsp/oscillator/Attractor.hpp"
#include "soemdsp/oscillator/DSFOscillator.hpp"
#include "soemdsp/oscillator/DistortionOscillator.hpp"
#include "soemdsp/oscillator/Ellipsoid.hpp"
#include "soemdsp/oscillator/Hypersaw.hpp"
#include "soemdsp/oscillator/PolyBLEP.hpp"
#include "soemdsp/oscillator/Supersaw.hpp"

// Timers and counters
#include "soemdsp/timer/Counter.hpp"
#include "soemdsp/timer/DelayedTrigger.hpp"
#include "soemdsp/timer/SampleAndHold.hpp"
#include "soemdsp/timer/Timer.hpp"


// Helpers for audio plugin interfacing
#include "soemdsp/plugin/ParamData.h"
#include "soemdsp/plugin/ParamHelpers.hpp"
#include "soemdsp/plugin/ParamType.hpp"
#include "soemdsp/DirtyUpdater.hpp"
#include "soemdsp/VoiceManager.hpp"
#include "soemdsp/SmootherManager.hpp"
