#pragma once

#include <iostream>
#include <ostream>
#include <soemdsp/runtime/validation/CircuitValidation.hpp>

namespace soemdsp::runtime
{

inline const char* validationSeverityName(ValidationSeverity severity)
{
    switch (severity)
    {
        case ValidationSeverity::Info:
            return "info";
        case ValidationSeverity::Warning:
            return "warning";
        case ValidationSeverity::Error:
            return "error";
    }

    return "info";
}

inline void printCircuitValidation(
  const CircuitValidationReport& report,
  std::ostream& os = std::cout)
{
    os << "[VALIDATION]\n";
    if (report.ok())
    {
        os << "<ok>\n";
        return;
    }

    for (const auto& message : report.messages)
    {
        os << validationSeverityName(message.severity)
           << ": "
           << message.message
           << "\n";
    }
}

} // namespace soemdsp::runtime
