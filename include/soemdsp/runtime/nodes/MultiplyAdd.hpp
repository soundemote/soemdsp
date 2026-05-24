#pragma once

#include <soemdsp/runtime/Node.hpp>

namespace soemdsp::runtime::nodes {

struct MultiplyAdd : Node {
    MultiplyAdd()
    {
        name = "MultiplyAdd";

        inputs.resize(3);
        outputs.resize(1);

        inputs[0].name = "Value";
        inputs[1].name = "Scale";
        inputs[2].name = "Offset";

        outputs[0].name = "Out";

        for (auto& input : inputs)
        {
            input.type = PortType::Float;
            input.direction = PortDirection::Input;
        }

        outputs[0].type = PortType::Float;
        outputs[0].direction = PortDirection::Output;

        inputs[1].value = 1.0f;
    }

    void process() override
    {
        outputs[0].value =
            (inputs[0].value * inputs[1].value)
            + inputs[2].value;
    }
};

} // namespace soemdsp::runtime::nodes
