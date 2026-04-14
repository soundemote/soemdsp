#pragma once

#include "oracle.hpp"

#include <random>

namespace soemdsp::utility {
struct NoiseGenerator {
  protected:
    static constexpr double unipolar_mean_   = 0.5;
    static constexpr double unipolar_stddev_ = 1. / 6.;

    static constexpr double bipolar_mean_   = 0.0;
    static constexpr double bipolar_stddev_ = 1. / 3.;

    Oracle oracle_{};
    std::normal_distribution<double> gaussian_distribution_{};
    std::uniform_real_distribution<double> uniform_distribution_{};

    using normal_param_type  = std::normal_distribution<double>::param_type;
    using uniform_param_type = std::uniform_real_distribution<double>::param_type;

  public:
    SE_CUSTOM_CONSTRUCTOR(NoiseGenerator)() {
        setSeed(GlobalCounter::get());
    }

    // setting the seed also resets the noise generator
    void setSeed(std::mt19937::result_type seed) {
        oracle_.seed(seed);
    }

    // setting the seed also resets the noise generator
    void reset() {
        oracle_.seed(oracle_.GetSeed());
    }

    /**
     * @brief Gaussian noise centered at 0.5.
     *
     * @return double A random number most likely between 0 and 1.
     */
    double runGaussianUnipolar() {
        return gaussian_distribution_(static_cast<std::mt19937&>(oracle_), normal_param_type{ unipolar_mean_, unipolar_stddev_ });
    }

    /**
     * @brief Gaussian noise centered at 0.
     *
     * @return double A random number most likely between -1 and 1.
     */
    double runGaussianBipolar() {
        return gaussian_distribution_(static_cast<std::mt19937&>(oracle_), normal_param_type{ bipolar_mean_, bipolar_stddev_ });
    }

    double runUnipolar() {
        return uniform_distribution_(static_cast<std::mt19937&>(oracle_), uniform_param_type{ 0.0, 1.0 });
    }

    double runBipolar() {
        return uniform_distribution_(static_cast<std::mt19937&>(oracle_), uniform_param_type{ -1.0, 1.0 });
    }

    // mean: the center number, deviation: width of variance
    double runGaussian(double mean, double deviation) {
        return gaussian_distribution_(static_cast<std::mt19937&>(oracle_), normal_param_type{ mean, deviation });
    }

    double run(double min, double max) {
        return uniform_distribution_(static_cast<std::mt19937&>(oracle_), uniform_param_type{ min, max });
    }
};
} // namespace soemdsp::utility
