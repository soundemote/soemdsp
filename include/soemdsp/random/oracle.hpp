#pragma once

#include <random>

class Oracle {
  public:
    using result_type = std::mt19937::result_type;

    constexpr static auto default_seed = std::mt19937::default_seed;

    Oracle() = default;

    explicit Oracle(result_type seed)
      : initial_value_(seed)
      , generator_(seed) {}

    // Delete copying
    Oracle(Oracle const&)            = delete;
    Oracle& operator=(Oracle const&) = delete;

    void seed(result_type value = default_seed) {
        initial_value_ = value;
        generator_.seed(value);
    }

    [[nodiscard]] result_type GetSeed() const noexcept {
        return initial_value_;
    }

    // Allow use with distributions: gaussian_distribution_(static_cast<std::mt19937&>(oracle))
    explicit operator std::mt19937&() {
        return generator_;
    }

    explicit operator const std::mt19937&() const {
        return generator_;
    }

  private:
    result_type initial_value_ = std::random_device{}();
    std::mt19937 generator_{ initial_value_ };
};
