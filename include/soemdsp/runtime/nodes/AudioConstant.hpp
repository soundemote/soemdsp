#pragma once

#include <soemdsp/runtime/Node.hpp>

namespace soemdsp::runtime::nodes {

struct AudioConstant : Node {
    float value { 0.0f };

    AudioConstant(float v = 0.0f)
      : value(v)
    {
        name = "AudioConstant";
        category = "Generator";

        outputs.resize(1);

        outputs[0].name = "Out"; 
        outputs[0].type = PortType::Audio;               
        outputs[0].direction = PortDirection::Output;        
    }

    void process() override
    {
        auto& out = outputs[0];

        if (!out.audioBuffer)
        {
            return;
        }

        for (std::size_t i = 0; i < out.audioFrames; ++i)
        {
            out.audioBuffer[i] = value;
        }
    }
};

} // namespace soemdsp::runtime::nodes
