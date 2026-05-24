#pragma once

#include <iostream>
#include <ostream>
#include <soemdsp/runtime/serialization/CircuitSnapshot.hpp>

namespace soemdsp::runtime
{

inline void printCircuitSnapshot(
  const CircuitSnapshot& snapshot,
  std::ostream& os = std::cout)
{
    os << "[SNAPSHOT NODES]\n";
    if (snapshot.nodes.empty())
    {
        os << "<none>\n";
    }
    else
    {
        for (const auto& node : snapshot.nodes)
        {
            os << node.id
               << " : "
               << node.displayName
               << " ("
               << node.typeName
               << ")\n";
        }
    }

    os << "\n[SNAPSHOT PORTS]\n";
    if (snapshot.ports.empty())
    {
        os << "<none>\n";
    }
    else
    {
        for (const auto& port : snapshot.ports)
        {
            os << port.nodeId
               << " : "
               << port.portId
               << " : "
               << port.displayName
               << "\n";
        }
    }

    os << "\n[SNAPSHOT PARAMETERS]\n";
    if (snapshot.parameters.empty())
    {
        os << "<none>\n";
    }
    else
    {
        for (const auto& parameter : snapshot.parameters)
        {
            os << parameter.nodeId
               << " : "
               << parameter.parameterId
               << " : "
               << parameter.name
               << " = "
               << parameter.value
               << " ["
               << parameter.minValue
               << ", "
               << parameter.midValue
               << ", "
               << parameter.maxValue
               << "]\n";
        }
    }

    os << "\n[SNAPSHOT CONNECTIONS]\n";
    if (snapshot.connections.empty())
    {
        os << "<none>\n";
    }
    else
    {
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
}

} // namespace soemdsp::runtime
