#pragma once
// Reference-only low-level DSP object north-star.
// This is not part of the runtime API yet.
// "wires" means externally owned DSP memory, not graph/editor connections.

#include <cstdint>
namespace soemdsp::random
{
namespace detail
{
//coefficients
inline constexpr double k_inv_2pow53 = 1.0 / static_cast<double>(1ULL << 53);
//functions
inline double to_bipolar(std::uint64_t v) noexcept
{
    return static_cast<std::int64_t>(v >> 11) * k_inv_2pow53;
}
inline double to_unipolar(double b) noexcept
{
    return 0.5 * (b + 1.0);
}
inline double to_unipolar(std::uint64_t v) noexcept
{
    return to_unipolar(to_bipolar(v));
}
inline double range(double min, double max, double u) noexcept
{
    return min + (max - min) * u;
}
} //namespace detail
struct SplitMix64
{
    //external DSP memory accessors
    inline std::uint64_t& state_() noexcept { return wires[base + 0]; } //internal state
    inline std::uint64_t& seed_()  noexcept { return wires[base + 1]; } //reset source
    //function
    inline std::uint64_t next() noexcept
    {
        auto& x = state_();
        x += 0x9E3779B97F4A7C15ULL;
        std::uint64_t z = x;
        z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
        z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
        z = z ^ (z >> 31);
        return z;
    }
    inline void reset() noexcept
    {
        state_() = seed_();
    }
    inline double nextBipolar() noexcept
    {
        return detail::to_bipolar(next());
    }
    inline double nextUnipolar() noexcept
    {
        return detail::to_unipolar(next());
    }
    inline double nextRange(double min, double max) noexcept
    {
        return detail::range(min, max, nextUnipolar());
    }
    //memory
    std::uint64_t* wires = nullptr;
    std::uint32_t base{};
};
struct LCG64
{
    //external DSP memory accessors
    inline std::uint64_t& state_() noexcept { return wires[base + 0]; } //internal state
    inline std::uint64_t& seed_() noexcept { return wires[base + 1]; } //reset source
    //function
    inline std::uint64_t next() noexcept
    {
        auto& x = state_();
        x = x * a + c;
        return x;
    }
    inline void reset() noexcept
    {
        state_() = seed_();
    }
    inline double nextBipolar() noexcept
    {
        return detail::to_bipolar(next());
    }
    inline double nextUnipolar() noexcept
    {
        return detail::to_unipolar(next());
    }
    inline double nextRange(double min, double max) noexcept
    {
        return detail::range(min, max, nextUnipolar());
    }
    //coefficient
    static constexpr std::uint64_t a = 6364136223846793005ULL;
    static constexpr std::uint64_t c = 1442695040888963407ULL;
    //memory
    std::uint64_t* wires = nullptr;
    std::uint32_t base{};
};
struct XorShift64Star
{
    //external DSP memory accessors
    inline std::uint64_t& state_() noexcept { return wires[base + 0]; } //internal state
    inline std::uint64_t& seed_() noexcept { return wires[base + 1]; } //reset source
    //function
    inline std::uint64_t next() noexcept
    {
        auto& x = state_();
        x ^= x >> 12;
        x ^= x << 25;
        x ^= x >> 27;
        x = x * 2685821657736338717ULL;
        return x;
    }
    inline void reset() noexcept
    {
        auto& x = state_();
        auto& s = seed_();
        x = s;
    }
    inline double nextBipolar() noexcept
    {
        return detail::to_bipolar(next());
    }
    inline double nextUnipolar() noexcept
    {
        return detail::to_unipolar(next());
    }
    inline double nextRange(double min, double max) noexcept
    {
        return detail::range(min, max, nextUnipolar());
    }
    //memory
    std::uint64_t* wires = nullptr;
    std::uint32_t base{};
};
} //namespace soemdsp::random
