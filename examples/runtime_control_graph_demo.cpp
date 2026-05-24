#include <soemdsp/runtime/control/PrintControlGraph.hpp>

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
    return 0;
}
