#pragma once

#include "soemdsp/semath.hpp"

#include <iostream>
#include <string>
#include <functional>
#include <cstdint>
#include <vector>

namespace soemdsp::utility {
struct Hash {
    // Returns a hash value for given container of integers.
    template<typename Container>
    static uint32_t fromInts(const Container& integers) {
        uint32_t hash_sum{ 1 };
        for (uint32_t i : integers) {
            hash_sum *= 53 + hash32shift(i);
        }
        return hash_sum;
    }

    // Returns a hash for given string using std::hash
    static uint32_t fromString(const std::string& str) {
        std::hash<std::string> hasher;
        return static_cast<uint32_t>(hasher(str) % std::numeric_limits<uint32_t>::max());
    }

    // Returns a final hash value by combining given string and container of integers. Set the contents of integers_ and characters_ before calling. 
    uint32_t calculate() {
        out_ = (kPrime + fromString(characters_)) * fromInts(integers_);
        return out_;
    }
    uint32_t get() {
        return out_;
    }

    std::vector<uint32_t> integers_;
    std::string characters_;
    uint32_t out_{};

  private:
    static const uint32_t kPrime{ 104729 };
    static uint32_t hash32shift(uint32_t a) {
        a = (a + 0x7ed55d16) + (a << 12);
        a = (a ^ 0xc761c23c) ^ (a >> 19);
        a = (a + 0x165667b1) + (a << 5);
        a = (a + 0xd3a2646c) ^ (a << 9);
        a = (a + 0xfd7046c5) + (a << 3);
        a = (a ^ 0xb55a4f09) ^ (a >> 16);
        return a;
    }
};

struct Hash2 { // UNTESTED from ChatGPT
    template<typename Container>
    static size_t fromInts(const Container& integers) {
        size_t seed = 0;
        for (const auto& i : integers) {
            hashCombine(seed, i);
        }
        return seed;
    }

    static size_t fromString(const std::string& str) {
        return std::hash<std::string>{}(str);
    }

    size_t calculate() {
        size_t seed = fromString(characters_);
        seed ^= fromInts(integers_) + kHashMixConstant + (seed << 6) + (seed >> 2);
        out_ = seed;
        return out_;
    }

    size_t get() const {
        return out_;
    }

    std::vector<size_t> integers_;
    std::string characters_;
    size_t out_; 

  private:
    template<typename T>
    static void hashCombine(size_t& seed, const T& value) {
        seed ^= std::hash<T>{}(value) + kHashMixConstant + (seed << 6) + (seed >> 2);
    }
    static const size_t kHashMixConstant{ 2654435769 }; // round(2^32 * 1.0/GoldenRatio)
};
} // namespace soemdsp
