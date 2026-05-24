#pragma once

#include <ostream>

#include <soemdsp/runtime/Circuit.hpp>

namespace soemdsp::runtime::debug
{

inline const char* portTypeName(PortType type)
{
    switch (type)
    {
        case PortType::Control:
            return "control";
        case PortType::Audio:
            return "audio";
        case PortType::Trigger:
            return "trigger";
    }

    return "unknown";
}

inline const char* portDirectionName(PortDirection direction)
{
    switch (direction)
    {
        case PortDirection::Input:
            return "in";
        case PortDirection::Output:
            return "out";
    }

    return "unknown";
}

inline void exportText(const Circuit& circuit,
                       std::ostream& out)
{
    out << "soemdsp circuit\n\n";

    out << "[nodes]\n";
    out << "id type category name x y\n";
    for (const auto& node : circuit.nodes)
    {
        out << node->id
            << " "
            << node->typeName()
            << " "
            << node->category
            << " "
            << node->name
            << " "
            << node->editorX
            << " "
            << node->editorY
            << "\n";
    }

    out << "\n[ports]\n";
    out << "node_id direction port_id type name visible\n";
    for (const auto& node : circuit.nodes)
    {
        for (const auto& port : node->inputs)
        {
            out << node->id
                << " "
                << portDirectionName(port.direction)
                << " "
                << port.id
                << " "
                << portTypeName(port.type)
                << " "
                << port.name
                << " "
                << port.visible
                << "\n";
        }

        for (const auto& port : node->outputs)
        {
            out << node->id
                << " "
                << portDirectionName(port.direction)
                << " "
                << port.id
                << " "
                << portTypeName(port.type)
                << " "
                << port.name
                << " "
                << port.visible
                << "\n";
        }
    }

    out << "\n[connections]\n";
    out << "id source_node source_port destination_node destination_port\n";
    for (const auto& connection : circuit.connections)
    {
        out << connection.id
            << " "
            << connection.sourceNode->id
            << " "
            << connection.sourcePort->id
            << " "
            << connection.destinationNode->id
            << " "
            << connection.destinationPort->id
            << "\n";
    }
}

} // namespace soemdsp::runtime::debug
