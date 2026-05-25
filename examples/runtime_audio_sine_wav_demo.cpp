#include <algorithm>
#include <array>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <soemdsp/soemdsp.hpp>

namespace
{
constexpr int sampleRate = 44100;
constexpr int channelCount = 1;
constexpr int bitsPerSample = 16;
constexpr float amplitude = 0.35f;
constexpr double seconds = 1.0;

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
    if (!writeMono16Wav(path, rendered))
    {
        std::cerr << "Failed to write " << path << "\n";
        return 1;
    }

    std::cout << "rendered wav: " << path << "\n";
    std::cout << "sample rate: " << sampleRate << "\n";
    std::cout << "frames: " << rendered.size() << "\n";
    std::cout << "channels: " << channelCount << "\n";
    std::cout << "bit depth: " << bitsPerSample << "\n";
    std::cout << "source: runtime Circuit -> SineOscillator -> AudioOutput\n";

    return 0;
}
