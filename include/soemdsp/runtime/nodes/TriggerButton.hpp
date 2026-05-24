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

        outputs.emplace_back(
            PortType::Trigger,
            PortDirection::Output
        );

        outputs[0].name = "Trigger";
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
