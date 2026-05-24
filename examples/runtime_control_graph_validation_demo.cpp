#include <soemdsp/runtime/control/PrintControlGraph.hpp>
#include <soemdsp/runtime/control/PrintControlGraphValidation.hpp>

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
    const auto report = validateControlGraph(graph);
    printControlGraphValidation(report);
    return 0;
}
