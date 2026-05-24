#pragma once

#include <random>

#include "oracle.hpp"

namespace soemdsp::utility {

struct NoiseGenerator {
    NoiseGenerator() {
        setSeed(Oracle::default_seed); // or GlobalCounter::get() if you prefer
    }

    Oracle oracle_;

    std::normal_distribution<double> gaussian_distribution_;
    std::uniform_real_distribution<double> uniform_distribution_;

    // Common distribution parameters
    double unipolar_mean_   = 0.5;
    double unipolar_stddev_ = 0.2;

    double bipolar_mean_   = 0.0;
    double bipolar_stddev_ = 0.5;

    // ====================== SEED CONTROL ======================

    void setSeed(Oracle::result_type seed) {
        oracle_.seed(seed);
    }

    void reset() {
        oracle_.seed(oracle_.GetSeed());
    }

    // ====================== RANDOM FUNCTIONS ======================

    double runGaussianUnipolar() {
        return gaussian_distribution_(static_cast<std::mt19937&>(oracle_), std::normal_distribution<double>::param_type{ unipolar_mean_, unipolar_stddev_ });
    }

    double runGaussianBipolar() {
        return gaussian_distribution_(static_cast<std::mt19937&>(oracle_), std::normal_distribution<double>::param_type{ bipolar_mean_, bipolar_stddev_ });
    }

    double runUnipolar() {
        return uniform_distribution_(static_cast<std::mt19937&>(oracle_), std::uniform_real_distribution<double>::param_type{ 0.0, 1.0 });
    }

    double runBipolar() {
        return uniform_distribution_(static_cast<std::mt19937&>(oracle_), std::uniform_real_distribution<double>::param_type{ -1.0, 1.0 });
    }

    double runGaussian(double mean, double deviation) {
        return gaussian_distribution_(static_cast<std::mt19937&>(oracle_), std::normal_distribution<double>::param_type{ mean, deviation });
    }

    double run(double min, double max) {
        return uniform_distribution_(static_cast<std::mt19937&>(oracle_), std::uniform_real_distribution<double>::param_type{ min, max });
    }
};

} // namespace soemdsp::utility
