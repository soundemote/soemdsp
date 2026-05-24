#pragma once

#include <algorithm>

#include <soemdsp/runtime/Node.hpp>

namespace soemdsp::runtime::nodes {

struct AudioMultiplyAdd : Node {
    AudioMultiplyAdd()
    {
        name = "AudioMultiplyAdd";

        inputs.resize(3);
        outputs.resize(1);

        inputs[0].name = "Value";
        inputs[1].name = "Scale";
        inputs[2].name = "Offset";
        outputs[0].name = "Out";

        inputs[0].type = PortType::Audio;
        inputs[1].type = PortType::Float;
        inputs[2].type = PortType::Float;
        outputs[0].type = PortType::Audio;

        inputs[0].direction = PortDirection::Input;
        inputs[1].direction = PortDirection::Input;
        inputs[2].direction = PortDirection::Input;
        outputs[0].direction = PortDirection::Output;

        inputs[1].value = 1.0f;
    }

    void process() override
    {
        auto& value  = inputs[0];
        auto& scale  = inputs[1];
        auto& offset = inputs[2];
        auto& out    = outputs[0];

        if (!value.audioBuffer || !out.audioBuffer)
        {
            return;
        }

        const auto n = std::min(value.audioFrames, out.audioFrames);

        for (std::size_t i = 0; i < n; ++i)
        {
            out.audioBuffer[i] =
                (value.audioBuffer[i] * scale.value) + offset.value;
        }
    }
};

} // namespace soemdsp::runtime::nodes
