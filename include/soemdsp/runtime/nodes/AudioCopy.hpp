#pragma once

#include <soemdsp/runtime/Circuit.hpp>
#include <soemdsp/runtime/Node.hpp>

namespace soemdsp::runtime::nodes
{

struct AudioCopy : Node
{
    AudioCopy()
    {
        inputs.emplace_back(PortType::Audio);
        outputs.emplace_back(PortType::Audio);
    }

    void process() override
    {
        auto* input  = inputs[0].audioBuffer;
        auto* output = outputs[0].audioBuffer;

        for (std::size_t i = 0; i < Circuit::blockSize; ++i)
        {
            output[i] = input[i];
        }
    }
};

} // namespace soemdsp::runtime::nodes
