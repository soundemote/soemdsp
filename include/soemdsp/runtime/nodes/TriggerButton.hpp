#pragma once

#include <soemdsp/runtime/Node.hpp>

namespace soemdsp::runtime::nodes
{

struct TriggerButton : Node
{
    bool pressed{ false };

    TriggerButton()
    {
        name = "TriggerButton";
        category = "Trigger";
        description = "Manual trigger source";

        outputs.emplace_back(
            PortType::Trigger,
            PortDirection::Output
        );

        outputs[0].name = "Trigger";
        outputs[0].description = "Manual trigger pulse";
    }

    std::string typeName() const override
    {
        return "trigger_button";
    }

    void trigger()
    {
        pressed = true;
    }

    void process() override
    {
        outputs[0].triggered = pressed;

        pressed = false;
    }
};

} // namespace soemdsp::runtime::nodes
