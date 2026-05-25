#pragma once

#include <filesystem>
#include <fstream>
#include <soemdsp/runtime/dsp/PrintDspBlockPhaseReport.hpp>

namespace soemdsp::runtime
{

inline bool writeDspBlockPhaseReportTextFile(
  const DspBlockPhaseReport& report,
  const std::filesystem::path& path)
{
    std::ofstream stream(path);
    if (!stream.is_open())
    {
        return false;
    }

    printDspBlockPhaseReport(report, stream);
    return true;
}

} // namespace soemdsp::runtime

