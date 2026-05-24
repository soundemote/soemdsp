#include <iomanip>
#include <iostream>
#include <memory>

#include <soemdsp/soemdsp.hpp>

int main()
{
    using namespace soemdsp::runtime;
    using namespace soemdsp::runtime::debug;
    using namespace soemdsp::runtime::nodes;

    Circuit circuit;

    auto node = std::make_unique<FloatConstant>(0.5f);
    auto* gainNode = node.get();

    Parameter gain;
    gain.id           = "gain";
    gain.name         = "Gain";
    gain.value        = 0.5f;
    gain.defaultValue = 0.5f;
    gain.minValue     = 0.0f;
    gain.midValue     = 0.5f;
    gain.maxValue     = 1.0f;

    gainNode->parameters.push_back(gain);
    circuit.nodes.push_back(std::move(node));

    circuit.prepare();

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "parameter count: "
              << circuit.parameterCount()
              << "\n";

    std::cout << "[BEFORE]\n";
    std::cout << "dirty: "
              << std::boolalpha
              << circuit.isParameterDirty(gainNode->id, "gain")
              << "\n";
    std::cout << "dirty count: "
              << circuit.dirtyParameterCount()
              << "\n";
    printCircuit(circuit);

    circuit.setParameterValue(gainNode->id, "gain", 2.0f);

    std::cout << "\n[AFTER SET]\n";
    std::cout << "dirty: "
              << circuit.isParameterDirty(gainNode->id, "gain")
              << "\n";
    std::cout << "dirty count: "
              << circuit.dirtyParameterCount()
              << "\n";
    printCircuit(circuit);

    circuit.resetParameterValue(gainNode->id, "gain");

    std::cout << "\n[AFTER RESET]\n";
    std::cout << "dirty: "
              << circuit.isParameterDirty(gainNode->id, "gain")
              << "\n";
    std::cout << "dirty count: "
              << circuit.dirtyParameterCount()
              << "\n";
    printCircuit(circuit);

    circuit.setParameterNormalizedValue(gainNode->id, "gain", 0.25f);

    std::cout << "\n[AFTER NORMALIZED SET]\n";
    std::cout << "dirty: "
              << circuit.isParameterDirty(gainNode->id, "gain")
              << "\n";
    std::cout << "dirty count: "
              << circuit.dirtyParameterCount()
              << "\n";
    printCircuit(circuit);

    return 0;
}
