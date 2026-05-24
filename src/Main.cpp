#include <soemdsp/soemdsp.hpp>

#include <iostream>
#include <memory>

int main()
{
    using namespace soemdsp::runtime;
    using namespace soemdsp::runtime::nodes;

    Circuit circuit;

    // =====================================================
    // CREATE NODES
    // =====================================================

    auto lfoFreq    = std::make_unique<FloatConstant>(20.0f);
    auto lfo        = std::make_unique<SineOscillator>();

    auto lfoDepth   = std::make_unique<FloatConstant>(2000.0f);
    auto carrierHz  = std::make_unique<FloatConstant>(440.0f);

    auto modulation = std::make_unique<AudioMultiplyAdd>();

    auto carrier    = std::make_unique<SineOscillator>();

    auto output     = std::make_unique<AudioOutput>();

    // =====================================================
    // CACHE PORTS
    // =====================================================

    auto* lfoFreqOut = &lfoFreq->outputs[0];

    auto* lfoFreqIn = &lfo->inputs[0];
    auto* lfoOut    = &lfo->outputs[0];

    auto* depthOut = &lfoDepth->outputs[0];
    auto* baseOut  = &carrierHz->outputs[0];

    auto* modValue  = &modulation->inputs[0];
    auto* modScale  = &modulation->inputs[1];
    auto* modOffset = &modulation->inputs[2];
    auto* modOut    = &modulation->outputs[0];

    auto* carrierFreq = &carrier->inputs[0];
    auto* carrierOut  = &carrier->outputs[0];

    auto* outputIn = &output->inputs[0];

    // =====================================================
    // MOVE INTO CIRCUIT
    // =====================================================

    circuit.nodes.push_back(std::move(lfoFreq));    // 0
    circuit.nodes.push_back(std::move(lfo));        // 1
    circuit.nodes.push_back(std::move(lfoDepth));   // 2
    circuit.nodes.push_back(std::move(carrierHz));  // 3
    circuit.nodes.push_back(std::move(modulation)); // 4
    circuit.nodes.push_back(std::move(carrier));    // 5
    circuit.nodes.push_back(std::move(output));     // 6

    // =====================================================
    // CONNECTIONS
    // =====================================================

    // LFO frequency
    circuit.connect(
        *circuit.nodes[0],
        *lfoFreqOut,
        *circuit.nodes[1],
        *lfoFreqIn
    );

    // LFO output -> modulation value
    circuit.connect(
        *circuit.nodes[1],
        *lfoOut,
        *circuit.nodes[4],
        *modValue
    );

    // depth -> modulation scale
    circuit.connect(
        *circuit.nodes[2],
        *depthOut,
        *circuit.nodes[4],
        *modScale
    );

    // base pitch -> modulation offset
    circuit.connect(
        *circuit.nodes[3],
        *baseOut,
        *circuit.nodes[4],
        *modOffset
    );

    // modulation result -> carrier frequency
    circuit.connect(
        *circuit.nodes[4],
        *modOut,
        *circuit.nodes[5],
        *carrierFreq
    );

    // carrier -> output
    circuit.connect(
        *circuit.nodes[5],
        *carrierOut,
        *circuit.nodes[6],
        *outputIn
    );

    circuit.output = outputIn;

    // =====================================================
    // PROCESS
    // =====================================================

    circuit.process();

    // =====================================================
    // PRINT
    // =====================================================

    std::cout << "soemdsp v "
              << SOEMDSP_VERSION_STRING
              << std::endl;

    std::cout << "[LFO] ";
    for (std::size_t i = 0; i < 8; ++i)
    {
        std::cout << lfoOut->audioBuffer[i] << " ";
    }
    std::cout << "\n";

    std::cout << "[MOD] ";
    for (std::size_t i = 0; i < 8; ++i)
    {
        std::cout << modOut->audioBuffer[i] << " ";
    }
    std::cout << "\n";

    auto* finalOut = circuit.outputBuffer();

    std::cout << "[OUT] ";
    for (std::size_t i = 0; i < 8; ++i)
    {
        std::cout << finalOut[i] << " ";
    }
    std::cout << "\n";

    return 0;
}
