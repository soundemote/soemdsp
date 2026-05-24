#pragma once

#include <algorithm>
#include <cstdint>
#include <string>
#include <unordered_set>
#include <vector>
#include <soemdsp/runtime/control/ControlGraph.hpp>

namespace soemdsp::runtime
{

struct ControlGraphEvaluationInput
{
    std::uint64_t macroNodeId{};
    float value{};
};

struct ControlGraphEvaluationOutput
{
    std::uint64_t nodeId{};
    std::string portId;
    float value{};
};

struct ControlGraphEvaluationResult
{
    std::vector<ControlGraphEvaluationOutput> outputs;
    bool success{};
    std::string message;
};

inline const ControlNode* findControlNode(
  const ControlGraph& graph,
  std::uint64_t nodeId)
{
    for (const auto& node : graph.nodes)
    {
        if (node.id == nodeId)
        {
            return &node;
        }
    }

    return nullptr;
}

inline const ControlConnection* findFirstControlConnectionFrom(
  const ControlGraph& graph,
  std::uint64_t sourceNodeId)
{
    for (const auto& connection : graph.connections)
    {
        if (connection.sourceNodeId == sourceNodeId)
        {
            return &connection;
        }
    }

    return nullptr;
}

inline float evaluateControlCurveShape(
  ControlCurveShape shape,
  float value)
{
    const auto x = std::clamp(value, 0.0f, 1.0f);

    switch (shape)
    {
        case ControlCurveShape::Linear:
            return x;
        case ControlCurveShape::EaseIn:
            return x * x;
        case ControlCurveShape::EaseOut:
            return 1.0f - ((1.0f - x) * (1.0f - x));
        case ControlCurveShape::Smoothstep:
            return x * x * (3.0f - (2.0f * x));
    }

    return x;
}

inline ControlGraphEvaluationResult evaluateControlGraphLinear(
  const ControlGraph& graph,
  ControlGraphEvaluationInput input)
{
    ControlGraphEvaluationResult result;
    auto currentNodeId = input.macroNodeId;
    auto value         = std::clamp(input.value, 0.0f, 1.0f);
    std::unordered_set<std::uint64_t> visited;

    while (true)
    {
        if (!visited.insert(currentNodeId).second)
        {
            result.success = false;
            result.message = "cycle detected in control graph";
            return result;
        }

        const auto* node = findControlNode(graph, currentNodeId);
        if (node == nullptr)
        {
            result.success = false;
            result.message = "control node missing";
            return result;
        }

        switch (node->kind)
        {
            case ControlNodeKind::MacroKnob:
            case ControlNodeKind::Smooth:
            case ControlNodeKind::Split:
                break;
            case ControlNodeKind::Curve:
                if (node->curveSettings.has_value())
                {
                    value = evaluateControlCurveShape(
                      node->curveSettings->shape,
                      value);
                }
                break;
            case ControlNodeKind::Clamp01:
                value = std::clamp(value, 0.0f, 1.0f);
                break;
            case ControlNodeKind::Scale:
                if (node->scaleSettings.has_value())
                {
                    const auto& settings = *node->scaleSettings;
                    value = settings.minValue +
                            (value * (settings.maxValue - settings.minValue));
                }
                break;
            case ControlNodeKind::Invert:
                value = 1.0f - value;
                break;
            case ControlNodeKind::ParameterTarget:
                result.outputs.push_back({ node->id, "value", value });
                result.success = true;
                result.message = "ok";
                return result;
        }

        const auto* connection =
          findFirstControlConnectionFrom(graph, currentNodeId);
        if (connection == nullptr)
        {
            result.success = false;
            result.message = "control chain ended before parameter target";
            return result;
        }

        currentNodeId = connection->destinationNodeId;
    }
}

} // namespace soemdsp::runtime
