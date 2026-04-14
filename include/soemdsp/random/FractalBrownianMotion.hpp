#pragma once

#include "soemdsp/Wire.hpp"
#include "soemdsp/semath.hpp"
#include "stb/stb_perlin.h"

namespace soemdsp::modulator {

struct FractalBrownianMotion {
    SE_CUSTOM_CONSTRUCTOR(FractalBrownianMotion)
    (double time = 0.0, int octaves = 4, double persistence = 0.5, int seed = 0.0, double scale = 1.0)
      : time_(time)
      , octaves_(octaves)
      , persistence_(persistence)
      , seed_(seed)
      , scale_(scale) {}

    // io
    void reset() {
        time_ = 0.0;
    }
    double runBipolar() {
        double total     = 0.0f; // Total accumulated value
        double amplitude = 1.0f; // Amplitude of the current octave
        double frequency = 1.0f; // Frequency of the current octave

        // Loop through octaves to build fractal noise
        maxValue_ = 0.0f; // Maximum possible value, used for normalizing the noise (starts at 0, accumulates over octaves)
        for (int i = 0; i < octaves_; ++i) {
            total += toDouble(stb_perlin_noise3(toFloat(time_ * scale_ * frequency), toFloat(seed_), 0.0f, 0, 0, 0)) * amplitude;
            maxValue_ += amplitude;
            amplitude *= persistence_; // Decay amplitude for each octave (range: 0.0f - 1.0f)
            frequency *= 2.0f;         // Double the frequency for the next octave (range: 1.0f - 2^octaves)
        }

        time_ += increment_;

        return total;
    }

    double runUnipolar() {
        return (runBipolar() / maxValue_ + 1.0f) / 2.0f;
    }

    // parameters
    Wire<double> increment_;
    Wire<int> octaves_{ 4 };          // 1 - 8, Number of octaves for fractal noise, higher values for more detail)
    Wire<double> persistence_{ 0.5 }; // 0.0 to <1.0, High values create a "rougher" texture with more detail and high-frequency components. Low persistence creates a "smoother" texture with
                                      // larger-scale patterns and less noticeable fine details.
    Wire<double> scale_{ 1.0 };       // 0.1 to 10.0, Scale factor for input time, smaller values means smoother noise)
    Wire<int> seed_;

    // coefficients
    double maxValue_{};
    double time_;
};
} // namespace soemdsp::modulator
