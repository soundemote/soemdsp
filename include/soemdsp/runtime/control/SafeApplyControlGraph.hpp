#pragma once

#include <string>
#include <soemdsp/runtime/control/ApplyControlGraph.hpp>
#include <soemdsp/runtime/control/ValidateControlGraph.hpp>
#include <soemdsp/runtime/control/ValidateControlGraphTargets.hpp>

namespace soemdsp::runtime
{

struct ControlGraphSafeApplyResult
{
    bool graphValid{};
    bool targetsValid{};
    bool evaluated{};
    bool applied{};
    std::string message;
};

inline ControlGraphSafeApplyResult safeApplyControlGraphLinearToCircuit(
  const ControlGraph& graph,
  const ControlGraphEvaluationInput& input,
  Circuit& circuit)
{
    const auto graphValidation = validateControlGraph(graph);
    if (graphValidation.hasErrors())
    {
        return {
          false,
          false,
          false,
          false,
          "structural validation failed" };
    }

    const auto targetValidation =
      validateControlGraphTargets(graph, circuit);
    if (targetValidation.hasErrors())
    {
        return {
          true,
          false,
          false,
          false,
          "target validation failed" };
    }

    const auto apply =
      applyControlGraphLinearToCircuit(graph, input, circuit);
    return {
      true,
      true,
      apply.evaluated,
      apply.applied,
      apply.message };
}

} // namespace soemdsp::runtime
