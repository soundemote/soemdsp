#pragma once

#include <soemdsp/runtime/Node.hpp>

namespace soemdsp::runtime::nodes
{

struct TriggerEdge : Node
{
    bool previous{ false };

    TriggerEdge()
    {
        name = "TriggerEdge";
        category = "Trigger";
        description = "Detects trigger rising edges";

        inputs.emplace_back(
            PortType::Trigger,
            PortDirection::Input
        );

        outputs.emplace_back(
            PortType::Trigger,
            PortDirection::Output
        );

        inputs[0].name = "In";
        outputs[0].name = "Out";
    }

    std::string typeName() const override
    {
        return "trigger_edge";
    }

    void process() override
    {
        const bool current =
            inputs[0].triggered;

        outputs[0].triggered =
            current && !previous;

        previous = current;
    }
};

} // namespace soemdsp::runtime::nodes
