cmake_minimum_required(VERSION 3.28)
project(soemdsp)

add_definitions(-DNOMINMAX)

# ========================================================
# Fetch Dependencies
# ========================================================
include(FetchContent)

FetchContent_Declare(magic_enum
    GIT_REPOSITORY https://github.com/Neargye/magic_enum.git
    GIT_TAG v0.9.7
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(magic_enum)

FetchContent_Declare(fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG 12.1.0
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(fmt)

if(NOT TARGET fmt::fmt)
    add_library(fmt::fmt ALIAS fmt)
endif()

# ========================================================
# Main Library
# ========================================================
add_library(soemdsp STATIC)
add_library(soemdsp::soemdsp ALIAS soemdsp)

target_include_directories(soemdsp PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
    ${CMAKE_CURRENT_LIST_DIR}/libs
)

# ========================================================
# Collect Files
# ========================================================

# Headers from include/ (organized)
file(GLOB_RECURSE INCLUDE_HEADERS
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_LIST_DIR}/include/*.h"
    "${CMAKE_CURRENT_LIST_DIR}/include/*.hpp"
)

# Headers from libs/ (organized)
file(GLOB_RECURSE LIBS_HEADERS
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_LIST_DIR}/libs/*.h"
    "${CMAKE_CURRENT_LIST_DIR}/libs/*.hpp"
)

# All source files (.cpp) → go into one big flat "src" folder
file(GLOB_RECURSE ALL_SOURCE_FILES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_LIST_DIR}/include/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/libs/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp"
)

target_sources(soemdsp PRIVATE
    ${INCLUDE_HEADERS}
    ${LIBS_HEADERS}
    ${ALL_SOURCE_FILES}
)

# ========================================================
# Folder Organization in Visual Studio
# ========================================================

# 1. include/ folder - keeps real folder structure
source_group(TREE "${CMAKE_CURRENT_LIST_DIR}/include"
    PREFIX "include"
    FILES ${INCLUDE_HEADERS}
)

# 2. libs/ headers - keep their structure under "include/libs"
source_group(TREE "${CMAKE_CURRENT_LIST_DIR}/libs"
    PREFIX "include/libs"
    FILES ${LIBS_HEADERS}
)

# 3. ALL .cpp files in one flat "src" folder
source_group("src" FILES ${ALL_SOURCE_FILES})

# ========================================================
# Build Settings
# ========================================================
target_link_libraries(soemdsp
    PUBLIC
    magic_enum::magic_enum
    fmt::fmt
)

target_compile_features(soemdsp PUBLIC cxx_std_20)

set_property(TARGET soemdsp PROPERTY FOLDER "Libs")

add_subdirectory(src)
add_subdirectory(examples)
