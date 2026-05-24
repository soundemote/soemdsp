#pragma once

#include <string>
#include <utility>
#include <vector>
#include <soemdsp/runtime/Circuit.hpp>
#include <soemdsp/runtime/control/ValidateControlGraph.hpp>

namespace soemdsp::runtime
{

struct ControlGraphTargetValidationMessage
{
    ControlGraphValidationSeverity severity{};
    std::string message;
};

struct ControlGraphTargetValidationReport
{
    std::vector<ControlGraphTargetValidationMessage> messages;

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

inline void addControlGraphTargetValidationMessage(
  ControlGraphTargetValidationReport& report,
  ControlGraphValidationSeverity severity,
  std::string message)
{
    report.messages.push_back({ severity, std::move(message) });
}

inline ControlGraphTargetValidationReport validateControlGraphTargets(
  const ControlGraph& graph,
  const Circuit& circuit)
{
    ControlGraphTargetValidationReport report;

    for (const auto& node : graph.nodes)
    {
        if (node.kind != ControlNodeKind::ParameterTarget)
        {
            continue;
        }

        if (!node.parameterTarget.has_value())
        {
            addControlGraphTargetValidationMessage(
              report,
              ControlGraphValidationSeverity::Warning,
              "parameter target control node " +
                std::to_string(node.id) +
                " has no parameter target metadata");
            continue;
        }

        const auto& target = *node.parameterTarget;
        if (target.parameterId.empty())
        {
            addControlGraphTargetValidationMessage(
              report,
              ControlGraphValidationSeverity::Warning,
              "parameter target control node " +
                std::to_string(node.id) +
                " has an empty target parameter id");
            continue;
        }

        if (circuit.findParameter(target.nodeId, target.parameterId) == nullptr)
        {
            addControlGraphTargetValidationMessage(
              report,
              ControlGraphValidationSeverity::Error,
              "parameter target control node " +
                std::to_string(node.id) +
                " points to missing circuit parameter node " +
                std::to_string(target.nodeId) +
                " param " +
                target.parameterId);
        }
    }

    return report;
}

} // namespace soemdsp::runtime
