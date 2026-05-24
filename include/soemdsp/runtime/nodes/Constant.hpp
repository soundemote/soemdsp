#pragma once

#include <soemdsp/runtime/Node.hpp>

namespace soemdsp::runtime::nodes {

struct Constant : Node {
    Constant(float v = 0.0f)
    {
        name = "Constant";

        outputs.resize(1);
        outputs[0].type = PortType::Control;
        outputs[0].direction = PortDirection::Output;
        outputs[0].value = v;
    }

    std::string typeName() const override
    {
        return "constant";
    }

    void set(float v)
    {
        outputs[0].value = v;
    }
};

} // namespace soemdsp::runtime::nodes
