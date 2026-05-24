#pragma once

#include <iostream>
#include <ostream>
#include <soemdsp/runtime/validation/ValidationSummary.hpp>

namespace soemdsp::runtime
{

inline void printValidationSummary(
  const ValidationSummary& summary,
  std::ostream& os = std::cout)
{
    os << "[VALIDATION SUMMARY]\n"
       << "messages: "
       << summary.messageCount
       << "\ninfo: "
       << summary.infoCount
       << "\nwarnings: "
       << summary.warningCount
       << "\nerrors: "
       << summary.errorCount
       << "\ngate: "
       << toString(summary.gate)
       << "\nallows execution: "
       << (summary.allowsExecution ? "true" : "false")
       << "\n";
}

} // namespace soemdsp::runtime
