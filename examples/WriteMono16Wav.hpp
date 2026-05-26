#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

namespace soemdsp::examples
{

inline void writeU16(std::ofstream& stream, std::uint16_t value)
{
    const std::array<char, 2> bytes {
        static_cast<char>(value & 0xff),
        static_cast<char>((value >> 8) & 0xff),
    };

    stream.write(bytes.data(), bytes.size());
}

inline void writeU32(std::ofstream& stream, std::uint32_t value)
{
    const std::array<char, 4> bytes {
        static_cast<char>(value & 0xff),
        static_cast<char>((value >> 8) & 0xff),
        static_cast<char>((value >> 16) & 0xff),
        static_cast<char>((value >> 24) & 0xff),
    };

    stream.write(bytes.data(), bytes.size());
}

inline bool writeMono16Wav(
  const std::string& path,
  const std::vector<float>& samples,
  int sampleRate)
{
    constexpr int channelCount = 1;
    constexpr int bitsPerSample = 16;

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
    writeU32(wav, static_cast<std::uint32_t>(sampleRate));
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

} // namespace soemdsp::examples
