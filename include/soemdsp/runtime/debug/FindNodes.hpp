#pragma once

#include <cstdint>
#include <iostream>
#include <string_view>
#include <vector>

#include <soemdsp/runtime/Circuit.hpp>

namespace soemdsp::runtime::debug
{

inline std::vector<const Node*> findNodesByCategory(
    const Circuit& circuit,
    std::string_view category)
{
    std::vector<const Node*> nodes;

    for (const auto& node : circuit.nodes)
    {
        if (std::string_view{ node->category } == category)
        {
            nodes.push_back(node.get());
        }
    }

    return nodes;
}

inline std::vector<const Node*> findNodesByName(
    const Circuit& circuit,
    std::string_view name)
{
    std::vector<const Node*> nodes;

    for (const auto& node : circuit.nodes)
    {
        if (std::string_view{ node->name } == name)
        {
            nodes.push_back(node.get());
        }
    }

    return nodes;
}

inline const Node* findNodeById(
    const Circuit& circuit,
    std::uint64_t id)
{
    for (const auto& node : circuit.nodes)
    {
        if (node->id == id)
        {
            return node.get();
        }
    }

    return nullptr;
}

inline void printNodeSummaries(
    const Circuit& circuit)
{
    std::cout << "\n[NODE SUMMARIES]\n";

    for (const auto& node : circuit.nodes)
    {
        std::cout
            << node->id
            << " : "
            << node->summary()
            << "\n";
    }
}

} // namespace soemdsp::runtime::debug
