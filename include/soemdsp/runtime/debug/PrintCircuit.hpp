#pragma once

#include <soemdsp/runtime/Circuit.hpp>
#include <soemdsp/runtime/debug/PrintGraph.hpp>

namespace soemdsp::runtime::debug
{

inline void printCircuit(const Circuit& circuit)
{
    printNodes(circuit);
    printPorts(circuit);
    printConnections(circuit);
}

} // namespace soemdsp::runtime::debug
