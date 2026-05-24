#pragma once

#include <filesystem>
#include <fstream>
#include <soemdsp/runtime/report/PrintCircuitReport.hpp>

namespace soemdsp::runtime
{

inline bool writeCircuitReportTextFile(
  const CircuitReport& report,
  const std::filesystem::path& path)
{
    std::ofstream stream(path);
    if (!stream.is_open())
    {
        return false;
    }

    printCircuitReport(report, stream);
    return stream.good();
}

} // namespace soemdsp::runtime
