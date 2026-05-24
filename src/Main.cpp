#include <iostream>
#include <memory>
#include <soemdsp/soemdsp.hpp>

int main()
{
    using namespace soemdsp::runtime;
    using namespace soemdsp::runtime::nodes;
    using namespace soemdsp::runtime::debug;

    Circuit circuit;

    //=====================================================
    //CREATE NODES
    //=====================================================

    auto lfoFreq = std::make_unique<FloatConstant>(20.0f);
    auto lfo     = std::make_unique<SineOscillator>();

    auto lfoDepth  = std::make_unique<FloatConstant>(2000.0f);
    auto carrierHz = std::make_unique<FloatConstant>(440.0f);

    auto modulation = std::make_unique<AudioMultiplyAdd>();

    auto carrier = std::make_unique<SineOscillator>();

    auto output = std::make_unique<AudioOutput>();

    auto triggerButton  = std::make_unique<TriggerButton>();
    auto triggerPrinter = std::make_unique<TriggerPrinter>();

    //=====================================================
    //CACHE PORTS
    //=====================================================

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

    auto* triggerOut = &triggerButton->outputs[0];
    auto* triggerIn  = &triggerPrinter->inputs[0];

    //=====================================================
    //MOVE INTO CIRCUIT
    //=====================================================

    circuit.nodes.push_back(std::move(lfoFreq));    //0
    circuit.nodes.push_back(std::move(lfo));        //1
    circuit.nodes.push_back(std::move(lfoDepth));   //2
    circuit.nodes.push_back(std::move(carrierHz));  //3
    circuit.nodes.push_back(std::move(modulation)); //4
    circuit.nodes.push_back(std::move(carrier));    //5
    circuit.nodes.push_back(std::move(output));     //6
    circuit.nodes.push_back(std::move(triggerButton));  //7
    circuit.nodes.push_back(std::move(triggerPrinter)); //8

    //=====================================================
    //CONNECTIONS
    //=====================================================

    //LFO frequency
    circuit.connect(
      *circuit.nodes[0],
      *lfoFreqOut,
      *circuit.nodes[1],
      *lfoFreqIn);

    //LFO output -> modulation value
    circuit.connect(
      *circuit.nodes[1],
      *lfoOut,
      *circuit.nodes[4],
      *modValue);

    //depth -> modulation scale
    circuit.connect(
      *circuit.nodes[2],
      *depthOut,
      *circuit.nodes[4],
      *modScale);

    //base pitch -> modulation offset
    circuit.connect(
      *circuit.nodes[3],
      *baseOut,
      *circuit.nodes[4],
      *modOffset);

    //modulation result -> carrier frequency
    circuit.connect(
      *circuit.nodes[4],
      *modOut,
      *circuit.nodes[5],
      *carrierFreq);

    //carrier -> output
    circuit.connect(
      *circuit.nodes[5],
      *carrierOut,
      *circuit.nodes[6],
      *outputIn);

    circuit.connect(
      *circuit.nodes[7],
      *triggerOut,
      *circuit.nodes[8],
      *triggerIn);

    circuit.output = outputIn;

    //=====================================================
    //PROCESS
    //=====================================================

    circuit.prepare();
    validateCircuit(circuit);
   printNodes(circuit);

    std::cout << "\n\n";

    static_cast<TriggerButton*>(
      circuit.nodes[7].get())
      ->trigger();

    std::cout << "[PROCESS 1]\n";

    static_cast<TriggerButton*>(
      circuit.nodes[7].get())
      ->trigger();

    circuit.process();

    std::cout << "[PROCESS 2]\n";

    circuit.process();

    std::cout << "[PROCESS 3]\n";

    static_cast<TriggerButton*>(
      circuit.nodes[7].get())
      ->trigger();

    circuit.reset();

    std::cout << "\n[DISCONNECT 5]\n";

    circuit.disconnect(5);

    circuit.process();

    //=====================================================
    //PRINT
    //=====================================================

    std::cout << "<3 soemdsp v "
              << SOEMDSP_VERSION_STRING 
              << std::endl;

    printConnections(circuit);

    printAudio("LFO", lfoOut->audioBuffer, 8);
    printAudio("MOD", modOut->audioBuffer, 8);
    printAudio("OUT", circuit.outputBuffer(), 8);

    return 0;
}
