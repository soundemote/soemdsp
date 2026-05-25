#pragma once

#include <filesystem>
#include <fstream>
#include <soemdsp/runtime/dsp/PrintDspBindingApplySummary.hpp>

namespace soemdsp::runtime
{

inline bool writeDspBindingApplySummaryTextFile(
  const DspBindingApplySummary& summary,
  const std::filesystem::path& path)
{
    std::ofstream stream(path);
    if (!stream.is_open())
    {
        return false;
    }

    printDspBindingApplySummary(summary, stream);
    return true;
}

} // namespace soemdsp::runtime
