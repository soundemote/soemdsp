#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <soemdsp/runtime/dsp/ApplyDspBinding.hpp>
#include <soemdsp/runtime/dsp/PrintDspBindingApplySummary.hpp>
#include <soemdsp/soemdsp.hpp>

using namespace soemdsp::runtime;
using namespace soemdsp::runtime::nodes;

namespace
{
constexpr int sampleRate = 44100;
constexpr int channelCount = 1;
constexpr int bitsPerSample = 16;
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

void writeU16(std::ofstream& stream, std::uint16_t value)
{
    const std::array<char, 2> bytes {
        static_cast<char>(value & 0xff),
        static_cast<char>((value >> 8) & 0xff),
    };

    stream.write(bytes.data(), bytes.size());
}

void writeU32(std::ofstream& stream, std::uint32_t value)
{
    const std::array<char, 4> bytes {
        static_cast<char>(value & 0xff),
        static_cast<char>((value >> 8) & 0xff),
        static_cast<char>((value >> 16) & 0xff),
        static_cast<char>((value >> 24) & 0xff),
    };

    stream.write(bytes.data(), bytes.size());
}

bool writeMono16Wav(const std::string& path, const std::vector<float>& samples)
{
    std::ofstream wav(path, std::ios::binary);
    if (!wav)
    {
        return false;
    }

    const auto dataBytes =
      static_cast<std::uint32_t>(samples.size() * sizeof(std::int16_t));
    const auto byteRate =
      static_cast<std::uint32_t>(sampleRate * channelCount * bitsPerSample / 8);
    const auto blockAlign =
      static_cast<std::uint16_t>(channelCount * bitsPerSample / 8);

    wav.write("RIFF", 4);
    writeU32(wav, 36 + dataBytes);
    wav.write("WAVE", 4);

    wav.write("fmt ", 4);
    writeU32(wav, 16);
    writeU16(wav, 1);
    writeU16(wav, channelCount);
    writeU32(wav, sampleRate);
    writeU32(wav, byteRate);
    writeU16(wav, blockAlign);
    writeU16(wav, bitsPerSample);

    wav.write("data", 4);
    writeU32(wav, dataBytes);

    for (const float sample : samples)
    {
        const float clipped = std::clamp(sample, -1.0f, 1.0f);
        const auto pcm =
          static_cast<std::int16_t>(clipped * 32767.0f);

        writeU16(wav, static_cast<std::uint16_t>(pcm));
    }

    return static_cast<bool>(wav);
}

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
        0.2f,
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

void renderFrames(
  TinySineDsp& oscillator,
  std::vector<float>& samples,
  std::size_t frameCount)
{
    for (std::size_t i = 0; i < frameCount; ++i)
    {
        samples.push_back(oscillator.processSample());
    }
}

void printApplySummary(
  const char* label,
  const DspBindingApplyResult& result)
{
    std::cout << label
              << "\n";
    printDspBindingApplySummary(
      makeDspBindingApplySummary(result));
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
    const auto totalFrames =
      static_cast<std::size_t>(sampleRate * seconds);
    const auto halfFrames = totalFrames / 2;

    std::vector<float> samples;
    samples.reserve(totalFrames);

    std::cout << "[DSP OBJECT BOUND WAV RESYNC]\n";

    const auto firstApply =
      applyDspParameterBindings(binding, circuit);
    printApplySummary("[FIRST APPLY]", firstApply);
    if (!firstApply.ok)
    {
        std::cerr << "First binding apply failed; skipping render.\n";
        return 1;
    }

    const float firstFrequency = frequencyMemory;
    const float firstAmplitude = amplitudeMemory;
    renderFrames(oscillator, samples, halfFrames);

    const auto frequencyChanged =
      circuit.setParameterValue(1, "frequency", 440.0f);
    const auto amplitudeChanged =
      circuit.setParameterValue(2, "amplitude", 0.35f);

    std::cout << "set frequency returned: "
              << (frequencyChanged ? "true" : "false")
              << "\n";
    std::cout << "set amplitude returned: "
              << (amplitudeChanged ? "true" : "false")
              << "\n";

    const auto secondApply =
      applyDspParameterBindings(binding, circuit);
    printApplySummary("[SECOND APPLY]", secondApply);
    if (!secondApply.ok)
    {
        std::cerr << "Second binding apply failed; skipping render.\n";
        return 1;
    }

    const float secondFrequency = frequencyMemory;
    const float secondAmplitude = amplitudeMemory;
    renderFrames(oscillator, samples, totalFrames - samples.size());

    const std::string path = "runtime_dsp_object_bound_wav_resync_demo.wav";
    if (!writeMono16Wav(path, samples))
    {
        std::cerr << "Failed to write " << path << "\n";
        return 1;
    }

    std::cout << "first half frequency: "
              << firstFrequency
              << "\n";
    std::cout << "first half amplitude: "
              << firstAmplitude
              << "\n";
    std::cout << "second half frequency: "
              << secondFrequency
              << "\n";
    std::cout << "second half amplitude: "
              << secondAmplitude
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
    std::cout << "source: Circuit parameter changes -> binding resync -> external memory -> TinySineDsp\n";

    return 0;
}
