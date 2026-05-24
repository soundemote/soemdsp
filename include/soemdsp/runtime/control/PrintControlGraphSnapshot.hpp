#pragma once

#include <iostream>
#include <ostream>
#include <soemdsp/runtime/control/ControlGraphSnapshot.hpp>

namespace soemdsp::runtime
{

inline void printControlGraphSnapshot(
  const ControlGraphSnapshot& snapshot,
  std::ostream& os = std::cout)
{
    os << "[CONTROL GRAPH SNAPSHOT NODES]\n";
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
               << node.name
               << " ("
               << node.kind
               << ")";
            if (node.hasCurveSettings)
            {
                os << " shape "
                   << node.curveShape;
            }
            if (node.hasScaleSettings)
            {
                os << " range ["
                   << node.scaleMinValue
                   << ", "
                   << node.scaleMaxValue
                   << "]";
            }
            if (node.hasParameterTarget)
            {
                os << " -> node "
                   << node.targetNodeId
                   << " param "
                   << node.targetParameterId;
            }
            os << "\n";
        }
    }

    os << "\n[CONTROL GRAPH SNAPSHOT CONNECTIONS]\n";
    if (snapshot.connections.empty())
    {
        os << "<none>\n";
    }
    else
    {
        for (const auto& connection : snapshot.connections)
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
