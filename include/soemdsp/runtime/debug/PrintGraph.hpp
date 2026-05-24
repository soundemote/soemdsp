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
            << node->displayName()
            << "\n";
    }
}

inline void printPorts(const Circuit& circuit)
{
    std::cout << "\n[PORTS]\n";

    for (const auto& node : circuit.nodes)
    {
        std::cout
            << node->id
            << " : "
            << node->displayName()
            << "\n";

        for (const auto& port : node->inputs)
        {
            std::cout
                << "  in  "
                << port.id
                << " : "
                << port.summary()
                << "\n";
        }

        for (const auto& port : node->outputs)
        {
            std::cout
                << "  out "
                << port.id
                << " : "
                << port.summary()
                << "\n";
        }
    }
}

inline void printParameters(const Circuit& circuit)
{
    std::cout << "\n[PARAMETERS]\n";

    bool foundParameters = false;

    for (const auto& node : circuit.nodes)
    {
        if (node->parameters.empty())
        {
            continue;
        }

        foundParameters = true;

        std::cout
            << node->id
            << " : "
            << node->displayName()
            << "\n";

        for (const auto& parameter : node->parameters)
        {
            std::cout
                << "  "
                << parameter.id
                << " : "
                << parameter.name
                << " = "
                << parameter.value
                << " ["
                << parameter.minValue
                << ", "
                << parameter.midValue
                << ", "
                << parameter.maxValue
                << "]\n";
        }
    }

    if (!foundParameters)
    {
        std::cout << "<none>\n";
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
