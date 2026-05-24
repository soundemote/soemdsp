#pragma once

#include <filesystem>
#include <fstream>
#include <soemdsp/runtime/serialization/PrintCircuitSnapshot.hpp>

namespace soemdsp::runtime
{

inline bool writeCircuitSnapshotTextFile(
  const CircuitSnapshot& snapshot,
  const std::filesystem::path& path)
{
    std::ofstream stream(path);
    if (!stream.is_open())
    {
        return false;
    }

    printCircuitSnapshot(snapshot, stream);
    return stream.good();
}

} // namespace soemdsp::runtime
