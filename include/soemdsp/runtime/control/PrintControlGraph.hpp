#pragma once

#include <iostream>
#include <ostream>
#include <soemdsp/runtime/control/ControlGraph.hpp>

namespace soemdsp::runtime
{

inline void printControlGraph(
  const ControlGraph& graph,
  std::ostream& os = std::cout)
{
    os << "[CONTROL GRAPH NODES]\n";
    if (graph.nodes.empty())
    {
        os << "<none>\n";
    }
    else
    {
        for (const auto& node : graph.nodes)
        {
            os << node.id
               << " : "
               << node.name
               << " ("
               << toString(node.kind)
               << ")\n";
        }
    }

    os << "\n[CONTROL GRAPH CONNECTIONS]\n";
    if (graph.connections.empty())
    {
        os << "<none>\n";
    }
    else
    {
        for (const auto& connection : graph.connections)
        {
            os << connection.sourceNodeId
               << ":"
               << connection.sourcePortId
               << " -> "
               << connection.destinationNodeId
               << ":"
               << connection.destinationPortId
               << "\n";
        }
    }
}

} // namespace soemdsp::runtime
