#include <iostream>

#include <soemdsp/runtime/control/PrintControlGraph.hpp>
#include <soemdsp/runtime/control/PrintControlGraphReport.hpp>
#include <soemdsp/runtime/control/WriteControlGraphSnapshot.hpp>
#include <soemdsp/runtime/control/WriteControlGraphReport.hpp>

using namespace soemdsp::runtime;

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

    graph.connections.push_back({
      1,
      "",
      99,
      "" });

    return graph;
}

} // namespace

int main()
{
    const auto graph = createInvalidGraph();
    printControlGraph(graph);
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
    return 0;
}
