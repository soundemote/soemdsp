#pragma once

#include <iostream>
#include <ostream>
#include <soemdsp/runtime/control/ValidateControlGraph.hpp>

namespace soemdsp::runtime
{

inline const char* toString(ControlGraphValidationSeverity severity)
{
    switch (severity)
    {
        case ControlGraphValidationSeverity::Info:
            return "info";
        case ControlGraphValidationSeverity::Warning:
            return "warning";
        case ControlGraphValidationSeverity::Error:
            return "error";
    }

    return "info";
}

inline void printControlGraphValidation(
  const ControlGraphValidationReport& report,
  std::ostream& os = std::cout)
{
    os << "[CONTROL GRAPH VALIDATION]\n"
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
