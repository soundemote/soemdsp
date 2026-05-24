#pragma once

#include <iostream>
#include <ostream>
#include <soemdsp/runtime/report/CircuitReport.hpp>
#include <soemdsp/runtime/serialization/PrintCircuitSnapshot.hpp>
#include <soemdsp/runtime/validation/PrintCircuitValidation.hpp>
#include <soemdsp/runtime/validation/PrintValidationSummary.hpp>

namespace soemdsp::runtime
{

inline void printCircuitReport(
  const CircuitReport& report,
  std::ostream& os = std::cout)
{
    os << "[CIRCUIT REPORT]\n";
    printCircuitSnapshot(report.snapshot, os);
    os << "\n";
    printCircuitValidation(report.validation, os);
    printValidationSummary(report.validationSummary, os);
}

} // namespace soemdsp::runtime
