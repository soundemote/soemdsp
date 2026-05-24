#pragma once
#include <cassert>
#include <cstdint>
#include <random>
#include <soemdsp/Wire.hpp>
#include <soemdsp/sehelper.hpp>
#include <soemdsp/semath.hpp>
// clan-format off
//noise generators
namespace soemdsp::random {
struct LCG64 {
    LCG64() = default;
    //Parameters now managed as Wires
    Wire seed_{ 161803398.0 };
    Wire state_{ 161803398.0 };
    void reset() {
        state_ = seed_;
    }
    double runBipolar() {
        //Perform LCG math
        state_ = state_ * MULTIPLIER + INCREMENT;

        //Bitwise logic boundary
        int64_t temp = static_cast<int64_t>((static_cast<uint64_t>(state_) >> 10) & MASK_54BITS) - k2p53int;
        return static_cast<double>(temp) / k2p53dbl;
    }
    double runUnipolar() {
        return convert::to_unipolar(runBipolar());
    }
    static constexpr uint64_t MULTIPLIER  { 6364136223846793005ULL };
    static constexpr uint64_t INCREMENT   { 1442695040888963407ULL };
    static constexpr uint64_t MASK_54BITS { 0x3FFFFFFFFFFFFFULL    };
    static constexpr int64_t  k2p53int    { 9007199254740992LL     };
    static constexpr double   k2p53dbl    { 9007199254740992.0     };
};
struct LFSR64 {
    LFSR64() = default;
    //Parameters managed as Wires
    Wire seed_  { 0x1.5555555555555p+62 }; // 64 bits of alternating 01010101
    Wire state_ { 0x1.5555555555555p+62 };  // 64 bits of alternating 01010101

    void reset() {
        state_ = seed_;
    }

    void setSeedAndReset(double newSeed) {
        seed_  = newSeed;
        state_ = newSeed;
    }

    double runBipolar() {
        //Bitwise update using 64-bit LFSR
        uint64_t s = static_cast<uint64_t>(state_);
        (s & 1ULL) ? s = (s >> 1) ^ POLY : s >>= 1;
        state_ = static_cast<double>(s);

        //Extract 54 bits, center, and normalize
        int64_t temp = static_cast<int64_t>((s >> 10) & MASK_54BITS) - k2p53int;
        return static_cast<double>(temp) / k2p53dbl;
    }

    double runUnipolar() {
        return (runBipolar() + 1.0) * 0.5;
    }

    //Constants
    static constexpr uint64_t POLY        { 0xD800000000000000ULL };
    static constexpr uint64_t MASK_54BITS { 0x3FFFFFFFFFFFFFULL   };
    static constexpr int64_t  k2p53int    { 9007199254740992LL    };
    static constexpr double   k2p53dbl    { 9007199254740992.0    };
};
} //namespace soemdsp::random
namespace soemdsp::global {
//thread-local instance is unique to each thread and prevents race conditions without requiring locks
inline thread_local random::LCG64 randomengine;
} //namespace soemdsp::global

