#pragma once

#include <iostream>
#include <ostream>
#include <soemdsp/runtime/control/PrintControlGraphValidation.hpp>
#include <soemdsp/runtime/control/ValidateControlGraphTargets.hpp>

namespace soemdsp::runtime
{

inline void printControlGraphTargetValidation(
  const ControlGraphTargetValidationReport& report,
  std::ostream& os = std::cout)
{
    os << "[CONTROL GRAPH TARGET VALIDATION]\n"
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
