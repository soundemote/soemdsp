#pragma once

#include <filesystem>
#include <fstream>
#include <soemdsp/runtime/dsp/PrintDspBindingReport.hpp>

namespace soemdsp::runtime
{

inline bool writeDspBindingReportTextFile(
  const DspBindingReport& report,
  const std::filesystem::path& path)
{
    std::ofstream stream(path);
    if (!stream.is_open())
    {
        return false;
    }

    printDspBindingReport(report, stream);
    return stream.good();
}

} // namespace soemdsp::runtime
