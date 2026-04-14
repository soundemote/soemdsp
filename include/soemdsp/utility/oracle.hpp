#pragma once

#include <random>

/**
 * @brief Source of random numbers.
 * May need refactoring for things like testing with a specific seed.
 */
class Oracle {
  public:
    using result_type = std::mt19937::result_type;

  private:
    std::random_device randomDevice_{};
    result_type initial_value_{ randomDevice_() };
    std::mt19937 generator_{ initial_value_ };

  public:
    constexpr static auto default_seed = std::mt19937::default_seed;

    Oracle()                    = default;
    Oracle(Oracle const& other) = delete;
    Oracle(Oracle&& other)      = delete;
    explicit Oracle(result_type value)
      : initial_value_(value)
      , generator_(value) {}
    ~Oracle() = default;

    /**
     * @brief Get underlying prng object.
     * Usage: static_cast<std::mt19937&>(oracle);
     * @return std::mt19937& prng object.
     */
    explicit operator std::mt19937&() { return generator_; }

    /**
     * @brief Reinitializes the internal state of the random-number engine using new seed value.
     *
     * @param value Seed value to use in the initialization of the internal state.
     */
    void seed(result_type value = default_seed) {
        initial_value_ = value;
        generator_.seed(value);
    }

    /**
     * @brief Get the value the random engine was seeded with.
     */
    [[nodiscard]] result_type GetSeed() const noexcept { return initial_value_; }

    Oracle& operator=(Oracle const& other) = delete;
    Oracle& operator=(Oracle&& other)      = delete;
};
