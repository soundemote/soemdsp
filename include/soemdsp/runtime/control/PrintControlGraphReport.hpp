#pragma once

#include <iostream>
#include <ostream>
#include <soemdsp/runtime/control/ControlGraphReport.hpp>
#include <soemdsp/runtime/control/PrintControlGraphSnapshot.hpp>
#include <soemdsp/runtime/control/PrintControlGraphValidation.hpp>

namespace soemdsp::runtime
{

inline void printControlGraphReport(
  const ControlGraphReport& report,
  std::ostream& os = std::cout)
{
    os << "[CONTROL GRAPH REPORT]\n";
    printControlGraphSnapshot(report.snapshot, os);
    os << "\n";
    printControlGraphValidation(report.validation, os);
}

} // namespace soemdsp::runtime
