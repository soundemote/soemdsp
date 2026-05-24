#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <soemdsp/runtime/control/ControlGraph.hpp>

namespace soemdsp::runtime
{

struct ControlNodeSnapshotEntry
{
    std::uint64_t id{};
    std::string kind;
    std::string name;
    std::string description;
    float editorX{};
    float editorY{};
    bool hasParameterTarget{};
    NodeId targetNodeId{};
    std::string targetParameterId;
    bool hasCurveSettings{};
    std::string curveShape;
    float curveMidpoint{};
    bool hasScaleSettings{};
    float scaleMinValue{};
    float scaleMaxValue{};
};

struct ControlConnectionSnapshotEntry
{
    std::uint64_t sourceNodeId{};
    std::string sourcePortId;
    std::uint64_t destinationNodeId{};
    std::string destinationPortId;
};

struct ControlGraphSnapshot
{
    std::vector<ControlNodeSnapshotEntry> nodes;
    std::vector<ControlConnectionSnapshotEntry> connections;
};

inline ControlGraphSnapshot snapshotControlGraph(
  const ControlGraph& graph)
{
    ControlGraphSnapshot snapshot;

    for (const auto& node : graph.nodes)
    {
        snapshot.nodes.push_back({
          node.id,
          toString(node.kind),
          node.name,
          node.description,
          node.editorX,
          node.editorY,
          node.parameterTarget.has_value(),
          node.parameterTarget ? node.parameterTarget->nodeId : NodeId{},
          node.parameterTarget ? node.parameterTarget->parameterId : std::string{},
          node.curveSettings.has_value(),
          node.curveSettings ? toString(node.curveSettings->shape) : std::string{},
          node.curveSettings ? node.curveSettings->midpoint : 0.5f,
          node.scaleSettings.has_value(),
          node.scaleSettings ? node.scaleSettings->minValue : 0.0f,
          node.scaleSettings ? node.scaleSettings->maxValue : 0.0f });
    }

    for (const auto& connection : graph.connections)
    {
        snapshot.connections.push_back({
          connection.sourceNodeId,
          connection.sourcePortId,
          connection.destinationNodeId,
          connection.destinationPortId });
    }

    return snapshot;
}

} // namespace soemdsp::runtime
