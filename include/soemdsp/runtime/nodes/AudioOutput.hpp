//AudioOutput.hpp
#pragma once

#include <soemdsp/runtime/Node.hpp>

namespace soemdsp::runtime::nodes
{
struct AudioOutput : Node
{
    AudioOutput()
    {
        inputs.emplace_back(
          PortType::Audio,
          PortDirection::Input);
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
