#pragma once

#include "soemdsp/Wire.hpp"
#include "soemdsp/additive/HarmonicManager.hpp"
#include <vector>
#include "soemdsp/EnumArray.hpp"

namespace soemdsp::additive {

struct LayerBase {
    virtual void apply() = 0;
    virtual void resize(){};
    std::vector<Wire<double>> mod_;
    HarmonicManager* hm_{};
};

struct PhaseRotate : public LayerBase {
    enum class Par {
        rotation
    };

    SE_CUSTOM_CONSTRUCTOR(PhaseRotate)() {
        mod_.resize(Enum::count<Par>());
    }



    void apply() override {
        for (size_t n = 0; n < hm_->currentSize_; ++n) {
            hm_->phase_[n] += mod_[Enum::sizeT(Par::rotation)];
        }
    }
};

} // namespace soemdsp::additive
