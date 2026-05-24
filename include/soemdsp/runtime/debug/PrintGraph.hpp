#pragma once

#include <iostream>

#include <soemdsp/runtime/Circuit.hpp>

namespace soemdsp::runtime::debug
{

inline void printNodes(const Circuit& circuit)
{
    std::cout << "\n[NODES]\n";

    for (const auto& node : circuit.nodes)
    {
        std::cout
            << node->id
            << " : "
            << (node->category.empty() ? "Uncategorized" : node->category)
            << "/"
            << node->name
            << "\n";
    }
}

inline void printConnections(const Circuit& circuit)
{
    std::cout << "\n[CONNECTIONS]\n";

    for (const auto& connection : circuit.connections)
    {
        std::cout
            << connection.id
            << " : "
            << connection.sourceNode->name
            << "."
            << connection.sourcePort->name
            << "("
            << connection.sourcePort->id
            << ")"
            << " -> "
            << connection.destinationNode->name
            << "."
            << connection.destinationPort->name
            << "("
            << connection.destinationPort->id
            << ")"
            << "\n";
    }
}

} // namespace soemdsp::runtime::debug
