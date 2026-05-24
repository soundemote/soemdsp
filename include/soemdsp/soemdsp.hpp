#pragma once

// clang-format off

// General
#include <soemdsp/Phasor.hpp>
#include <soemdsp/SampleRate.hpp>
#include <soemdsp/Wire.hpp>
#include <soemdsp/sehelper.hpp>
#include <soemdsp/semath.hpp>
#include <soemdsp/semidi.hpp>

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

// Plugin
#include <soemdsp/musical/Transport.hpp>
#include <soemdsp/plugin/DirtyUpdater.hpp>
#include <soemdsp/plugin/SmootherManager.hpp>
#include <soemdsp/plugin/VoiceManager.hpp>
#include <soemdsp/plugin/Parameter.hpp>
#include <soemdsp/plugin/parameterhelper.hpp>

// clang-format on

#include <soemdsp/runtime/Connection.hpp>
#include <soemdsp/runtime/Graph.hpp>
#include <soemdsp/runtime/Node.hpp>
#include <soemdsp/runtime/Port.hpp>
#include <soemdsp/runtime/nodes/Add.hpp>
#include <soemdsp/runtime/nodes/Constant.hpp>
#include <soemdsp/runtime/nodes/Multiply.hpp>
#include <soemdsp/runtime/nodes/AudioConstant.hpp>
