#pragma once

#include <soemdsp/runtime/control/ControlGraph.hpp>
#include <soemdsp/runtime/control/ControlGraphSnapshot.hpp>
#include <soemdsp/runtime/control/ValidateControlGraph.hpp>

namespace soemdsp::runtime
{

struct ControlGraphReport
{
    ControlGraphSnapshot snapshot;
    ControlGraphValidationReport validation;
};

inline ControlGraphReport makeControlGraphReport(
  const ControlGraph& graph)
{
    ControlGraphReport report;
    report.snapshot   = snapshotControlGraph(graph);
    report.validation = validateControlGraph(graph);
    return report;
}

} // namespace soemdsp::runtime
