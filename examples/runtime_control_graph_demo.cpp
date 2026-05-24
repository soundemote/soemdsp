#include <iostream>
#include <memory>

#include <soemdsp/runtime/control/ApplyControlGraph.hpp>
#include <soemdsp/runtime/control/EvaluateControlGraph.hpp>
#include <soemdsp/runtime/control/PrintControlGraph.hpp>
#include <soemdsp/runtime/control/PrintControlGraphReport.hpp>
#include <soemdsp/runtime/control/PrintControlGraphTargetValidation.hpp>
#include <soemdsp/runtime/control/SafeApplyControlGraph.hpp>
#include <soemdsp/runtime/control/WriteControlGraphSnapshot.hpp>
#include <soemdsp/runtime/control/WriteControlGraphReport.hpp>
#include <soemdsp/soemdsp.hpp>

using namespace soemdsp::runtime;
using namespace soemdsp::runtime::nodes;

namespace
{

ControlNode makeNode(
  std::uint64_t id,
  ControlNodeKind kind,
  std::string name,
  float editorX,
  float editorY)
{
    ControlNode node;
    node.id      = id;
    node.kind    = kind;
    node.name    = std::move(name);
    node.editorX = editorX;
    node.editorY = editorY;
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

ControlGraph createDemoGraph()
{
    ControlGraph graph;

    graph.nodes.push_back(
      makeNode(1, ControlNodeKind::MacroKnob, "Brightness", 0.0f, 0.0f));
    graph.nodes.push_back(
      makeNode(2, ControlNodeKind::Curve, "S Curve", 180.0f, 0.0f));
    graph.nodes.push_back(
      makeNode(3, ControlNodeKind::Clamp01, "Clamp", 360.0f, 0.0f));
    graph.nodes.push_back(
      makeNode(4, ControlNodeKind::Scale, "Cutoff Scale", 540.0f, 0.0f));
    graph.nodes.push_back(
      makeNode(5, ControlNodeKind::ParameterTarget, "Osc Cutoff", 720.0f, 0.0f));
    graph.nodes.back().parameterTarget = ControlParameterTarget{ 100, "cutoff" };

    graph.connections.push_back(makeConnection(1, "value", 2, "input"));
    graph.connections.push_back(makeConnection(2, "output", 3, "input"));
    graph.connections.push_back(makeConnection(3, "output", 4, "input"));
    graph.connections.push_back(makeConnection(4, "output", 5, "value"));

    return graph;
}

Circuit createDemoCircuit()
{
    Circuit circuit;

    auto node = std::make_unique<FloatConstant>(0.5f);
    node->id = 100;

    Parameter cutoff;
    cutoff.id           = "cutoff";
    cutoff.name         = "Cutoff";
    cutoff.value        = 0.5f;
    cutoff.defaultValue = 0.5f;
    cutoff.minValue     = 20.0f;
    cutoff.midValue     = 1000.0f;
    cutoff.maxValue     = 20000.0f;

    node->parameters.push_back(cutoff);
    circuit.nodes.push_back(std::move(node));
    circuit.prepare();

    return circuit;
}

void printEvaluation(
  const ControlGraphEvaluationResult& result)
{
    std::cout << "success: " << (result.success ? "true" : "false") << "\n";
    if (!result.message.empty())
    {
        std::cout << "message: " << result.message << "\n";
    }

    for (const auto& output : result.outputs)
    {
        std::cout << "output: node " << output.nodeId
                  << " " << output.portId
                  << " " << output.value << "\n";
    }
}

void printApplyResult(
  const ControlGraphApplyResult& result,
  const Circuit& circuit)
{
    const auto* cutoff = circuit.findParameter(100, "cutoff");

    std::cout << "evaluated: "
              << (result.evaluated ? "true" : "false")
              << " | applied: "
              << (result.applied ? "true" : "false")
              << " | message: "
              << result.message
              << "\n";

    if (cutoff != nullptr)
    {
        std::cout << "cutoff value: "
                  << cutoff->value
                  << "\n";
    }
}

void printSafeApplyResult(
  const ControlGraphSafeApplyResult& result,
  const Circuit& circuit)
{
    const auto* cutoff = circuit.findParameter(100, "cutoff");

    std::cout << "graph valid: "
              << (result.graphValid ? "true" : "false")
              << "\n"
              << "targets valid: "
              << (result.targetsValid ? "true" : "false")
              << "\n"
              << "evaluated: "
              << (result.evaluated ? "true" : "false")
              << "\n"
              << "applied: "
              << (result.applied ? "true" : "false")
              << "\n"
              << "message: "
              << result.message
              << "\n";

    if (cutoff != nullptr)
    {
        std::cout << "cutoff: "
                  << cutoff->value
                  << "\n";
    }
}

} // namespace

int main()
{
    const auto graph = createDemoGraph();
    auto circuit = createDemoCircuit();
    printControlGraph(graph);
    const auto targetValidation =
      validateControlGraphTargets(graph, circuit);
    printControlGraphTargetValidation(targetValidation);
    const auto report = makeControlGraphReport(graph);
    printControlGraphReport(report);
    const auto wroteSnapshot =
      writeControlGraphSnapshotTextFile(
        report.snapshot,
        "runtime_control_graph_demo.control_snapshot.txt");
    std::cout << "control snapshot file: "
              << (wroteSnapshot ? "wrote" : "failed")
              << "\n";
    const auto wroteReport =
      writeControlGraphReportTextFile(
        report,
        "runtime_control_graph_demo.control_report.txt");
    std::cout << "control report file: "
              << (wroteReport ? "wrote" : "failed")
              << "\n";
    std::cout << "\n[CONTROL GRAPH EVALUATION]\n";
    printEvaluation(
      evaluateControlGraphLinear(graph, { 1, 0.25f }));
    printEvaluation(
      evaluateControlGraphLinear(graph, { 1, 1.25f }));
    std::cout << "\n[CONTROL GRAPH APPLY]\n";
    printApplyResult(
      applyControlGraphLinearToCircuit(graph, { 1, 0.25f }, circuit),
      circuit);
    printApplyResult(
      applyControlGraphLinearToCircuit(graph, { 1, 1.25f }, circuit),
      circuit);
    circuit.resetParameterValueByName(100, "cutoff");
    std::cout << "\n[CONTROL GRAPH SAFE APPLY]\n";
    printSafeApplyResult(
      safeApplyControlGraphLinearToCircuit(graph, { 1, 0.25f }, circuit),
      circuit);
    return 0;
}
