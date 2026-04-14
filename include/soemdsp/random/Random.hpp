#pragma once

#include "soemdsp/sehelper.hpp"
#include "soemdsp/semath.hpp"
#include "soemdsp/Wire.hpp"

#include <cassert>
#include <cstdint>
#include <random>

namespace soemdsp::random {
// Linear Congruential Generator from https://github.com/RobinSchmidt/RS-MET/blob/master/Libraries/RobsJuceModules/rapt/Generators/NoiseGenerator.h
struct LCG32 {
    SE_DEFAULT_CONSTRUCTOR(LCG32);

    // io
    void reset() {
        state_ = seed_;
    }
    void setSeedAndReset(int v) {
        state_ = seed_ = v;
    }
    double runUnipolar() {
        state_ = (1664525 * state_ + 1013904223) & 4294967295;
        out_   = k1z4294967296 * state_;
        return out_;
    }
    double runBipolar() {
        state_ = (1664525 * state_ + 1013904223) & 4294967295;
        out_   = k2z4294967296 * state_ - 1.0;
        return out_;
    }
    double out_{};

    // parameters
    Wire<int> seed_;

    // coefficients
    uint64_t state_{};
    static constexpr double k1z4294967296{ 1.0 / 4294967296.0 };
    static constexpr double k2z4294967296{ 2.0 / 4294967296.0 };
};

// Linear Feedback Shift Register from https://audiodev.blog/random-numbers/
struct LFSR32 {
    SE_DEFAULT_CONSTRUCTOR(LFSR32);

    // io
    void reset() {
        state_ = seed_;
    }
    void setSeedAndReset(uint32_t newSeed) {
        seed_  = newSeed;
        state_ = newSeed;
    }
    double runBipolar() {
        // Update state using the LFSR algorithm
        (state_ & 1UL) ? state_ = (state_ >> 1) ^ POLY : state_ >>= 1;

        // Extract a 25-bit value (shift right by 7 and mask),
        // then normalize to the range [-1, 1) by subtracting DIVISOR and dividing by DIVISOR.
        out_ = (double((state_ >> 7) & MASK) - DIVISOR) / DIVISOR;
        return out_;
    }
    double runUnipolar() {
        out_ = biToUni(runBipolar());
        return out_;
    }
    double out_{};

    // parameters
    uint32_t seed_{ 161803398UL }; // Default seed: chosen for its non-zero, somewhat arbitrary value

    // coefficients
    uint32_t state_{ seed_ };                  // Current state of the LFSR, initialized to seed
    static constexpr uint32_t POLY{ 0x80000062UL }; // Polynomial constant for feedback (2nd order tap)
    static constexpr uint32_t MASK{ 0x1FFFFFFUL };  // 25-bit mask for noise extraction
    static constexpr double DIVISOR{ 16777216.0f }; // Normalization divisor (2^24)
};

// Linear Feedback Shift Register from https://audiodev.blog/random-numbers/ + ChatGPT
struct LFSR64 {
    // Constructors
    SE_DEFAULT_CONSTRUCTOR(LFSR64);

    // io
    void reset() {
        state_ = seed_;
    }
    void setSeedAndReset(uint64_t newSeed) {
        seed_  = newSeed;
        state_ = newSeed;
    }
    double runBipolar() {
        // Update state using a 64-bit LFSR algorithm.
        // (If the least significant bit is 1, XOR with the polynomial constant.)
        (state_ & 1ULL) ? state_ = (state_ >> 1) ^ POLY : state_ >>= 1;

        // Extract 54 bits: shift right by 10 bits then mask.
        // This gives a value in the range [0, 2^54).
        // Subtract k2p53int (which is 2^53) to center the value around zero,
        // then normalize by dividing by k2p53dbl (2^53) to yield a result in [-1, 1).
        int64_t temp = int64_t((state_ >> 10) & MASK_54BITS) - k2p53int;
        out_         = double(temp) / k2p53dbl;
        return out_;
    }
    double runUnipolar() {
        out_ = biToUni(runBipolar());
        return out_;
    }
    double out_{};

