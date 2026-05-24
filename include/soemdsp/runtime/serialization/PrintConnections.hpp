#pragma once

#include <iostream>
#include <ostream>
#include <soemdsp/runtime/serialization/CircuitSnapshot.hpp>

namespace soemdsp::runtime
{

inline void printConnectionsText(
  const CircuitSnapshot& snapshot,
  std::ostream& os = std::cout)
{
    os << "[CONNECTIONS]\n";
    if (snapshot.connections.empty())
    {
        os << "<none>\n";
        return;
    }

    for (const auto& connection : snapshot.connections)
    {
        os << connection.sourceNode
           << ":"
           << connection.sourcePort
           << " -> "
           << connection.destinationNode
           << ":"
           << connection.destinationPort
           << "\n";
    }
}

} // namespace soemdsp::runtime
