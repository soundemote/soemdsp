#pragma once

#include <filesystem>
#include <fstream>
#include <soemdsp/runtime/dsp/PrintDspBindingApplyResult.hpp>

namespace soemdsp::runtime
{

inline bool writeDspBindingApplyResultTextFile(
  const DspBindingApplyResult& result,
  const std::filesystem::path& path)
{
    std::ofstream stream(path);
    if (!stream.is_open())
    {
        return false;
    }

    printDspBindingApplyResult(result, stream);
    return true;
}

} // namespace soemdsp::runtime
