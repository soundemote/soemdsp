#pragma once

#include <iostream>
#include <ostream>
#include <soemdsp/runtime/dsp/DspBindingApplySummary.hpp>

namespace soemdsp::runtime
{

inline void printDspBindingApplySummary(
  const DspBindingApplySummary& summary,
  std::ostream& os = std::cout)
{
    os << "[DSP BINDING APPLY SUMMARY]\n"
       << "ok: "
       << (summary.ok ? "true" : "false")
       << "\n"
       << "parameters applied: "
       << summary.parametersApplied
       << "\n"
       << "messages: "
       << summary.messageCount
       << "\n";
}

} // namespace soemdsp::runtime
