#pragma once

#include <soemdsp/SampleRate.hpp>
#include <soemdsp/runtime/Node.hpp>
#include <soemdsp/semath.hpp>

namespace soemdsp::runtime::nodes {

struct RampGenerator : Node {
    double phase { 0.0 };

    RampGenerator()
    {
        name = "RampGenerator";
        category = "Generator";

        inputs.resize(1);
        outputs.resize(1);

        inputs[0].name = "Frequency";
        inputs[0].type = PortType::Control;
        inputs[0].direction = PortDirection::Input;
        inputs[0].value = 1.0f;

        outputs[0].name = "Out";
        outputs[0].type = PortType::Audio;
        outputs[0].direction = PortDirection::Output;
    }

    void reset()
    {
        phase = 0.0;
    }

    void process() override
    {
        auto& frequency = inputs[0];
        auto& out = outputs[0];

        if (!out.audioBuffer)
        {
            return;
        }

        const double increment =
            soemdsp::SampleRate::frequencyToIncrement(
                frequency.value);

        for (std::size_t i = 0; i < out.audioFrames; ++i)
        {
            out.audioBuffer[i] =
                static_cast<float>(phase);

            phase =
                soemdsp::math::wrap(phase + increment);
        }
    }
};

} // namespace soemdsp::runtime::nodes
