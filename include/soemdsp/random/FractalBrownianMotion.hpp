#pragma once

#include <soemdsp/Wire.hpp>
#include <soemdsp/semath.hpp>

#include "stb/stb_perlin.h"

namespace soemdsp::modulator {

struct FractalBrownianMotion {
    FractalBrownianMotion(double time = 0.0, double octaves = 4, double persistence = 0.5, double seed = 0.0, double scale = 1.0)
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
            total += stb_perlin_noise3(time_ * scale_ * frequency, seed_, 0, 0, 0, 0) * amplitude;
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
    Wire increment_;
    Wire octaves_{ 4 };          // 1 - 8, Number of octaves for fractal noise, higher values for more detail)
    Wire persistence_{ 0.5 }; // 0.0 to <1.0, High values create a "rougher" texture with more detail and high-frequency components. Low persistence creates a "smoother" texture with
                                      // larger-scale patterns and less noticeable fine details.
    Wire scale_{ 1.0 };       // 0.1 to 10.0, Scale factor for input time, smaller values means smoother noise)
    Wire seed_;

    // coefficients
    double maxValue_{};
    double time_;

    private:
// Wrapper that manages the messy 6-parameter signature.
// Internal logic remains double-precision; external call is cast to float.
inline double perlin_wrapper(double x, double y, double z, int wrapX = 0, int wrapY = 0, int wrapZ = 0) {
    return static_cast<double>(stb_perlin_noise3(
        static_cast<float>(x),
        static_cast<float>(y),
        static_cast<float>(z),
        wrapX, wrapY, wrapZ
    ));
    }
};
} // namespace soemdsp::modulator
