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

    auto osc = std::make_unique<RampGenerator>();

    auto* freqIn = &osc->inputs[0];
    auto* oscOut = &osc->outputs[0];

    freqIn->value = 440.0f;

    graph.nodes.push_back(std::move(osc));

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
        std::cout << oscOut->audioBuffer[i] << " ";
    }
    std::cout << std::endl;

    std::cout << std::endl;

    return 0;
}
