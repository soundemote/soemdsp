#pragma once

#include <iostream>
#include <ostream>
#include <soemdsp/runtime/dsp/ApplyDspBinding.hpp>

namespace soemdsp::runtime
{

inline void printDspBindingApplyResult(
  const DspBindingApplyResult& result,
  std::ostream& os = std::cout)
{
    os << "[DSP BINDING APPLY RESULT]\n"
       << "ok: "
       << (result.ok ? "true" : "false")
       << "\n"
       << "parameters applied: "
       << result.parametersApplied
       << "\n"
       << "messages:\n";

    if (result.messages.empty())
    {
        os << "- <none>\n";
        return;
    }

    for (const auto& message : result.messages)
    {
        os << "- "
           << message
           << "\n";
    }
}

} // namespace soemdsp::runtime
