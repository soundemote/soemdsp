#pragma once

#include <algorithm>

#include <soemdsp/runtime/Node.hpp>

namespace soemdsp::runtime::nodes {

struct AudioMultiply : Node {
    AudioMultiply()
    {
        name = "AudioMultiply";

        inputs.resize(2);
        outputs.resize(1);

        inputs[0].type = PortType::Audio;
        inputs[1].type = PortType::Audio;
        outputs[0].type = PortType::Audio;

        inputs[0].direction = PortDirection::Input;
        inputs[1].direction = PortDirection::Input;
        outputs[0].direction = PortDirection::Output;

        inputs[0].name = "A";
        inputs[1].name = "B";
        outputs[0].name = "Out";
    }

    std::string typeName() const override
    {
        return "audio_multiply";
    }

    void process() override
    {
        auto& a = inputs[0];
        auto& b = inputs[1];
        auto& out = outputs[0];

        if (!a.audioBuffer || !b.audioBuffer || !out.audioBuffer)
        {
            return;
        }

        const auto n = std::min({ a.audioFrames, b.audioFrames, out.audioFrames });

        for (std::size_t i = 0; i < n; ++i)
        {
            out.audioBuffer[i] = a.audioBuffer[i] * b.audioBuffer[i];
        }
    }
};

} // namespace soemdsp::runtime::nodes
