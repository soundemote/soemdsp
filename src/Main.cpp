#include <soemdsp/soemdsp.hpp>

#include <iostream>
#include <memory>

int main()
{
    using namespace soemdsp::runtime;
    using namespace soemdsp::runtime::nodes;

    Circuit circuit;

    auto value  = std::make_unique<FloatConstant>(7.0f);
    auto scale  = std::make_unique<FloatConstant>(3.0f);
    auto offset = std::make_unique<FloatConstant>(100.0f);
    auto madd   = std::make_unique<MultiplyAdd>();

    auto* valueOut  = &value->outputs[0];
    auto* scaleOut  = &scale->outputs[0];
    auto* offsetOut = &offset->outputs[0];

    auto* maddValue  = &madd->inputs[0];
    auto* maddScale  = &madd->inputs[1];
    auto* maddOffset = &madd->inputs[2];
    auto* maddOut    = &madd->outputs[0];

    circuit.nodes.push_back(std::move(value));
    circuit.nodes.push_back(std::move(scale));
    circuit.nodes.push_back(std::move(offset));
    circuit.nodes.push_back(std::move(madd));

    circuit.connect(*circuit.nodes[0], *valueOut,  *circuit.nodes[3], *maddValue);
    circuit.connect(*circuit.nodes[1], *scaleOut,  *circuit.nodes[3], *maddScale);
    circuit.connect(*circuit.nodes[2], *offsetOut, *circuit.nodes[3], *maddOffset);

    circuit.process();

    std::cout << "soemdsp v"
              << SOEMDSP_VERSION_STRING
              << std::endl;

    std::cout << "[MADD] "
              << maddOut->value
              << std::endl;

    return 0;
}
