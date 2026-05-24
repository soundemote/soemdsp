#include <soemdsp/additive/HarmonicManager.hpp>
#include <soemdsp/additive/layer.hpp>

namespace soemdsp::additive {

void HarmonicManager::run() {

    for (auto& layer : layers_) {
        layer.apply();
    }

    for (size_t n = 0; n < currentSize_; ++n) {
        cycle_[n] = math::wrap(cycle_[n] + increment_[n]);
        outL_ += math::sin(cycle_[n] + phase_[n]) * amplitude_[n];
    }
    outR_ = outL_;
}

inline void HarmonicManager::reset() {
    std::fill(cycle_.begin(), cycle_.begin() + currentSize_, 0.0);
}

inline void HarmonicManager::resize(size_t sz) {
    debug::WITHINRANGE(sz, size_t{}, MAX_HARMONICS);
    if (currentSize_ < sz) {
        // do something specific to growing
    } else if (currentSize_ > sz) {
        // do something specific to shrinking
    }
    currentSize_ = sz;
    for (auto& layer : layers_) {
        layer.resize();
    }
}
} // namespace soemdsp::additive
