// clang-format off
#include <soemdsp/soemdsp.hpp>
// clang-format on

#include <iostream>
#include <memory>

int main()
{
    using namespace soemdsp::runtime;
    using namespace soemdsp::runtime::nodes;

    Graph graph;

    //Nodes
    auto a = std::make_unique<Constant>(137.42857f);
    auto b = std::make_unique<Constant>(9021.333f);
    auto c = std::make_unique<Constant>(0.03125f);
    auto d = std::make_unique<Constant>(444.4444f);

    auto add1 = std::make_unique<Add>();
    auto add2 = std::make_unique<Add>();
    auto mul  = std::make_unique<Multiply>();

    //Cache raw pointers BEFORE move
    auto* aOut = &a->outputs[0];
    auto* bOut = &b->outputs[0];
    auto* cOut = &c->outputs[0];
    auto* dOut = &d->outputs[0];

    auto* add1In0 = &add1->inputs[0];
    auto* add1In1 = &add1->inputs[1];
    auto* add1Out = &add1->outputs[0];

    auto* add2In0 = &add2->inputs[0];
    auto* add2In1 = &add2->inputs[1];
    auto* add2Out = &add2->outputs[0];

    auto* mulIn0 = &mul->inputs[0];
    auto* mulIn1 = &mul->inputs[1];
    auto* mulOut = &mul->outputs[0];

    //Move nodes into graph
    graph.nodes.push_back(std::move(a));    //0
    graph.nodes.push_back(std::move(b));    //1
    graph.nodes.push_back(std::move(c));    //2
    graph.nodes.push_back(std::move(d));    //3
    graph.nodes.push_back(std::move(add1)); //4
    graph.nodes.push_back(std::move(add2)); //5
    graph.nodes.push_back(std::move(mul));  //6

    //Connections
    graph.connections.push_back({ graph.nodes[0].get(), aOut, graph.nodes[4].get(), add1In0 });
    graph.connections.push_back({ graph.nodes[1].get(), bOut, graph.nodes[4].get(), add1In1 });

    graph.connections.push_back({ graph.nodes[2].get(), cOut, graph.nodes[5].get(), add2In0 });
    graph.connections.push_back({ graph.nodes[3].get(), dOut, graph.nodes[5].get(), add2In1 });

    graph.connections.push_back({ graph.nodes[4].get(), add1Out, graph.nodes[6].get(), mulIn0 });
    graph.connections.push_back({ graph.nodes[5].get(), add2Out, graph.nodes[6].get(), mulIn1 });

 

    std::array<float, 8> buffer {};
    auto audio = std::make_unique<AudioConstant>(0.25f);
auto* audioOut = &audio->outputs[0];
audioOut->audioBuffer = buffer.data();
audioOut->audioFrames = buffer.size();

graph.nodes.push_back(std::move(audio));

   //Process graph
    graph.process();

    //Final result
std::cout << "[AUDIO] ";
for (auto v : buffer)
{
    std::cout << v << " ";
}
std::cout << std::endl;
    return 0;
}
