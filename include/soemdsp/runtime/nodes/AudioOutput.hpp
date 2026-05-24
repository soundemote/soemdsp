#pragma once

#include <soemdsp/runtime/Node.hpp>

namespace soemdsp::runtime::nodes
{
struct AudioOutput : Node
{
    AudioOutput()
    {
        name = "AudioOutput";
        inputs.emplace_back(PortType::Audio, PortDirection::Input);
        inputs[0].name = "In"; 
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
