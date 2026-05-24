#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>
#include <soemdsp/runtime/Ids.hpp>

namespace soemdsp::runtime
{

enum class ControlNodeKind
{
    MacroKnob,
    Curve,
    Clamp01,
    Scale,
    Invert,
    Smooth,
    ParameterTarget,
    Split
};

struct ControlParameterTarget
{
    NodeId nodeId{};
    std::string parameterId;
};

struct ControlNode
{
    std::uint64_t id{};
    ControlNodeKind kind{};
    std::string name;
    std::string description;
    float editorX{};
    float editorY{};
    std::optional<ControlParameterTarget> parameterTarget;
};

struct ControlConnection
{
    std::uint64_t sourceNodeId{};
    std::string sourcePortId;
    std::uint64_t destinationNodeId{};
    std::string destinationPortId;
};

struct ControlGraph
{
    std::vector<ControlNode> nodes;
    std::vector<ControlConnection> connections;
};

inline const char* toString(ControlNodeKind kind)
{
    switch (kind)
    {
        case ControlNodeKind::MacroKnob:
            return "macro_knob";
        case ControlNodeKind::Curve:
            return "curve";
        case ControlNodeKind::Clamp01:
            return "clamp01";
        case ControlNodeKind::Scale:
            return "scale";
        case ControlNodeKind::Invert:
            return "invert";
        case ControlNodeKind::Smooth:
            return "smooth";
        case ControlNodeKind::ParameterTarget:
            return "parameter_target";
        case ControlNodeKind::Split:
            return "split";
    }

    return "unknown";
}

} // namespace soemdsp::runtime
