#pragma once

#include <string>
#include <utility>
#include <vector>
#include <soemdsp/runtime/Circuit.hpp>

namespace soemdsp::runtime
{

enum class ValidationSeverity
{
    Info,
    Warning,
    Error
};

struct ValidationMessage
{
    ValidationSeverity severity{};
    std::string message;
};

struct CircuitValidationReport
{
    std::vector<ValidationMessage> messages;

    std::size_t infoCount() const
    {
        std::size_t count = 0;

        for (const auto& message : messages)
        {
            if (message.severity == ValidationSeverity::Info)
            {
                ++count;
            }
        }

        return count;
    }

    std::size_t warningCount() const
    {
        std::size_t count = 0;

        for (const auto& message : messages)
        {
            if (message.severity == ValidationSeverity::Warning)
            {
                ++count;
            }
        }

        return count;
    }

    std::size_t errorCount() const
    {
        std::size_t count = 0;

        for (const auto& message : messages)
        {
            if (message.severity == ValidationSeverity::Error)
            {
                ++count;
            }
        }

        return count;
    }

    std::size_t messageCount() const
    {
        return messages.size();
    }

    bool hasErrors() const
    {
        for (const auto& message : messages)
        {
            if (message.severity == ValidationSeverity::Error)
            {
                return true;
            }
        }

        return false;
    }

    bool hasWarnings() const
    {
        for (const auto& message : messages)
        {
            if (message.severity == ValidationSeverity::Warning)
            {
                return true;
            }
        }

        return false;
    }

    bool ok() const
    {
        return messages.empty();
    }
};

inline void addValidationMessage(
  CircuitValidationReport& report,
  ValidationSeverity severity,
  std::string message)
{
    report.messages.push_back({ severity, std::move(message) });
}

inline bool circuitContainsNode(
  const Circuit& circuit,
  const Node* node)
{
    for (const auto& candidate : circuit.nodes)
    {
        if (candidate.get() == node)
        {
            return true;
        }
    }

    return false;
}

inline bool nodeContainsPort(
  const Node& node,
  const Port* port)
{
    for (const auto& candidate : node.inputs)
    {
        if (&candidate == port)
        {
            return true;
        }
    }

    for (const auto& candidate : node.outputs)
    {
        if (&candidate == port)
        {
            return true;
        }
    }

    return false;
}

inline CircuitValidationReport validateCircuit(const Circuit& circuit)
{
    CircuitValidationReport report;

    for (std::size_t i = 0; i < circuit.nodes.size(); ++i)
    {
        const auto& node = circuit.nodes[i];
        for (std::size_t j = i + 1; j < circuit.nodes.size(); ++j)
        {
            if (node->id == circuit.nodes[j]->id)
            {
                addValidationMessage(
                  report,
                  ValidationSeverity::Error,
                  "duplicate node id " + std::to_string(node->id));
            }
        }

        for (std::size_t parameterIndex = 0;
             parameterIndex < node->parameters.size();
             ++parameterIndex)
        {
            const auto& parameter = node->parameters[parameterIndex];
            for (std::size_t otherIndex = parameterIndex + 1;
                 otherIndex < node->parameters.size();
                 ++otherIndex)
            {
                if (parameter.id == node->parameters[otherIndex].id)
                {
                    addValidationMessage(
                      report,
                      ValidationSeverity::Error,
                      "node " + std::to_string(node->id) +
                        " has duplicate parameter id '" +
                        parameter.id +
                        "'");
                }
            }

            if (parameter.minValue > parameter.maxValue)
            {
                addValidationMessage(
                  report,
                  ValidationSeverity::Error,
                  "parameter '" + parameter.id +
                    "' on node " + std::to_string(node->id) +
                    " has minValue greater than maxValue");
                continue;
            }

            if (parameter.midValue < parameter.minValue ||
                parameter.midValue > parameter.maxValue)
            {
                addValidationMessage(
                  report,
                  ValidationSeverity::Warning,
                  "parameter '" + parameter.id +
                    "' on node " + std::to_string(node->id) +
                    " has midValue outside minValue/maxValue");
            }

            if (parameter.value < parameter.minValue ||
                parameter.value > parameter.maxValue)
            {
                addValidationMessage(
                  report,
                  ValidationSeverity::Warning,
                  "parameter '" + parameter.id +
                    "' on node " + std::to_string(node->id) +
                    " has value outside minValue/maxValue");
            }

            if (parameter.defaultValue < parameter.minValue ||
                parameter.defaultValue > parameter.maxValue)
            {
                addValidationMessage(
                  report,
                  ValidationSeverity::Warning,
                  "parameter '" + parameter.id +
                    "' on node " + std::to_string(node->id) +
                    " has defaultValue outside minValue/maxValue");
            }
        }
    }

    for (const auto& connection : circuit.connections)
    {
        if (!connection.valid())
        {
            addValidationMessage(
              report,
              ValidationSeverity::Error,
              "connection " + std::to_string(connection.id) +
                " has a missing endpoint");
            continue;
        }

        if (!circuitContainsNode(circuit, connection.sourceNode))
        {
            addValidationMessage(
              report,
              ValidationSeverity::Error,
              "connection " + std::to_string(connection.id) +
                " source node is not in circuit");
        }

        if (!circuitContainsNode(circuit, connection.destinationNode))
        {
            addValidationMessage(
              report,
              ValidationSeverity::Error,
              "connection " + std::to_string(connection.id) +
                " destination node is not in circuit");
        }

        if (!nodeContainsPort(*connection.sourceNode, connection.sourcePort))
        {
            addValidationMessage(
              report,
              ValidationSeverity::Error,
              "connection " + std::to_string(connection.id) +
                " source port is not on source node");
        }

        if (!nodeContainsPort(*connection.destinationNode, connection.destinationPort))
        {
            addValidationMessage(
              report,
              ValidationSeverity::Error,
              "connection " + std::to_string(connection.id) +
                " destination port is not on destination node");
        }

        // TODO: add deeper type/direction/rate validation as graph loading matures.
    }

    return report;
}

} // namespace soemdsp::runtime
