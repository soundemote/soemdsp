#pragma once

#include <soemdsp/runtime/Circuit.hpp>
#include <soemdsp/runtime/debug/PrintGraph.hpp>
#include <soemdsp/runtime/debug/PrintLayout.hpp>

namespace soemdsp::runtime::debug
{

inline void printCircuit(const Circuit& circuit)
{
    printNodes(circuit);
    printPorts(circuit);
    printParameters(circuit);
    printLayout(circuit);
    printConnections(circuit);
}

} // namespace soemdsp::runtime::debug
