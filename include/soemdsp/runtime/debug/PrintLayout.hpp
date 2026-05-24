#pragma once

#include <iostream>

#include <soemdsp/runtime/Circuit.hpp>

namespace soemdsp::runtime::debug
{

inline void printLayout(const Circuit& circuit)
{
    std::cout << "\n[LAYOUT]\n";

    for (const auto& node : circuit.nodes)
    {
        std::cout
            << node->id
            << " : "
            << node->displayName()
            << " @ ("
            << node->editorX
            << ", "
            << node->editorY
            << ")\n";
    }
}

} // namespace soemdsp::runtime::debug
