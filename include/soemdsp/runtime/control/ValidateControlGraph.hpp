#pragma once

#include <string>
#include <utility>
#include <vector>
#include <soemdsp/runtime/control/ControlGraph.hpp>

namespace soemdsp::runtime
{

enum class ControlGraphValidationSeverity
{
    Info,
    Warning,
    Error
};

struct ControlGraphValidationMessage
{
    ControlGraphValidationSeverity severity{};
    std::string message;
};

struct ControlGraphValidationReport
{
    std::vector<ControlGraphValidationMessage> messages;

    bool hasErrors() const
    {
        for (const auto& message : messages)
        {
            if (message.severity == ControlGraphValidationSeverity::Error)
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
            if (message.severity == ControlGraphValidationSeverity::Warning)
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

    std::size_t infoCount() const
    {
        std::size_t count = 0;

        for (const auto& message : messages)
        {
            if (message.severity == ControlGraphValidationSeverity::Info)
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
            if (message.severity == ControlGraphValidationSeverity::Warning)
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
            if (message.severity == ControlGraphValidationSeverity::Error)
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
};

inline void addControlGraphValidationMessage(
  ControlGraphValidationReport& report,
  ControlGraphValidationSeverity severity,
  std::string message)
{
    report.messages.push_back({ severity, std::move(message) });
}

inline bool controlGraphContainsNodeId(
  const ControlGraph& graph,
  std::uint64_t nodeId)
{
    for (const auto& node : graph.nodes)
    {
        if (node.id == nodeId)
        {
            return true;
        }
    }

    return false;
}

inline ControlGraphValidationReport validateControlGraph(
  const ControlGraph& graph)
{
    ControlGraphValidationReport report;

    if (!graph.connections.empty() && graph.nodes.empty())
    {
        addControlGraphValidationMessage(
          report,
          ControlGraphValidationSeverity::Error,
          "control graph has connections but no nodes");
    }

    for (std::size_t i = 0; i < graph.nodes.size(); ++i)
    {
        const auto& node = graph.nodes[i];
        for (std::size_t j = i + 1; j < graph.nodes.size(); ++j)
        {
            if (node.id == graph.nodes[j].id)
            {
                addControlGraphValidationMessage(
                  report,
                  ControlGraphValidationSeverity::Error,
                  "duplicate control node id " + std::to_string(node.id));
            }
        }

        if (node.name.empty())
        {
            addControlGraphValidationMessage(
              report,
              ControlGraphValidationSeverity::Warning,
              "control node " + std::to_string(node.id) +
                " has an empty name");
        }

        if (node.kind == ControlNodeKind::ParameterTarget)
        {
            if (!node.parameterTarget.has_value())
            {
                addControlGraphValidationMessage(
                  report,
                  ControlGraphValidationSeverity::Warning,
                  "parameter target control node " +
                    std::to_string(node.id) +
                    " has no parameter target metadata");
            }
            else if (node.parameterTarget->parameterId.empty())
            {
                addControlGraphValidationMessage(
                  report,
                  ControlGraphValidationSeverity::Warning,
                  "parameter target control node " +
                    std::to_string(node.id) +
                    " has an empty target parameter id");
            }
        }

        if (node.kind == ControlNodeKind::Curve &&
            !node.curveSettings.has_value())
        {
            addControlGraphValidationMessage(
              report,
              ControlGraphValidationSeverity::Warning,
              "curve control node " +
                std::to_string(node.id) +
                " has no curve settings");
        }

        if (node.kind == ControlNodeKind::Scale)
        {
            if (!node.scaleSettings.has_value())
            {
                addControlGraphValidationMessage(
                  report,
                  ControlGraphValidationSeverity::Warning,
                  "scale control node " +
                    std::to_string(node.id) +
                    " has no scale settings");
            }
            else
            {
                const auto& settings = *node.scaleSettings;
                if (settings.minValue == settings.maxValue)
                {
                    addControlGraphValidationMessage(
                      report,
                      ControlGraphValidationSeverity::Warning,
                      "scale control node " +
                        std::to_string(node.id) +
                        " has equal min and max values");
                }

                if (settings.minValue > settings.maxValue)
                {
                    addControlGraphValidationMessage(
                      report,
                      ControlGraphValidationSeverity::Warning,
                      "scale control node " +
                        std::to_string(node.id) +
                        " has minValue greater than maxValue");
                }
            }
        }
    }

    for (const auto& connection : graph.connections)
    {
        if (!controlGraphContainsNodeId(graph, connection.sourceNodeId))
        {
            addControlGraphValidationMessage(
              report,
              ControlGraphValidationSeverity::Error,
              "control connection source node " +
                std::to_string(connection.sourceNodeId) +
                " does not exist");
        }

        if (!controlGraphContainsNodeId(graph, connection.destinationNodeId))
        {
            addControlGraphValidationMessage(
              report,
              ControlGraphValidationSeverity::Error,
              "control connection destination node " +
                std::to_string(connection.destinationNodeId) +
                " does not exist");
        }

        if (connection.sourcePortId.empty())
        {
            addControlGraphValidationMessage(
              report,
              ControlGraphValidationSeverity::Warning,
              "control connection from node " +
                std::to_string(connection.sourceNodeId) +
                " has an empty source port id");
        }

        if (connection.destinationPortId.empty())
        {
            addControlGraphValidationMessage(
              report,
              ControlGraphValidationSeverity::Warning,
              "control connection to node " +
                std::to_string(connection.destinationNodeId) +
                " has an empty destination port id");
        }

        // TODO: add node-kind port semantic validation when execution/binding exists.
    }

    return report;
}

} // namespace soemdsp::runtime
