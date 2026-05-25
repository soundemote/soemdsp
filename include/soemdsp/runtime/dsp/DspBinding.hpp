#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <soemdsp/runtime/Ids.hpp>

namespace soemdsp::runtime
{

enum class DspBindingTargetKind
{
    MemorySlot,
    SemanticSetter,
    InitValue,
    ResetValue,
    ControlInput
};

inline const char* toString(DspBindingTargetKind kind)
{
    switch (kind)
    {
        case DspBindingTargetKind::MemorySlot:
            return "memory_slot";
        case DspBindingTargetKind::SemanticSetter:
            return "semantic_setter";
        case DspBindingTargetKind::InitValue:
            return "init_value";
        case DspBindingTargetKind::ResetValue:
            return "reset_value";
        case DspBindingTargetKind::ControlInput:
            return "control_input";
    }

    return "memory_slot";
}

struct DspParameterBinding
{
    NodeId nodeId{};
    std::string parameterId;
    DspBindingTargetKind targetKind{};
    std::string targetName;
    std::uint32_t memoryOffset{};
    float* memorySlot{};
};

struct DspObjectBinding
{
    NodeId nodeId{};
    std::string objectType;
    std::string objectName;
    std::vector<DspParameterBinding> parameterBindings;
};

} // namespace soemdsp::runtime
