#include <iostream>

#include <soemdsp/runtime/control/PrintControlGraph.hpp>
#include <soemdsp/runtime/control/PrintControlGraphValidation.hpp>
#include <soemdsp/runtime/control/PrintControlGraphSnapshot.hpp>
#include <soemdsp/runtime/control/WriteControlGraphSnapshot.hpp>

using namespace soemdsp::runtime;

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

    graph.connections.push_back(makeConnection(1, "value", 2, "input"));
    graph.connections.push_back(makeConnection(2, "output", 3, "input"));
    graph.connections.push_back(makeConnection(3, "output", 4, "input"));
    graph.connections.push_back(makeConnection(4, "output", 5, "value"));

    return graph;
}

} // namespace

int main()
{
    const auto graph = createDemoGraph();
    printControlGraph(graph);
    const auto report = validateControlGraph(graph);
    printControlGraphValidation(report);
    const auto snapshot = snapshotControlGraph(graph);
    printControlGraphSnapshot(snapshot);
    const auto wroteSnapshot =
      writeControlGraphSnapshotTextFile(
        snapshot,
        "runtime_control_graph_demo.control_snapshot.txt");
    std::cout << "control snapshot file: "
              << (wroteSnapshot ? "wrote" : "failed")
              << "\n";
    return 0;
}
