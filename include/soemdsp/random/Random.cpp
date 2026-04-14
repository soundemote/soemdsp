#include "Random.hpp"

namespace soemdsp::random {

    void LCG32::reset() {
        state_ = seed_;
    }
    void LCG32::setSeedAndReset(int v) {
        state_ = seed_ = v;
    }
    double LCG32::runUnipolar() {
        state_ = (1664525 * state_ + 1013904223) & 4294967295;
        out_   = k1z4294967296 * state_;
        return out_;
    }
    double LCG32::runBipolar() {
        state_ = (1664525 * state_ + 1013904223) & 4294967295;
        out_   = k2z4294967296 * state_ - 1.0;
        return out_;
    }

    void LFSR32::reset() {
        state_ = seed_;
    }
    void LFSR32::setSeedAndReset(uint32_t newSeed) {
        seed_  = newSeed;
        state_ = newSeed;
    }
    double LFSR32::runBipolar() {
        // Update state using the LFSR algorithm
        (state_ & 1UL) ? state_ = (state_ >> 1) ^ POLY : state_ >>= 1;

        // Extract a 25-bit value (shift right by 7 and mask),
        // then normalize to the range [-1, 1) by subtracting DIVISOR and dividing by DIVISOR.
        out_ = (double((state_ >> 7) & MASK) - DIVISOR) / DIVISOR;
        return out_;
    }
    double LFSR32::runUnipolar() {
        out_ = biToUni(runBipolar());
        return out_;
    }

    void LCG64::reset() {
        state_ = seed_;
    }
    void LCG64::setSeedAndReset(uint64_t newSeed) {
        seed_  = newSeed;
        state_ = newSeed;
    }
    double LCG64::runBipolar() {
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
    double LCG64::runUnipolar() {
        out_ = biToUni(runBipolar());
        return out_;
    }

    void LFSR64::reset() {
        state_ = seed_;
    }
    void LFSR64::setSeedAndReset(uint64_t v) {
        seed_  = v;
        state_ = v;
    }
    double LFSR64::runUnipolar() {
        // Update state using the 64-bit LCG formula
        state_ = state_ * MULTIPLIER + INCREMENT;
        // Extract 53 bits (shift by 11 bits, then mask) and normalize to [0, 1)
        out_ = double((state_ >> 11) & MASK_53BITS) / k2p53dbl; // k2p53flt = 2^53
        return out_;
    }
    double LFSR64::runBipolar() {
        // Update state using the 64-bit LCG formula
        state_ = state_ * MULTIPLIER + INCREMENT;
        // Extract 54 bits (shift by 10 bits, then mask), subtract k2p53int (2^53), and normalize to [-1, 1)
        int64_t temp = int64_t((state_ >> 10) & MASK_54BITS) - k2p53int; // k2p53int = 2^53
        out_         = double(temp) / k2p53dbl;                          // k2p53flt = 2^53
        return out_;
    }

    void LFSR64::reset() {
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
}
