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
          node.editorY });
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
