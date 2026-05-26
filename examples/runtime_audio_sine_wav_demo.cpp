#include <algorithm>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "WriteMono16Wav.hpp"

#include <soemdsp/soemdsp.hpp>

namespace
{
constexpr int sampleRate = 44100;
constexpr float amplitude = 0.35f;
constexpr double seconds = 1.0;
} // namespace

int main()
{
    using namespace soemdsp::runtime;
    using namespace soemdsp::runtime::nodes;

    soemdsp::SampleRate::Update(sampleRate, Circuit::blockSize);

    Circuit circuit;

    auto oscillator = std::make_unique<SineOscillator>();
    auto output = std::make_unique<AudioOutput>();

    oscillator->inputs[0].value = 440.0f;

    auto* oscillatorOut = &oscillator->outputs[0];
    auto* outputIn = &output->inputs[0];

    circuit.nodes.push_back(std::move(oscillator));
    circuit.nodes.push_back(std::move(output));

    circuit.connect(
      *circuit.nodes[0],
      *oscillatorOut,
      *circuit.nodes[1],
      *outputIn);

    circuit.output = outputIn;
    circuit.prepare();

    const auto totalFrames =
      static_cast<std::size_t>(sampleRate * seconds);
    std::vector<float> rendered;
    rendered.reserve(totalFrames);

    while (rendered.size() < totalFrames)
    {
        circuit.process();

        const float* block = circuit.outputBuffer();
        if (!block)
        {
            std::cerr << "No output buffer available.\n";
            return 1;
        }

        const std::size_t remaining = totalFrames - rendered.size();
        const std::size_t framesToCopy =
          std::min<std::size_t>(Circuit::blockSize, remaining);

        for (std::size_t i = 0; i < framesToCopy; ++i)
        {
            rendered.push_back(block[i] * amplitude);
        }
    }

    const std::string path = "runtime_audio_sine_wav_demo.wav";
    const auto wavReport =
      soemdsp::examples::writeMono16WavWithReport(
        path,
        rendered,
        sampleRate);
    soemdsp::examples::printMono16WavWriteReport(wavReport);
    const auto wroteReport =
      soemdsp::examples::writeMono16WavWriteReportTextFile(
        wavReport,
        "runtime_audio_sine_wav_demo.wav.txt");
    std::cout << "wav report file written: "
              << (wroteReport ? "true" : "false")
              << "\n";

    if (!wavReport.wrote)
    {
        std::cerr << "Failed to write " << path << "\n";
        return 1;
    }

    std::cout << "source: runtime Circuit -> SineOscillator -> AudioOutput\n";

    return 0;
}
