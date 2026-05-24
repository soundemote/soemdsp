#pragma once

#include <vector>

#include <soemdsp/utility/EnumArray.hpp>
#include <soemdsp/Wire.hpp>
#include <soemdsp/additive/HarmonicManager.hpp>

namespace soemdsp::additive {

struct LayerBase {
    virtual void apply() = 0;
    virtual void resize(){};
    std::vector<Wire> mod_;
    HarmonicManager* hm_{};
};

struct PhaseRotate : public LayerBase {
    enum class Par {
        rotation
    };

    PhaseRotate() {
        mod_.resize(convert::enumcount_to_sizet<Par>());
    }

    void apply() override {
        for (size_t n = 0; n < hm_->currentSize_; ++n) {
            //hm_->phase_[n] += mod_[magic_enum::enum_index(Par::rotation)];
        }
    }
};

} // namespace soemdsp::additive
