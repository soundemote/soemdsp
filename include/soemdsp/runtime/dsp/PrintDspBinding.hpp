#pragma once

#include <iostream>
#include <ostream>
#include <soemdsp/runtime/dsp/DspBinding.hpp>

namespace soemdsp::runtime
{

inline void printDspObjectBinding(
  const DspObjectBinding& binding,
  std::ostream& os = std::cout)
{
    os << "[DSP OBJECT BINDING]\n"
       << "node "
       << binding.nodeId
       << " : "
       << binding.objectName
       << " ("
       << binding.objectType
       << ")\n";

    os << "\n[PARAMETER BINDINGS]\n";
    if (binding.parameterBindings.empty())
    {
        os << "<none>\n";
        return;
    }

    for (const auto& parameterBinding : binding.parameterBindings)
    {
        os << "node "
           << parameterBinding.nodeId
           << " param "
           << parameterBinding.parameterId
           << " -> "
           << toString(parameterBinding.targetKind)
           << " "
           << parameterBinding.targetName
           << " offset "
           << parameterBinding.memoryOffset
           << "\n";
    }
}

} // namespace soemdsp::runtime
