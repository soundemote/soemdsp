#pragma once

// External-memory noise atoms (mem + base).
// Plain realtime-safe structs; caller owns the memory buffer.
// Not graph nodes. Prefer explicit reset() / *Changed() over callback tables.

#include <cstdint>

namespace soemdsp::random::atom {
namespace detail {

inline constexpr double k_inv_2pow53 = 1.0 / static_cast<double>(1ULL << 53);

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

} // namespace detail

// Slot layout: base+0 = state, base+1 = seed (reset source).
struct SplitMix64
{
    inline std::uint64_t& state_() noexcept { return mem[base + 0]; }
    inline std::uint64_t& seed_() noexcept { return mem[base + 1]; }

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

    // Call after an external writer changes seed_(). Avoids redundant resets.
    inline void syncSeed() noexcept
    {
        if (seed_() != lastSeed_)
        {
            lastSeed_ = seed_();
            reset();
        }
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

    std::uint64_t* mem = nullptr;
    std::uint32_t base{};
    std::uint64_t lastSeed_{};
};

// Slot layout: base+0 = state, base+1 = seed.
struct LCG64
{
    inline std::uint64_t& state_() noexcept { return mem[base + 0]; }
    inline std::uint64_t& seed_() noexcept { return mem[base + 1]; }

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

    inline void syncSeed() noexcept
    {
        if (seed_() != lastSeed_)
        {
            lastSeed_ = seed_();
            reset();
        }
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

    static constexpr std::uint64_t a = 6364136223846793005ULL;
    static constexpr std::uint64_t c = 1442695040888963407ULL;

    std::uint64_t* mem = nullptr;
    std::uint32_t base{};
    std::uint64_t lastSeed_{};
};

// Slot layout: base+0 = state, base+1 = seed.
struct XorShift64Star
{
    inline std::uint64_t& state_() noexcept { return mem[base + 0]; }
    inline std::uint64_t& seed_() noexcept { return mem[base + 1]; }

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
        state_() = seed_();
    }

    inline void syncSeed() noexcept
    {
        if (seed_() != lastSeed_)
        {
            lastSeed_ = seed_();
            reset();
        }
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

    std::uint64_t* mem = nullptr;
    std::uint32_t base{};
    std::uint64_t lastSeed_{};
};

} // namespace soemdsp::random::atom
