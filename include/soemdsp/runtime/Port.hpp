#pragma once

#include <cstdint>
#include <cstddef>
#include <string>

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
    std::string name;

    float value { 0.0f };

    float* audioBuffer { nullptr };
    std::size_t audioFrames { 0 };

    void* backing { nullptr };
};

} // namespace soemdsp::runtime
