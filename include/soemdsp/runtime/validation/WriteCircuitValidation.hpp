#pragma once

#include <filesystem>
#include <fstream>
#include <soemdsp/runtime/validation/PrintCircuitValidation.hpp>

namespace soemdsp::runtime
{

inline bool writeCircuitValidationTextFile(
  const CircuitValidationReport& report,
  const std::filesystem::path& path)
{
    std::ofstream stream(path);
    if (!stream.is_open())
    {
        return false;
    }

    printCircuitValidation(report, stream);
    return stream.good();
}

} // namespace soemdsp::runtime
