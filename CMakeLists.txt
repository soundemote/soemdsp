cmake_minimum_required(VERSION 3.23)

project(
  TODOFIXTHIS
  VERSION 0.1
  DESCRIPTION "TODOFIXTHIS"
  HOMEPAGE_URL "https://www.soundemote.com/"
  LANGUAGES C CXX
)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  get_property(isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)

  if(NOT isMultiConfig)
    set(allowedBuildTypes Debug Release RelWithDebInfo)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "${allowedBuildTypes}")

    if(NOT CMAKE_BUILD_TYPE)
      set(CMAKE_BUILD_TYPE
          Debug
          CACHE STRING "" FORCE
      )
    elseif(NOT CMAKE_BUILD_TYPE IN_LIST allowedBuildTypes)
      message(FATAL_ERROR "Unknown build type: ${CMAKE_BUILD_TYPE}")
    endif()
  endif()
endif()

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

include(CMakeDependentOption)
include(GenerateExportHeader)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
include(CMakeFindDependencyMacro)
include(CheckIPOSupported)
include(CMakePrintHelpers)

include(cmake/thirdparty.cmake)

# Project options
set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN TRUE)
set(CMAKE_VERIFY_INTERFACE_HEADER_SETS TRUE)

# set(CMAKE_DEBUG_POSTFIX "-d")
add_library(
  soemdsp
  ./include/soemdsp.hpp
  ./include/soemdsp/additive/additive.hpp
  ./include/soemdsp/additive/harmonic_algorithms.hpp
  ./include/soemdsp/additive/harmonic_profiles.hpp
  ./include/soemdsp/curve_functions.cpp
  ./include/soemdsp/curve_functions.hpp
  ./include/soemdsp/delay/ModulatedDelay.cpp
  ./include/soemdsp/delay/ModulatedDelay.hpp
  ./include/soemdsp/delay/Reverb.cpp
  ./include/soemdsp/delay/Reverb.hpp
  ./include/soemdsp/delay/SignalSmithReverb.hpp
  ./include/soemdsp/DirtyUpdater.hpp
  ./include/soemdsp/dynamics/EarProtector.hpp
  ./include/soemdsp/dynamics/EnvelopeFollower.hpp
  ./include/soemdsp/dynamics/SilenceDetector.hpp
  ./include/soemdsp/dynamics/SoftClipper.hpp
  ./include/soemdsp/EnumArray.hpp
  ./include/soemdsp/filter/MultiStageFilter.hpp
  ./include/soemdsp/filter/OnePoleFilter.hpp
  ./include/soemdsp/filter/Smoother.hpp
  ./include/soemdsp/modulator/DigitalCurveEnvelope.hpp
  ./include/soemdsp/modulator/ExponentialEnvelope.hpp
  ./include/soemdsp/modulator/LinearEnvelope.hpp
  ./include/soemdsp/modulator/Parabol.hpp
  ./include/soemdsp/modulator/PluckEnvelope.hpp
  ./include/soemdsp/modulator/VibratoGenerator.hpp
  ./include/soemdsp/modulator/WowAndFlutter.hpp
  ./include/soemdsp/operations_area.hpp
  ./include/soemdsp/operations_char.cpp
  ./include/soemdsp/operations_char.hpp
  ./include/soemdsp/operations_string.cpp
  ./include/soemdsp/operations_string.hpp
  ./include/soemdsp/operations_vector.hpp
  ./include/soemdsp/oscillator/Attractor.hpp
  ./include/soemdsp/oscillator/DistortionOscillator.hpp
  ./include/soemdsp/oscillator/DSFOscillator.hpp
  ./include/soemdsp/oscillator/Ellipsoid.hpp
  ./include/soemdsp/oscillator/Hypersaw.hpp
  ./include/soemdsp/oscillator/JerobeamBlubb.cpp
  ./include/soemdsp/oscillator/JerobeamBlubb.h
  ./include/soemdsp/oscillator/JerobeamBoing.cpp
  ./include/soemdsp/oscillator/JerobeamBoing.h
  ./include/soemdsp/oscillator/JerobeamKeplerBouwkamp.cpp
  ./include/soemdsp/oscillator/JerobeamKeplerBouwkamp.h
  ./include/soemdsp/oscillator/JerobeamMushroom.cpp
  ./include/soemdsp/oscillator/JerobeamMushroom.h
  ./include/soemdsp/oscillator/JerobeamNyquistShannon.cpp
  ./include/soemdsp/oscillator/JerobeamNyquistShannon.h
  ./include/soemdsp/oscillator/JerobeamRadar.cpp
  ./include/soemdsp/oscillator/JerobeamRadar.h
  ./include/soemdsp/oscillator/JerobeamSpiral.cpp
  ./include/soemdsp/oscillator/JerobeamSpiral.h
  ./include/soemdsp/oscillator/JerobeamTorus.cpp
  ./include/soemdsp/oscillator/JerobeamTorus.h
  ./include/soemdsp/oscillator/JerobeamWirdoSpiral.cpp
  ./include/soemdsp/oscillator/JerobeamWirdoSpiral.h
  ./include/soemdsp/modulator/Phasor.cpp
  ./include/soemdsp/modulator/Phasor.h
  ./include/soemdsp/oscillator/PolyBLEP.hpp
  ./include/soemdsp/oscillator/Supersaw.hpp
  ./include/soemdsp/plugin/ParamData.h
  ./include/soemdsp/plugin/ParamHelpers.hpp
  ./include/soemdsp/plugin/ParamType.hpp
  ./include/soemdsp/random/FlexibleRandomWalk.hpp
  ./include/soemdsp/random/FractalBrownianMotion.hpp
  ./include/soemdsp/random/Random.hpp
  ./include/soemdsp/SampleRate.hpp
  ./include/soemdsp/sehelper.hpp
  ./include/soemdsp/semath.cpp
  ./include/soemdsp/semath.hpp
  ./include/soemdsp/semidi.hpp
  ./include/soemdsp/SmootherManager.hpp
  ./include/soemdsp/timer/Counter.hpp
  ./include/soemdsp/timer/DelayedTrigger.hpp
  ./include/soemdsp/timer/SampleAndHold.hpp
  ./include/soemdsp/timer/Timer.hpp
  ./include/soemdsp/utility/BeatDivision.cpp
  ./include/soemdsp/utility/BeatDivision.hpp
  ./include/soemdsp/utility/Graph.hpp
  ./include/soemdsp/utility/Hash.hpp
  ./include/soemdsp/utility/MaxMSPHelper.cpp
  ./include/soemdsp/utility/MaxMSPHelper.h
  ./include/soemdsp/utility/NoiseGenerator.hpp
  ./include/soemdsp/utility/oracle.hpp
  ./include/soemdsp/utility/StringIterator.cpp
  ./include/soemdsp/utility/StringIterator.hpp
  ./include/soemdsp/utility/Transport.cpp
  ./include/soemdsp/utility/Transport.hpp
  ./include/soemdsp/Var.hpp
  ./include/soemdsp/VoiceManager.hpp
  # ./libs/RSMET/rosic/rosic_BreakpointModulator.cpp
  ./libs/RSMET/Vector.cpp
  ./libs/RSMET/StandardContainerTools.cpp
  ./libs/RSMET/RSLib/RealFunctions.cpp
  ./libs/RSMET/RAPT/PoleZeroMapper.cpp
  ./libs/RSMET/RAPT/NumberTheory.cpp
  ./libs/RSMET/RAPT/MatrixTools.cpp
  ./libs/RSMET/RAPT/IntegerFunctions.cpp
  ./libs/RSMET/RAPT/FilterCoefficientConverter.cpp
  ./libs/RSMET/LinearAlgebraNew.cpp
  ./libs/RSMET/CurveFitting.cpp
  ./libs/RSMET/AudioFunctions.cpp
  ./libs/RSMET/LadderFilter.cpp
  ./libs/RSMET/rosic/rosic_CookbookFilter.cpp
  ./libs/RSMET/rosic/rosic_ComplexFunctionsAlgorithms.cpp
  ./libs/RSMET/rosic/rosic_ComplexFunctions.cpp
)

if(NOT TARGET magic_enum::magic_enum)
  add_subdirectory(libs/magic_enum)
endif()

target_include_directories(
  soemdsp PUBLIC ${CMAKE_CURRENT_LIST_DIR} ${CMAKE_CURRENT_LIST_DIR}/include
                 ${CMAKE_CURRENT_LIST_DIR}/libs
)
target_link_libraries(soemdsp PUBLIC magic_enum::magic_enum fmt::fmt oversimple)

add_subdirectory(src)
