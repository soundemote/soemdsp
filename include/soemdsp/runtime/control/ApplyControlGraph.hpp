#pragma once

#include <string>
#include <soemdsp/runtime/Circuit.hpp>
#include <soemdsp/runtime/control/EvaluateControlGraph.hpp>

namespace soemdsp::runtime
{

struct ControlGraphApplyResult
{
    bool evaluated{};
    bool applied{};
    std::string message;
};

inline ControlGraphApplyResult applyControlGraphLinearToCircuit(
  const ControlGraph& graph,
  const ControlGraphEvaluationInput& input,
  Circuit& circuit)
{
    const auto evaluation = evaluateControlGraphLinear(graph, input);
    if (!evaluation.success)
    {
        return { false, false, evaluation.message };
    }

    for (const auto& output : evaluation.outputs)
    {
        const auto* node = findControlNode(graph, output.nodeId);
        if (node == nullptr)
        {
            return { true, false, "evaluation output control node missing" };
        }

        if (node->kind != ControlNodeKind::ParameterTarget)
        {
            return { true, false, "evaluation output is not a parameter target" };
        }

        if (!node->parameterTarget.has_value())
        {
            return { true, false, "parameter target metadata missing" };
        }

        const auto& target = *node->parameterTarget;
        if (!circuit.setParameterNormalizedValue(
              target.nodeId,
              target.parameterId,
              output.value))
        {
            return { true, false, "failed to set target parameter" };
        }
    }

    return { true, true, "applied" };
}

} // namespace soemdsp::runtime
