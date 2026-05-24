#pragma once

#include <iostream>
#include <ostream>
#include <soemdsp/runtime/dsp/ValidateDspBinding.hpp>

namespace soemdsp::runtime
{

inline const char* toString(DspBindingValidationSeverity severity)
{
    switch (severity)
    {
        case DspBindingValidationSeverity::Info:
            return "info";
        case DspBindingValidationSeverity::Warning:
            return "warning";
        case DspBindingValidationSeverity::Error:
            return "error";
    }

    return "info";
}

inline void printDspBindingValidation(
  const DspBindingValidationReport& report,
  std::ostream& os = std::cout)
{
    os << "[DSP BINDING VALIDATION]\n"
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
