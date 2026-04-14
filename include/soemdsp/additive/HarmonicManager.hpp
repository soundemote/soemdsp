#pragma once

#include "soemdsp/sehelper.hpp"
#include "soemdsp/semath.hpp"

#include <vector>
#include <array>
#include <algorithm>

namespace soemdsp::additive {
constexpr size_t MAX_HARMONICS{ 1 << 11 }; // 2048

struct LayerBase;

struct HarmonicManager {
    SE_CUSTOM_CONSTRUCTOR(HarmonicManager)() {}

    //io
    void run();
    void reset();
    void resize(size_t sz);

    double outL_{};
    double outR_{};

    std::array<double, MAX_HARMONICS> cycle_{};
    std::array<double, MAX_HARMONICS> phase_{};
    std::array<double, MAX_HARMONICS> increment_{};
    std::array<double, MAX_HARMONICS> amplitude_{1.0};
    std::array<double, MAX_HARMONICS> panning_{};

    size_t currentSize_{};
    size_t lastHarmonicToRender_{};

    std::vector<LayerBase> layers_;
};
} // namespace soemdsp::additive
