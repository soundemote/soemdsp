#pragma once

#include <filesystem>
#include <fstream>
#include <soemdsp/runtime/control/PrintControlGraphSnapshot.hpp>

namespace soemdsp::runtime
{

inline bool writeControlGraphSnapshotTextFile(
  const ControlGraphSnapshot& snapshot,
  const std::filesystem::path& path)
{
    std::ofstream stream(path);
    if (!stream.is_open())
    {
        return false;
    }

    printControlGraphSnapshot(snapshot, stream);
    return stream.good();
}

} // namespace soemdsp::runtime
