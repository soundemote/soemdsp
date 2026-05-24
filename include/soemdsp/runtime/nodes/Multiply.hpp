#pragma once

#include <soemdsp/runtime/Node.hpp>

namespace soemdsp::runtime::nodes {

struct Multiply : Node {
    Multiply()
    {
        name = "Multiply";

        inputs.resize(2);
        outputs.resize(1);

        inputs[0].type = PortType::Control;
        inputs[1].type = PortType::Control;

        inputs[0].direction = PortDirection::Input;
        inputs[1].direction = PortDirection::Input;

        outputs[0].type = PortType::Control;
        outputs[0].direction = PortDirection::Output;
    }

    std::string typeName() const override
    {
        return "multiply";
    }

    void process() override
    {
        outputs[0].value = inputs[0].value * inputs[1].value;
    }
};

} // namespace soemdsp::runtime::nodes
