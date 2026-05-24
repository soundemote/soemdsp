#pragma once

#include <cstdint>

namespace soemdsp::runtime {

enum class PortType : std::uint8_t {
    Float,
    Audio,
    Trigger
};

enum class PortDirection : std::uint8_t {
    Input,
    Output
};

struct Port {
    PortType type { PortType::Float };
    PortDirection direction { PortDirection::Input };

    float value { 0.0f };

    void* backing { nullptr };
};

} // namespace soemdsp::runtime
