#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <ostream>
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

struct Mono16WavWriteReport
{
    bool wrote{};
    std::string path;
    int sampleRate{};
    int channelCount{ 1 };
    int bitsPerSample{ 16 };
    std::size_t frames{};
    std::size_t dataBytes{};
    std::size_t fileBytes{};
};

inline Mono16WavWriteReport writeMono16WavWithReport(
  const std::string& path,
  const std::vector<float>& samples,
  int sampleRate)
{
    Mono16WavWriteReport report;
    report.path = path;
    report.sampleRate = sampleRate;
    report.frames = samples.size();
    report.dataBytes = samples.size() * sizeof(std::int16_t);
    report.fileBytes = 44 + report.dataBytes;
    report.wrote = writeMono16Wav(path, samples, sampleRate);

    return report;
}

inline void printMono16WavWriteReport(
  const Mono16WavWriteReport& report,
  std::ostream& os = std::cout)
{
    os << "[MONO 16 WAV WRITE REPORT]\n"
       << "wrote: "
       << (report.wrote ? "true" : "false")
       << "\n"
       << "path: "
       << report.path
       << "\n"
       << "sample rate: "
       << report.sampleRate
       << "\n"
       << "channels: "
       << report.channelCount
       << "\n"
       << "bit depth: "
       << report.bitsPerSample
       << "\n"
       << "frames: "
       << report.frames
       << "\n"
       << "data bytes: "
       << report.dataBytes
       << "\n"
       << "file bytes: "
       << report.fileBytes
       << "\n";
}

inline bool writeMono16WavWriteReportTextFile(
  const Mono16WavWriteReport& report,
  const std::string& path)
{
    std::ofstream stream(path);
    if (!stream.is_open())
    {
        return false;
    }

    printMono16WavWriteReport(report, stream);
    return true;
}

} // namespace soemdsp::examples