////Linear Congruential Generator from https://github.com/RobinSchmidt/RS-MET/blob/master/Libraries/RobsJuceModules/rapt/Generators/NoiseGenerator.h
//struct LCG32 {
//    LCG32() = default;
//
//    void reset() {
//        state_ = seed_;
//    }
//    void setState(int v) {
//        state_ = v;
//    }
//    void setSeed(int v) {
//        seed_ = v;
//    }
//    void setStateAndSeed(int v) {
//        state_ = seed_ = v;
//    }
//    double runUnipolar() {
//        state_ = (1664525 * state_ + 1013904223) & 4294967295;
//        out_   = k1z4294967296 * state_;
//        return out_;
//    }
//    double runBipolar() {
//        state_ = (1664525 * state_ + 1013904223) & 4294967295;
//        out_   = k2z4294967296 * state_ - 1.0;
//        return out_;
//    }
//
//    double out_{};
//    Wire seed_;
//    uint64_t state_{};
//    static constexpr double k1z4294967296{ 1.0 / 4294967296.0 };
//    static constexpr double k2z4294967296{ 2.0 / 4294967296.0 };
//};
//
////Linear Feedback Shift Register from https://audiodev.blog/random-numbers/
//struct LFSR32 {
//    LFSR32() = default;
//
//    void reset() {
//        state_ = seed_;
//    }
//    void setState(uint32_t v) {
//        state_ = v;
//    }
//    void setSeed(uint32_t v) {
//        seed_ = v;
//    }
//    void setStateAndSeed(uint32_t v) {
//        state_ = v;
//        seed_  = v;
//    }
//    double runUnipolar() {
//        out_ = convert::biToUni(runBipolar());
//        return out_;
//    }
//    double runBipolar() {
//        //Update state using the LFSR algorithm
//        (state_ & 1UL) ? state_ = (state_ >> 1) ^ POLY : state_ >>= 1;
//
//        //Extract a 25-bit value (shift right by 7 and mask),
//        //then normalize to the range [-1, 1) by subtracting DIVISOR and dividing by DIVISOR.
//        out_ = (double((state_ >> 7) & MASK) - DIVISOR) / DIVISOR;
//        return out_;
//    }
//
//    double out_{};
//
//    //parameters
//    uint32_t seed_{ 161803398UL }; //Default seed: chosen for its non-zero, somewhat arbitrary value
//
//    //coefficients
//    uint32_t state_{ seed_ };                       //Current state of the LFSR, initialized to seed
//    static constexpr uint32_t POLY{ 0x80000062UL }; //Polynomial constant for feedback (2nd order tap)
//    static constexpr uint32_t MASK{ 0x1FFFFFFUL };  //25-bit mask for noise extraction
//    static constexpr double DIVISOR{ 16777216.0f }; //Normalization divisor (2^24)
//};
//
////Linear Feedback Shift Register from https://audiodev.blog/random-numbers/ + ChatGPT
//struct LFSR64 {
//    LFSR64() = default;
//
//    //io
//    void reset() {
//        state_ = seed_;
//    }
//    void setSeedAndReset(uint64_t newSeed) {
//        seed_  = newSeed;
//        state_ = newSeed;
//    }
//    double runBipolar() {
//        //Update state using a 64-bit LFSR algorithm.
//        //(If the least significant bit is 1, XOR with the polynomial constant.)
//        (state_ & 1ULL) ? state_ = (state_ >> 1) ^ POLY : state_ >>= 1;
//
//        //Extract 54 bits: shift right by 10 bits then mask.
//        //This gives a value in the range [0, 2^54).
//        //Subtract k2p53int (which is 2^53) to center the value around zero,
//        //then normalize by dividing by k2p53dbl (2^53) to yield a result in [-1, 1).
//        int64_t temp = int64_t((state_ >> 10) & MASK_54BITS) - k2p53int;
//        out_         = double(temp) / k2p53dbl;
//        return out_;
//    }
//    double runUnipolar() {
//        out_ = convert::to_bipolar(runBipolar());
//        return out_;
//    }
//    double out_{};
//
//    //parameters
//    uint64_t seed_{ 0x5555555555555555ULL }; //Default seed with alternating bits for balance
//
//    //coefficients
//    uint64_t state_{ seed_ };                                     //Current state of the LFSR, initialized to seed
//    static constexpr uint64_t POLY{ 0xD800000000000000ULL };      //64-bit polynomial constant (example candidate)
//    static constexpr uint64_t MASK_54BITS{ 0x3FFFFFFFFFFFFFULL }; //54-bit mask for noise extraction
//    static constexpr uint64_t k2p53int{ 9007199254740992ULL };    //2^53
//    static constexpr double k2p53dbl{ 9007199254740992.0 };       //2^53
//};
//
////Linear Congruential Generator from https://audiodev.blog/random-numbers/
//struct LCG64 {
//    LCG64() = default;
//
//    void reset() {
//        state_ = seed_;
//    }
//    void setSeedAndReset(uint64_t v) {
//        seed_  = v;
//        state_ = v;
//    }
//    double runUnipolar() {
//        //Update state using the 64-bit LCG formula
//        state_ = state_ * MULTIPLIER + INCREMENT;
//        //Extract 53 bits (shift by 11 bits, then mask) and normalize to [0, 1)
//        out_ = double((state_ >> 11) & MASK_53BITS) / k2p53dbl; //k2p53flt = 2^53
//        return out_;
//    }
//    double runBipolar() {
//        //Update state using the 64-bit LCG formula
//        state_ = state_ * MULTIPLIER + INCREMENT;
//        //Extract 54 bits (shift by 10 bits, then mask), subtract k2p53int (2^53), and normalize to [-1, 1)
//        int64_t temp = int64_t((state_ >> 10) & MASK_54BITS) - k2p53int; //k2p53int = 2^53
//        out_         = double(temp) / k2p53dbl; //k2p53flt = 2^53
//        return out_;
//    }
//
//    double out_;
//    uint64_t seed_{ 161803398ULL };
//    uint64_t state_{ seed_ };
//
//  private:
//    static constexpr uint64_t MULTIPLIER{ 6364136223846793005ULL };
//    static constexpr uint64_t INCREMENT{ 1442695040888963407ULL };
//    static constexpr uint64_t MASK_53BITS{ 0x1FFFFFFFFFFFFFULL }; //53-bit mask for unipolar output
//    static constexpr uint64_t MASK_54BITS{ 0x3FFFFFFFFFFFFFULL }; //54-bit mask for bipolar output
//    static constexpr uint64_t k2p53int{ 9007199254740992ULL }; //2^53 as an integer
//    static constexpr double k2p53dbl{ 9007199254740992.0 }; //2^53 as a floating point number
//};
////linear congruential generator (LCG)
//struct LCG {
//    //Constructors
//    LCG() = default;
//
//    //io
//    void reset() {
//        engine.seed(initialSeed);
//    }
//    void setState(unsigned int v) {
//        engine.seed(v);
//    }
//    void setSeed(unsigned int v) {
//        initialSeed = v;
//    }
//    void setStateAndSeed(unsigned int v) {
//        initialSeed = v;
//        engine.seed(v);
//    }
//    double runUnipolar() {
//        return out_ = double(engine() - kEngineMin) / DIVISOR;
//    }
//    double runBipolar() {
//        return out_ = convert::to_bipolar(runUnipolar());
//    }
//    double out_;
//
//    //controls
//    unsigned int initialSeed{ 0 }; //The initial seed for reproducibility
//
//    std::minstd_rand engine; //The underlying linear congruential generator (LCG)
//    static constexpr unsigned int kEngineMin{ std::minstd_rand::min() };
//    static constexpr unsigned int kEngineMax{ std::minstd_rand::max() };
//    static constexpr double DIVISOR{ kEngineMax - kEngineMin };
//};
//} //namespace soemdsp::random
