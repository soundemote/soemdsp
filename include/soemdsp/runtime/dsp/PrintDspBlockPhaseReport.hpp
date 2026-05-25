#pragma once

#include <iostream>
#include <ostream>
#include <soemdsp/runtime/dsp/DspBlockPhaseReport.hpp>

namespace soemdsp::runtime
{

inline void printDspBlockPhaseReport(
  const DspBlockPhaseReport& report,
  std::ostream& os = std::cout)
{
    os << "[DSP BLOCK PHASE REPORT]\n"
       << "preflight ok: "
       << (report.preflightOk ? "true" : "false")
       << "\n"
       << "apply ok: "
       << (report.applyOk ? "true" : "false")
       << "\n"
       << "process ok: "
       << (report.processOk ? "true" : "false")
       << "\n"
       << "bindings checked: "
       << report.bindingsChecked
       << "\n"
       << "preflight messages: "
       << report.preflightMessages
       << "\n"
       << "parameters applied: "
       << report.parametersApplied
       << "\n"
       << "apply messages: "
       << report.applyMessages
       << "\n"
       << "samples processed: "
       << report.samplesProcessed
       << "\n";
}

} // namespace soemdsp::runtime