    // parameters
    uint64_t seed_{ 0x5555555555555555ULL }; // Default seed with alternating bits for balance

    // coefficients
    uint64_t state_{ seed_ };                                       // Current state of the LFSR, initialized to seed
    static constexpr uint64_t POLY{ 0xD800000000000000ULL };      // 64-bit polynomial constant (example candidate)
    static constexpr uint64_t MASK_54BITS{ 0x3FFFFFFFFFFFFFULL }; // 54-bit mask for noise extraction
    static constexpr uint64_t k2p53int{ 9007199254740992ULL };    // 2^53
    static constexpr double k2p53dbl{ 9007199254740992.0 };       // 2^53
};

// Linear Congruential Generator from https://audiodev.blog/random-numbers/
struct LCG64 {
    SE_DEFAULT_CONSTRUCTOR(LCG64);

    // io
    void reset() {
        state_ = seed_;
    }
    void setSeedAndReset(uint64_t v) {
        seed_  = v;
        state_ = v;
    }
    double runUnipolar() {
        // Update state using the 64-bit LCG formula
        state_ = state_ * MULTIPLIER + INCREMENT;
        // Extract 53 bits (shift by 11 bits, then mask) and normalize to [0, 1)
        out_ = double((state_ >> 11) & MASK_53BITS) / k2p53dbl; // k2p53flt = 2^53
        return out_;
    }
    double runBipolar() {
        // Update state using the 64-bit LCG formula
        state_ = state_ * MULTIPLIER + INCREMENT;
        // Extract 54 bits (shift by 10 bits, then mask), subtract k2p53int (2^53), and normalize to [-1, 1)
        int64_t temp = int64_t((state_ >> 10) & MASK_54BITS) - k2p53int; // k2p53int = 2^53
        out_ = double(temp) / k2p53dbl;                                 // k2p53flt = 2^53
        return out_;
    }
    double out_;

    // parameters
    uint64_t seed_{ 161803398ULL };

    // coefficients
    uint64_t state_{ seed_ };
    static constexpr uint64_t MULTIPLIER{ 6364136223846793005ULL };
    static constexpr uint64_t INCREMENT{ 1442695040888963407ULL };
    static constexpr uint64_t MASK_53BITS{ 0x1FFFFFFFFFFFFFULL }; // 53-bit mask for unipolar output
    static constexpr uint64_t MASK_54BITS{ 0x3FFFFFFFFFFFFFULL }; // 54-bit mask for bipolar output
    static constexpr uint64_t k2p53int{ 9007199254740992ULL };    // 2^53 as an integer
    static constexpr double k2p53dbl{ 9007199254740992.0 };       // 2^53 as a floating point number
};

struct LCG {
    // Constructors
    SE_DEFAULT_CONSTRUCTOR(LCG);

    // io
    void reset() {
        engine.seed(initialSeed);
    }
    void setSeedAndReset(unsigned int newSeed) {
        initialSeed = newSeed;
        engine.seed(newSeed);
    }
    double runUnipolar() {
        return out_ = double(engine() - kEngineMin) / DIVISOR;
    }
    double runBipolar() {
        return out_ = uniToBi(runUnipolar());
    }
    double out_;

    // parameters
    unsigned int initialSeed{ 0 }; // The initial seed for reproducibility

    // coefficients
    std::minstd_rand engine; // The underlying linear congruential generator (LCG)

    // Cached constants for engine's range (computed at compile time)
    static constexpr unsigned int kEngineMin{ std::minstd_rand::min() };
    static constexpr unsigned int kEngineMax{ std::minstd_rand::max() };
    static constexpr double DIVISOR{ kEngineMax - kEngineMin };
};

} // namespace soemdsp::oscillator
