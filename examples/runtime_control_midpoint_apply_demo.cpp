#include <iomanip>
#include <iostream>
#include <memory>
#include <string>

#include <soemdsp/runtime/control/ControlGraphParameterMidpoint.hpp>
#include <soemdsp/runtime/control/ControlGraphApplyReport.hpp>
#include <soemdsp/runtime/control/WriteControlGraphApplyReport.hpp>
#include <soemdsp/soemdsp.hpp>

using namespace soemdsp::runtime;
using namespace soemdsp::runtime::nodes;

namespace
{

ControlNode makeNode(
  std::uint64_t id,
  ControlNodeKind kind,
  std::string name)
{
    ControlNode node;
    node.id   = id;
    node.kind = kind;
    node.name = std::move(name);
    return node;
}

ControlConnection makeConnection(
  std::uint64_t sourceNodeId,
  std::string sourcePortId,
  std::uint64_t destinationNodeId,
  std::string destinationPortId)
{
    ControlConnection connection;
    connection.sourceNodeId      = sourceNodeId;
    connection.sourcePortId      = std::move(sourcePortId);
    connection.destinationNodeId = destinationNodeId;
    connection.destinationPortId = std::move(destinationPortId);
    return connection;
}

Parameter makeCutoffParameter()
{
    Parameter cutoff;
    cutoff.id           = "cutoff";
    cutoff.name         = "Cutoff";
    cutoff.value        = 1000.0f;
    cutoff.defaultValue = 1000.0f;
    cutoff.minValue     = 20.0f;
    cutoff.midValue     = 1000.0f;
    cutoff.maxValue     = 20000.0f;
    return cutoff;
}

Circuit createCircuit(const Parameter& cutoff)
{
    Circuit circuit;

    auto node = std::make_unique<FloatConstant>(0.0f);
    node->id = 100;
    node->name = "Filter";
    node->category = "Processor";
    node->description = "Demo parameter target";
    node->parameters.push_back(cutoff);

    circuit.nodes.push_back(std::move(node));
    circuit.prepare();

    return circuit;
}

ControlGraph createControlGraph(const Parameter& cutoff)
{
    ControlGraph graph;

    graph.nodes.push_back(
      makeNode(1, ControlNodeKind::MacroKnob, "Brightness"));

    auto curve =
      makeNode(2, ControlNodeKind::Curve, "Cutoff Midpoint Curve");
    applyParameterMidpointToCurveNode(curve, cutoff);
    graph.nodes.push_back(curve);

    graph.nodes.push_back(
      makeNode(3, ControlNodeKind::Clamp01, "Clamp"));

    auto scale =
      makeNode(4, ControlNodeKind::Scale, "Normalized Scale");
    scale.scaleSettings = ControlScaleSettings{ 0.0f, 1.0f };
    graph.nodes.push_back(scale);

    auto target =
      makeNode(5, ControlNodeKind::ParameterTarget, "Cutoff Target");
    target.parameterTarget = ControlParameterTarget{ 100, "cutoff" };
    graph.nodes.push_back(target);

    graph.connections.push_back(makeConnection(1, "value", 2, "input"));
    graph.connections.push_back(makeConnection(2, "output", 3, "input"));
    graph.connections.push_back(makeConnection(3, "output", 4, "input"));
    graph.connections.push_back(makeConnection(4, "output", 5, "value"));

    return graph;
}

ControlGraphApplyReport applyAndPrint(
  const ControlGraph& graph,
  Circuit& circuit,
  float input)
{
    auto report =
      makeControlGraphApplyReport(graph, { 1, input }, circuit);

    const auto* cutoff = circuit.findParameter(100, "cutoff");
    if (cutoff == nullptr)
    {
        return report;
    }

    std::cout << "input "
              << input
              << " -> cutoff "
              << cutoff->value
              << " | graph errors "
              << report.graphValidation.errorCount()
              << " | target errors "
              << report.targetValidation.errorCount()
              << " | evaluated "
              << (report.applyResult.evaluated ? "true" : "false")
              << " | applied "
              << (report.applyResult.applied ? "true" : "false")
              << " | message "
              << report.applyResult.message
              << "\n";

    return report;
}

} // namespace

int main()
{
    std::cout << std::fixed << std::setprecision(4);

    const auto cutoff = makeCutoffParameter();
    auto circuit = createCircuit(cutoff);
    const auto graph = createControlGraph(cutoff);
    const auto normalizedMidpoint =
      normalizedMidpointForParameter(cutoff);

    std::cout << "[CONTROL MIDPOINT APPLY]\n"
              << "normalized parameter midpoint: ";
    if (normalizedMidpoint.has_value())
    {
        std::cout << *normalizedMidpoint << "\n";
    }
    else
    {
        std::cout << "<none>\n";
    }

    std::cout << std::setprecision(2);
    applyAndPrint(graph, circuit, 0.0f);
    applyAndPrint(graph, circuit, 0.5f);
    const auto finalReport =
      applyAndPrint(graph, circuit, 1.0f);

    const auto wroteReport =
      writeControlGraphApplyReportTextFile(
        finalReport,
        "runtime_control_midpoint_apply_demo.control_apply_report.txt");
    std::cout << "control apply report file: "
              << (wroteReport ? "wrote" : "failed")
              << "\n";

    return 0;
}
