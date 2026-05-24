#pragma once

#include <filesystem>
#include <fstream>
#include <soemdsp/runtime/serialization/PrintConnections.hpp>

namespace soemdsp::runtime
{

inline bool writeConnectionsTextFile(
  const CircuitSnapshot& snapshot,
  const std::filesystem::path& path)
{
    std::ofstream stream(path);
    if (!stream.is_open())
    {
        return false;
    }

    printConnectionsText(snapshot, stream);
    return stream.good();
}

} // namespace soemdsp::runtime
