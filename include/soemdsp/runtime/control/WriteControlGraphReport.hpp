#pragma once

#include <filesystem>
#include <fstream>
#include <soemdsp/runtime/control/PrintControlGraphReport.hpp>

namespace soemdsp::runtime
{

inline bool writeControlGraphReportTextFile(
  const ControlGraphReport& report,
  const std::filesystem::path& path)
{
    std::ofstream stream(path);
    if (!stream.is_open())
    {
        return false;
    }

    printControlGraphReport(report, stream);
    return stream.good();
}

} // namespace soemdsp::runtime
