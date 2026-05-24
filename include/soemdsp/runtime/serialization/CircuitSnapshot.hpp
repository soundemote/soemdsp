#pragma once

#include <string>
#include <vector>
#include <soemdsp/runtime/Ids.hpp>

namespace soemdsp::runtime
{

struct ParameterSnapshotEntry
{
    NodeId nodeId{};
    std::string parameterId;
    std::string name;
    float value{};
    float defaultValue{};
    float minValue{};
    float midValue{};
    float maxValue{};
    bool visible{};
    bool automatable{};
};

struct NodeSnapshotEntry
{
    NodeId id{};
    std::string typeName;
    std::string name;
    std::string displayName;
    std::string category;
    std::string description;
    float editorX{};
    float editorY{};
};

struct PortSnapshotEntry
{
    NodeId nodeId{};
    PortId portId{};
    std::string direction;
    std::string type;
    std::string name;
    std::string displayName;
    std::string description;
    bool visible{};
};

struct ConnectionSnapshotEntry
{
    ConnectionId id{};
    NodeId sourceNode{};
    PortId sourcePort{};
    NodeId destinationNode{};
    PortId destinationPort{};
};

struct CircuitSnapshot
{
    std::vector<NodeSnapshotEntry> nodes;
    std::vector<PortSnapshotEntry> ports;
    std::vector<ParameterSnapshotEntry> parameters;
    std::vector<ConnectionSnapshotEntry> connections;
};

} // namespace soemdsp::runtime
