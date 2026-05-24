#pragma once

#include <iostream>
#include <ostream>
#include <soemdsp/runtime/dsp/PrintDspBindingValidation.hpp>
#include <soemdsp/runtime/dsp/ValidateDspBindingTargets.hpp>

namespace soemdsp::runtime
{

inline void printDspBindingTargetValidation(
  const DspBindingTargetValidationReport& report,
  std::ostream& os = std::cout)
{
    os << "[DSP BINDING TARGET VALIDATION]\n"
       << "summary: "
       << report.infoCount()
       << " info, "
       << report.warningCount()
       << " warnings, "
       << report.errorCount()
       << " errors\n";

    if (report.ok())
    {
        os << "<ok>\n";
        return;
    }

    for (const auto& message : report.messages)
    {
        os << toString(message.severity)
           << ": "
           << message.message
           << "\n";
    }
}

} // namespace soemdsp::runtime
