#pragma once

#include <filesystem>
#include <fstream>
#include <soemdsp/runtime/control/PrintControlGraphApplyReport.hpp>

namespace soemdsp::runtime
{

inline bool writeControlGraphApplyReportTextFile(
  const ControlGraphApplyReport& report,
  const std::filesystem::path& path)
{
    std::ofstream stream(path);
    if (!stream.is_open())
    {
        return false;
    }

    printControlGraphApplyReport(report, stream);
    return stream.good();
}

} // namespace soemdsp::runtime
