#pragma once

// clang-format off

// General
#include <soemdsp/Phasor.hpp>
#include <soemdsp/SampleRate.hpp>
#include <soemdsp/Wire.hpp>
#include <soemdsp/seconstant.hpp>
#include <soemdsp/sehelper.hpp>
#include <soemdsp/semath.hpp>
#include <soemdsp/semidi.hpp>

// Dynamics
#include <soemdsp/dynamics/FlowerChildEnvelopeFollower.hpp>

// Utility
#include <soemdsp/utility/EnumArray.hpp>
#include <soemdsp/musical/BeatDivision.hpp>
#include <soemdsp/utility/Graph.hpp>
#include <soemdsp/utility/Hash.hpp>
#include <soemdsp/random/NoiseGenerator.hpp>
#include <soemdsp/utility/StringIterator.hpp>
#include <soemdsp/utility/curve_functions.hpp>
#include <soemdsp/utility/operations_area.hpp>
#include <soemdsp/utility/operations_char.hpp>
#include <soemdsp/utility/operations_string.hpp>
#include <soemdsp/utility/operations_vector.hpp>

// Modulator
#include <soemdsp/modulator/AnalogVocoder.hpp>
#include <soemdsp/modulator/Vactrol.hpp>

// Plugin
#include <soemdsp/musical/Transport.hpp>
#include <soemdsp/plugin/DirtyUpdater.hpp>
#include <soemdsp/plugin/SmootherManager.hpp>
#include <soemdsp/plugin/VoiceManager.hpp>
#include <soemdsp/plugin/Parameter.hpp>
#include <soemdsp/plugin/parameterhelper.hpp>

// clang-format on

#include <soemdsp/runtime/Connection.hpp>
#include <soemdsp/runtime/Circuit.hpp>
#include <soemdsp/runtime/Node.hpp>
#include <soemdsp/runtime/Parameter.hpp>
#include <soemdsp/runtime/Port.hpp>
#include <soemdsp/runtime/nodes/Add.hpp>
#include <soemdsp/runtime/nodes/Constant.hpp>
#include <soemdsp/runtime/nodes/Multiply.hpp>
#include <soemdsp/runtime/nodes/AudioConstant.hpp>
#include <soemdsp/runtime/nodes/AudioAdd.hpp>
#include <soemdsp/runtime/nodes/AudioMultiply.hpp>
#include <soemdsp/runtime/nodes/SineOscillator.hpp>
#include <soemdsp/runtime/nodes/FloatConstant.hpp>
#include <soemdsp/runtime/nodes/RampGenerator.hpp>
#include <soemdsp/runtime/nodes/MultiplyAdd.hpp>
#include <soemdsp/runtime/nodes/AudioMultiplyAdd.hpp>
#include <soemdsp/runtime/nodes/AudioOutput.hpp>
#include <soemdsp/runtime/nodes/AudioCopy.hpp>
#include <soemdsp/runtime/debug/ExportText.hpp>
#include <soemdsp/runtime/debug/FindNodes.hpp>
#include <soemdsp/runtime/debug/PrintCircuit.hpp>
#include <soemdsp/runtime/debug/PrintLayout.hpp>

#include <soemdsp/version.hpp>
