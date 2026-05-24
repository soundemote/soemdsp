#pragma once

#include <soemdsp/runtime/Node.hpp>

namespace soemdsp::runtime::nodes
{
struct AudioOutput : Node
{
    AudioOutput()
    {
        name = "AudioOutput";
        category = "Output";
        description = "Final audio sink";
        inputs.emplace_back(PortType::Audio, PortDirection::Input);
        inputs[0].name = "In"; 
        inputs[0].description = "Final audio signal";
    }

    void process() override
    {
    //sink node
    }
    float* output() noexcept
    {
        return inputs[0].audioBuffer;
    }
};
} //namespace soemdsp::runtime::nodes
