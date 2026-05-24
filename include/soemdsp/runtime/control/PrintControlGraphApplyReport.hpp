#pragma once

#include <iostream>
#include <ostream>
#include <soemdsp/runtime/control/ControlGraphApplyReport.hpp>
#include <soemdsp/runtime/control/PrintControlGraphTargetValidation.hpp>
#include <soemdsp/runtime/control/PrintControlGraphValidation.hpp>

namespace soemdsp::runtime
{

inline void printControlGraphApplyReport(
  const ControlGraphApplyReport& report,
  std::ostream& os = std::cout)
{
    os << "[CONTROL GRAPH APPLY REPORT]\n";
    printControlGraphValidation(report.graphValidation, os);

    if (report.applyResult.graphValid || !report.targetValidation.messages.empty())
    {
        printControlGraphTargetValidation(report.targetValidation, os);
    }
    else
    {
        os << "target validation: skipped\n";
    }

    os << "evaluated: "
       << (report.applyResult.evaluated ? "true" : "false")
       << "\n"
       << "applied: "
       << (report.applyResult.applied ? "true" : "false")
       << "\n"
       << "message: "
       << report.applyResult.message
       << "\n";
}

} // namespace soemdsp::runtime
