#pragma once

#include <soemdsp/runtime/control/ApplyControlGraph.hpp>
#include <soemdsp/runtime/control/SafeApplyControlGraph.hpp>
#include <soemdsp/runtime/control/ValidateControlGraph.hpp>
#include <soemdsp/runtime/control/ValidateControlGraphTargets.hpp>

namespace soemdsp::runtime
{

struct ControlGraphApplyReport
{
    ControlGraphValidationReport graphValidation;
    ControlGraphTargetValidationReport targetValidation;
    ControlGraphSafeApplyResult applyResult;
};

inline ControlGraphApplyReport makeControlGraphApplyReport(
  const ControlGraph& graph,
  const ControlGraphEvaluationInput& input,
  Circuit& circuit)
{
    ControlGraphApplyReport report;
    report.graphValidation = validateControlGraph(graph);

    if (report.graphValidation.hasErrors())
    {
        report.applyResult = {
          false,
          false,
          false,
          false,
          "structural validation failed" };
        return report;
    }

    report.targetValidation = validateControlGraphTargets(graph, circuit);
    if (report.targetValidation.hasErrors())
    {
        report.applyResult = {
          true,
          false,
          false,
          false,
          "target validation failed" };
        return report;
    }

    const auto apply = applyControlGraphLinearToCircuit(graph, input, circuit);
    report.applyResult = {
      true,
      true,
      apply.evaluated,
      apply.applied,
      apply.message };

    return report;
}

} // namespace soemdsp::runtime
