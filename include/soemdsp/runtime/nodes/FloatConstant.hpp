#pragma once

#include <soemdsp/runtime/Node.hpp>

namespace soemdsp::runtime::nodes {

struct FloatConstant : Node {
    float constant { 0.0f };

    FloatConstant(float v = 0.0f)
      : constant(v)
    {
        name = "FloatConstant";
        category = "Generator";
        description = "Outputs a constant control value";

        outputs.resize(1);

        outputs[0].name = "Value";
        outputs[0].type = PortType::Control;
        outputs[0].direction = PortDirection::Output;
    }

    std::string typeName() const override
    {
        return "float_constant";
    }

    void process() override
    {
        outputs[0].value = constant;
    }
};

} // namespace soemdsp::runtime::nodes
