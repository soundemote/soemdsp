#pragma once

#include "ParamType.hpp"
#include "soemdsp.hpp"

#include <fmt/core.h>

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>

namespace soemdsp::plugin {

// clap flags moved to type class
struct ParamData {
    std::string name_{};
    std::string groupName_{}; // optional one level grouping, like VST3, AU and CLAP.
    uint32_t id_{};
    uint32_t idx_{};

    // if this is null then you forgot to call setProtoype
    std::shared_ptr<ParameterTypeBase> parameterType_;

    void setProtoype(std::shared_ptr<ParameterTypeBase> parType) {
        parameterType_ = parType;

        if (parameterType_->type == ParameterTypeBase::Type::DISCRETE) {
            parameterType_->flags |= 1 << 0; // CLAP_PARAM_IS_STEPPED;
        }
    }

    void setGroupName(std::string_view t) {
        groupName_ = t;
    }
    void setId(const uint32_t id, std::string_view name) {
        id_      = id;
        isIdSet_ = true;
        name_    = name;
    }
    void setFlags(const uint32_t f) {
        parameterType_->flags |= f;
    }
    void setDecimalPlaces(int maxDigits, int minDecimalPlaces, int maxDecimal) {
        parameterType_->maxDigits        = maxDigits;
        parameterType_->minDecimalPlaces = minDecimalPlaces;
        parameterType_->minDecimalPlaces = maxDecimal;
    }

    // this value is set when parameters are configured
    bool isIdSet_{ false };
};

} // namespace soemdsp::plugin
