#pragma once

#include <iostream>

#include <soemdsp/runtime/Circuit.hpp>

namespace soemdsp::runtime::debug
{

inline bool validateCircuit(const Circuit& circuit)
{
    bool valid = true;

    for (const auto& connection : circuit.connections)
    {
        if (!connection.valid())
        {
            std::cout
                << "[INVALID CONNECTION] "
                << connection.id
                << "\n";

            valid = false;
            continue;
        }

        if (connection.sourcePort->direction !=
            PortDirection::Output)
        {
            std::cout
                << "[INVALID SOURCE PORT] "
                << connection.id
                << "\n";

            valid = false;
        }

        if (connection.destinationPort->direction !=
            PortDirection::Input)
        {
            std::cout
                << "[INVALID DESTINATION PORT] "
                << connection.id
                << "\n";

            valid = false;
        }
    }

    return valid;
}

} // namespace soemdsp::runtime::debug
