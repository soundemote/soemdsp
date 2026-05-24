#pragma once

#include <algorithm>
#include <iostream>
#include <vector>

#include <soemdsp/runtime/Circuit.hpp>

namespace soemdsp::runtime::debug
{

inline std::vector<const Node*> sortedNodesByCategory(const Circuit& circuit)
{
    std::vector<const Node*> nodes;
    nodes.reserve(circuit.nodes.size());

    for (const auto& node : circuit.nodes)
    {
        nodes.push_back(node.get());
    }

    std::sort(
        nodes.begin(),
        nodes.end(),
        [](const Node* a, const Node* b)
        {
            if (a->category != b->category)
            {
                return a->category < b->category;
            }

            if (a->name != b->name)
            {
                return a->name < b->name;
            }

            return a->id < b->id;
        }
    );

    return nodes;
}

inline void printSortedNodes(const Circuit& circuit)
{
    std::cout << "\n[SORTED NODES]\n";

    const auto nodes = sortedNodesByCategory(circuit);
    std::string currentCategory;

    for (const auto* node : nodes)
    {
        const auto category =
            node->category.empty()
                ? std::string{ "Uncategorized" }
                : node->category;

        if (category != currentCategory)
        {
            currentCategory = category;

            std::cout
                << "\n["
                << currentCategory
                << "]\n";
        }

        std::cout
            << node->id
            << " : "
            << node->name
            << "\n";
    }
}

} // namespace soemdsp::runtime::debug
