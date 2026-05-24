#include <iostream>
#include <memory>

#include <soemdsp/runtime/control/PrintControlGraphApplyReport.hpp>
#include <soemdsp/runtime/control/PrintControlGraph.hpp>
#include <soemdsp/runtime/control/PrintControlGraphReport.hpp>
#include <soemdsp/runtime/control/PrintControlGraphTargetValidation.hpp>
#include <soemdsp/runtime/control/WriteControlGraphApplyReport.hpp>
#include <soemdsp/runtime/control/WriteControlGraphSnapshot.hpp>
#include <soemdsp/runtime/control/WriteControlGraphReport.hpp>
#include <soemdsp/soemdsp.hpp>

using namespace soemdsp::runtime;
using namespace soemdsp::runtime::nodes;

namespace
{

ControlGraph createInvalidGraph()
{
    ControlGraph graph;

    graph.nodes.push_back({
      1,
      ControlNodeKind::MacroKnob,
      "Macro",
      "Duplicate id source",
      0.0f,
      0.0f });

    graph.nodes.push_back({
      1,
      ControlNodeKind::Curve,
      "",
      "Duplicate id and empty name",
      160.0f,
      0.0f });

    graph.nodes.push_back({
      2,
      ControlNodeKind::ParameterTarget,
      "Missing Target Metadata",
      "ParameterTarget without target metadata",
      320.0f,
      0.0f });

    graph.nodes.push_back({
      3,
      ControlNodeKind::ParameterTarget,
      "Missing Circuit Target",
      "ParameterTarget pointing to missing Circuit parameter",
      480.0f,
      0.0f,
      ControlParameterTarget{ 999, "missing_cutoff" } });

    graph.connections.push_back({
      1,
      "",
      99,
      "" });

    return graph;
}

Circuit createValidationCircuit()
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

} // namespace

int main()
{
    const auto graph = createInvalidGraph();
    auto circuit = createValidationCircuit();
    printControlGraph(graph);
    const auto targetValidation =
      validateControlGraphTargets(graph, circuit);
    printControlGraphTargetValidation(targetValidation);
    const auto report = makeControlGraphReport(graph);
    printControlGraphReport(report);
    const auto wroteSnapshot =
      writeControlGraphSnapshotTextFile(
        report.snapshot,
        "runtime_control_graph_validation_demo.control_snapshot.txt");
    std::cout << "control snapshot file: "
              << (wroteSnapshot ? "wrote" : "failed")
              << "\n";
    const auto wroteReport =
      writeControlGraphReportTextFile(
        report,
        "runtime_control_graph_validation_demo.control_report.txt");
    std::cout << "control report file: "
              << (wroteReport ? "wrote" : "failed")
              << "\n";
    const auto applyReport =
      makeControlGraphApplyReport(graph, { 1, 0.25f }, circuit);
    printControlGraphApplyReport(applyReport);
    const auto wroteApplyReport =
      writeControlGraphApplyReportTextFile(
        applyReport,
        "runtime_control_graph_validation_demo.control_apply_report.txt");
    std::cout << "control apply report file: "
              << (wroteApplyReport ? "wrote" : "failed")
              << "\n";
    return 0;
}
