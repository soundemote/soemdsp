#include <iomanip>
#include <iostream>
#include <memory>

#include <soemdsp/runtime/debug/ConsoleStyle.hpp>
#include <soemdsp/soemdsp.hpp>

namespace
{

void printGainMeter(float value)
{
    constexpr int width = 20;
    const int filled =
      static_cast<int>(value * static_cast<float>(width));

    std::cout << soemdsp::runtime::debug::console::yellow
              << "gain [";

    for (int i = 0; i < width; ++i)
    {
        std::cout << (i < filled ? '#' : '-');
    }

    std::cout << "] "
              << value
              << soemdsp::runtime::debug::console::reset
              << "\n";
}

void printParameterState(
  const soemdsp::runtime::Circuit& circuit,
  soemdsp::runtime::NodeId nodeId)
{
    const auto* parameter =
      circuit.findParameter(nodeId, "gain");

    std::cout << "dirty: "
              << std::boolalpha
              << circuit.isParameterDirty(nodeId, "gain")
              << " | dirty count: "
              << circuit.dirtyParameterCount()
              << " | total: "
              << circuit.parameterCount()
              << "\n";

    if (parameter)
    {
        printGainMeter(parameter->value);
    }
}

} // namespace

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
    console::title(
      std::cout,
      "soemdsp-sandbox / runtime_parameter_setter_demo");
    console::info(std::cout, "44.1kHz | block 64 | debug");
    console::divider(std::cout);

    console::section(std::cout, "initial state");
    printParameterState(circuit, gainNode->id);
    printCircuit(circuit);

    circuit.setParameterValue(gainNode->id, "gain", 2.0f);

    console::section(std::cout, "raw set/clamp");
    console::success(std::cout, "set gain to 2.0f; clamped to max");
    printParameterState(circuit, gainNode->id);
    printCircuit(circuit);

    circuit.resetParameterValue(gainNode->id, "gain");

    console::section(std::cout, "reset");
    console::success(std::cout, "reset gain to default value");
    printParameterState(circuit, gainNode->id);
    printCircuit(circuit);

    circuit.setParameterNormalizedValue(gainNode->id, "gain", 0.25f);

    console::section(std::cout, "normalized set");
    console::success(std::cout, "set normalized gain to 0.25");
    printParameterState(circuit, gainNode->id);
    printCircuit(circuit);

    const auto resetCount = circuit.resetAllParameterValues();

    console::section(std::cout, "reset all");
    std::cout << "reset count: "
              << resetCount
              << "\n";
    printParameterState(circuit, gainNode->id);
    printCircuit(circuit);

    const auto snapshot = circuit.parameterSnapshot();

    console::section(std::cout, "snapshot");
    std::cout << "snapshot size: "
              << snapshot.size()
              << "\n";
    for (const auto& item : snapshot)
    {
        std::cout << "node "
                  << item.nodeId
                  << " param "
                  << item.parameterId
                  << " value "
                  << item.value
                  << "\n";
    }
    console::debug(std::cout, "presentation-only demo output");

    return 0;
}
