#pragma once
#include <cstddef>
#include <cstdint>
namespace soemdsp::runtime
{
struct Node;
struct Port;
struct Connection
{
    std::uint64_t id{ 0 };
    Node* sourceNode{ nullptr };
    Port* sourcePort{ nullptr };
    Node* destinationNode{ nullptr };
    Port* destinationPort{ nullptr };

    bool valid() const
    {
        return sourceNode &&
               sourcePort &&
               destinationNode &&
               destinationPort;
    }
};
} //namespace soemdsp::runtime
