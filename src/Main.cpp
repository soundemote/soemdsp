#include <soemdsp/soemdsp.hpp>

#include <iostream>
#include <memory>

int main()
{
    using namespace soemdsp::runtime;
    using namespace soemdsp::runtime::nodes;

    Graph graph;

    auto a = std::make_unique<AudioConstant>(0.25f);
    auto b = std::make_unique<AudioConstant>(0.50f);
    auto add = std::make_unique<AudioAdd>();

    auto* aOut = &a->outputs[0];
    auto* bOut = &b->outputs[0];

    auto* addIn0 = &add->inputs[0];
    auto* addIn1 = &add->inputs[1];
    auto* addOut = &add->outputs[0];

    graph.nodes.push_back(std::move(a));   // 0
    graph.nodes.push_back(std::move(b));   // 1
    graph.nodes.push_back(std::move(add)); // 2

    graph.connections.push_back({
        graph.nodes[0].get(),
        aOut,
        graph.nodes[2].get(),
        addIn0
    });

    graph.connections.push_back({
        graph.nodes[1].get(),
        bOut,
        graph.nodes[2].get(),
        addIn1
    });

    graph.process();

    std::cout << "soemdsp v"
              << SOEMDSP_VERSION_STRING
              << std::endl;

    std::cout << "[AUDIO] ";
    for (std::size_t i = 0; i < 8; ++i)
    {
        std::cout << addOut->audioBuffer[i] << " ";
    }
    std::cout << std::endl;

    return 0;
}
