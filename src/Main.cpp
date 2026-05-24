#include <iostream>
#include <memory>
#include <soemdsp/soemdsp.hpp>

int main()
{
    //Shorten namespace typing
    using namespace soemdsp::runtime;
    using namespace soemdsp::runtime::nodes;

    //Main graph object that owns:
    //- nodes
    //- connections
    //- audio buffers
    Graph graph;

    auto freq = std::make_unique<FloatConstant>(440.0f);
    auto ramp = std::make_unique<RampGenerator>();

    auto* freqOut = &freq->outputs[0];
    auto* rampFreq = &ramp->inputs[0];
    auto* rampOut = &ramp->outputs[0];

    graph.nodes.push_back(std::move(freq)); // 0
    graph.nodes.push_back(std::move(ramp)); // 1

    graph.connect(
    *graph.nodes[0],
    *freqOut,
    *graph.nodes[1],
    *rampFreq
);
    graph.process();

    std::cout << std::endl
              << std::endl
              << std::endl
              << "soemdsp v"
              << SOEMDSP_VERSION_STRING
              << std::endl;

    std::cout << "[RAMP] ";
    for (std::size_t i = 0; i < 8; ++i)
    {
        std::cout << rampOut->audioBuffer[i] << " ";
    }
    std::cout << std::endl;

    std::cout << std::endl;

    return 0;
}
