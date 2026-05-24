#pragma once

#include <soemdsp/Phasor.hpp>
#include <soemdsp/oscillator/SineWavetable.hpp>
#include <soemdsp/runtime/Node.hpp>

namespace soemdsp::runtime::nodes {

struct SineOscillator : Node {
    soemdsp::oscillator::Phasor phasor;
    soemdsp::oscillator::SineWavetable<> wavetable;

    SineOscillator()
    {
        name = "SineOscillator";

        inputs.resize(1);
        outputs.resize(1);

        inputs[0].name = "Frequency";
        inputs[0].type = PortType::Float;
        inputs[0].direction = PortDirection::Input;
        inputs[0].value = 440.0f;

        outputs[0].name = "Out";
        outputs[0].type = PortType::Audio;
        outputs[0].direction = PortDirection::Output;

        phasor.setSampleRate(44100.0);
        phasor.setFrequency(440.0);
    }

    void setSampleRate(double sampleRate)
    {
        phasor.setSampleRate(sampleRate);
    }

    void process() override
    {
        auto& frequency = inputs[0];
        auto& out = outputs[0];

        if (!out.audioBuffer)
        {
            return;
        }

        phasor.setFrequency(frequency.value);

        for (std::size_t i = 0; i < out.audioFrames; ++i)
        {
            const auto phase = phasor.getUnipolarValue();
            out.audioBuffer[i] = static_cast<float>(wavetable.sin(phase));
            phasor.increment();
        }
    }
};

} // namespace soemdsp::runtime::nodes
