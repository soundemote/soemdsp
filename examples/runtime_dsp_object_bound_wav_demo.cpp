#include <cmath>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "WriteMono16Wav.hpp"

#include <soemdsp/runtime/dsp/ApplyDspBinding.hpp>
#include <soemdsp/runtime/dsp/PrintDspBindingApplySummary.hpp>
#include <soemdsp/soemdsp.hpp>

using namespace soemdsp::runtime;
using namespace soemdsp::runtime::nodes;

namespace
{
constexpr int sampleRate = 44100;
constexpr double seconds = 1.0;
constexpr double twoPi = 6.28318530717958647692;

struct TinySineDsp
{
    float* frequency{};
    float* amplitude{};
    double phase{};

    float processSample() noexcept
    {
        const float hz = frequency != nullptr ? *frequency : 440.0f;
        const float amp = amplitude != nullptr ? *amplitude : 0.0f;
        const float sample =
          static_cast<float>(std::sin(phase) * amp);

        phase += twoPi * static_cast<double>(hz) / sampleRate;
        while (phase >= twoPi)
        {
            phase -= twoPi;
        }

        return sample;
    }
};

std::unique_ptr<FloatConstant> createNodeWithParameter(
  NodeId nodeId,
  const char* parameterId,
  const char* name,
  float value,
  float minValue,
  float midValue,
  float maxValue)
{
    auto node = std::make_unique<FloatConstant>(0.0f);
    node->id = nodeId;
    node->name = name;

    Parameter parameter;
    parameter.id = parameterId;
    parameter.name = name;
    parameter.value = value;
    parameter.defaultValue = value;
    parameter.minValue = minValue;
    parameter.midValue = midValue;
    parameter.maxValue = maxValue;
    node->parameters.push_back(parameter);

    return node;
}

Circuit createCircuit()
{
    Circuit circuit;

    circuit.nodes.push_back(
      createNodeWithParameter(
        1,
        "frequency",
        "Frequency",
        220.0f,
        20.0f,
        440.0f,
        2000.0f));

    circuit.nodes.push_back(
      createNodeWithParameter(
        2,
        "amplitude",
        "Amplitude",
        0.35f,
        0.0f,
        0.5f,
        1.0f));

    circuit.prepare();

    return circuit;
}

DspObjectBinding createSineBinding(
  float& frequencyMemory,
  float& amplitudeMemory)
{
    DspObjectBinding binding;
    binding.nodeId = 1;
    binding.objectType = "TinySineDsp";
    binding.objectName = "Tiny Sine DSP";

    binding.parameterBindings.push_back({
      1,
      "frequency",
      DspBindingTargetKind::MemorySlot,
      "frequency",
      0,
      &frequencyMemory });

    binding.parameterBindings.push_back({
      2,
      "amplitude",
      DspBindingTargetKind::MemorySlot,
      "amplitude",
      1,
      &amplitudeMemory });

    return binding;
}

std::vector<float> renderSamples(TinySineDsp& oscillator)
{
    const auto totalFrames =
      static_cast<std::size_t>(sampleRate * seconds);
    std::vector<float> samples;
    samples.reserve(totalFrames);

    for (std::size_t i = 0; i < totalFrames; ++i)
    {
        samples.push_back(oscillator.processSample());
    }

    return samples;
}
} // namespace

int main()
{
    auto circuit = createCircuit();

    float frequencyMemory = 0.0f;
    float amplitudeMemory = 0.0f;
    TinySineDsp oscillator{ &frequencyMemory, &amplitudeMemory };

    const auto binding =
      createSineBinding(frequencyMemory, amplitudeMemory);
    const auto applyResult =
      applyDspParameterBindings(binding, circuit);

    std::cout << "[DSP OBJECT BOUND WAV]\n";
    printDspBindingApplySummary(
      makeDspBindingApplySummary(applyResult));

    if (!applyResult.ok)
    {
        std::cerr << "Binding apply failed; skipping render.\n";
        return 1;
    }

    const auto samples = renderSamples(oscillator);

    const std::string path = "runtime_dsp_object_bound_wav_demo.wav";
    if (!soemdsp::examples::writeMono16Wav(path, samples, sampleRate))
    {
        std::cerr << "Failed to write " << path << "\n";
        return 1;
    }

    std::cout << "frequencyMemory after apply: "
              << frequencyMemory
              << "\n";
    std::cout << "amplitudeMemory after apply: "
              << amplitudeMemory
              << "\n";
    std::cout << "rendered wav: "
              << path
              << "\n";
    std::cout << "sample rate: "
              << sampleRate
              << "\n";
    std::cout << "frames: "
              << samples.size()
              << "\n";
    std::cout << "source: Circuit parameters -> binding -> external memory -> TinySineDsp\n";

    return 0;
}
