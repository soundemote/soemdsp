#pragma once

#include <iostream>

#include <soemdsp/runtime/Node.hpp>

namespace soemdsp::runtime::nodes
{

struct TriggerPrinter : Node
{
    TriggerPrinter()
    {
        name = "TriggerPrinter";
        category = "Debug";
        description = "Prints trigger events to console";

        inputs.emplace_back(
            PortType::Trigger,
            PortDirection::Input
        );

        inputs[0].name = "Trigger";
    }

    void process() override
    {
        if (inputs[0].triggered)
        {
            std::cout << "[TRIGGER]\n";
        }
    }
};

} // namespace soemdsp::runtime::nodes
