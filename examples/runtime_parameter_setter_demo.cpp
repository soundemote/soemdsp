#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string_view>
#include <thread>

#include <soemdsp/runtime/debug/ConsoleStyle.hpp>
#include <soemdsp/runtime/serialization/PrintCircuitSnapshot.hpp>
#include <soemdsp/runtime/serialization/WriteCircuitSnapshot.hpp>
#include <soemdsp/soemdsp.hpp>

namespace
{

using namespace soemdsp::runtime;
using namespace soemdsp::runtime::debug;
using namespace soemdsp::runtime::nodes;

struct DemoState
{
    Circuit circuit;
    Node* gainNode{};
};

enum class Stage
{
    Initial,
    RawSet,
    Reset,
    NormalizedSet,
    ResetAll,
    Snapshot
};

bool hasArg(int argc, char** argv, std::string_view target)
{
    for (int i = 1; i < argc; ++i)
    {
        if (argv[i] == target)
        {
            return true;
        }
    }

    return false;
}

DemoState createDemoState()
{
    DemoState state;

    auto node = std::make_unique<FloatConstant>(0.5f);
    state.gainNode = node.get();

    Parameter gain;
    gain.id           = "gain";
    gain.name         = "Gain";
    gain.value        = 0.5f;
    gain.defaultValue = 0.5f;
    gain.minValue     = 0.0f;
    gain.midValue     = 0.5f;
    gain.maxValue     = 1.0f;

    state.gainNode->parameters.push_back(gain);
    state.circuit.nodes.push_back(std::move(node));
    state.circuit.prepare();

    return state;
}

void clearScreen()
{
    if (console::enabled())
    {
        std::cout << "\x1b[2J\x1b[H";
    }
}

void printHeader()
{
    console::title(
      std::cout,
      "soemdsp-sandbox / runtime_parameter_setter_demo");
    console::info(std::cout, "44.1kHz | block 64 | debug");
    console::divider(std::cout);
}

void printGainMeter(float value)
{
    constexpr int width = 20;
    const int filled =
      static_cast<int>(value * static_cast<float>(width));

    std::cout << console::style(console::yellow)
              << "gain [";

    for (int i = 0; i < width; ++i)
    {
        std::cout << (i < filled ? '#' : '-');
    }

    std::cout << "] "
              << value
              << console::style(console::reset)
              << "\n";
}

void printAsciiScope(float gain, int frame)
{
    constexpr int width = 56;
    constexpr int height = 11;
    constexpr float pi = 3.14159265358979323846f;
    const float amplitude =
      gain * static_cast<float>((height - 1) / 2);
    const float center =
      static_cast<float>(height - 1) * 0.5f;
    const float phase =
      static_cast<float>(frame) * 0.42f;

    std::cout << console::style(console::gray)
              << "scope"
              << console::style(console::reset)
              << "\n";

    for (int y = 0; y < height; ++y)
    {
        std::cout << console::style(console::cyan);

        for (int x = 0; x < width; ++x)
        {
            const float t =
              (static_cast<float>(x) / static_cast<float>(width)) *
              2.0f * pi * 2.0f;
            const float sample =
              center - (std::sin(t + phase) * amplitude);
            const int traceY =
              static_cast<int>(sample + 0.5f);

            if (y == traceY)
            {
                std::cout << '*';
            }
            else if (y == static_cast<int>(center))
            {
                std::cout << '-';
            }
            else
            {
                std::cout << ' ';
            }
        }

        std::cout << console::style(console::reset) << "\n";
    }
}

void printParameterState(
  const Circuit& circuit,
  NodeId nodeId,
  int frame)
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
        printAsciiScope(parameter->value, frame);
    }
}

void printSnapshot(const Circuit& circuit)
{
    const auto snapshot = circuit.parameterSnapshot();
    const auto circuitSnapshot = circuit.snapshot();

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

    std::cout << "circuit snapshot: nodes "
              << circuitSnapshot.nodes.size()
              << " | ports "
              << circuitSnapshot.ports.size()
              << " | parameters "
              << circuitSnapshot.parameters.size()
              << " | connections "
              << circuitSnapshot.connections.size()
              << "\n";
    printCircuitSnapshot(circuitSnapshot);

    const auto wroteSnapshot =
      writeCircuitSnapshotTextFile(
        circuitSnapshot,
        "runtime_parameter_setter_demo.snapshot.txt");
    std::cout << "snapshot file: "
              << (wroteSnapshot ? "wrote" : "failed")
              << "\n";

    console::debug(std::cout, "presentation-only demo output");
}

void runStage(DemoState& state, Stage stage, bool repaint, int frame)
{
    if (repaint)
    {
        clearScreen();
    }

    printHeader();

    switch (stage)
    {
        case Stage::Initial:
            state.circuit.resetAllParameterValues();
            console::section(std::cout, "initial state");
            printParameterState(state.circuit, state.gainNode->id, frame);
            printCircuit(state.circuit);
            break;

        case Stage::RawSet:
            state.circuit.setParameterValue(state.gainNode->id, "gain", 2.0f);
            console::section(std::cout, "raw set/clamp");
            console::success(std::cout, "set gain to 2.0f; clamped to max");
            printParameterState(state.circuit, state.gainNode->id, frame);
            printCircuit(state.circuit);
            break;

        case Stage::Reset:
            state.circuit.resetParameterValue(state.gainNode->id, "gain");
            console::section(std::cout, "reset");
            console::success(std::cout, "reset gain to default value");
            printParameterState(state.circuit, state.gainNode->id, frame);
            printCircuit(state.circuit);
            break;

        case Stage::NormalizedSet:
            state.circuit.setParameterNormalizedValue(state.gainNode->id, "gain", 0.25f);
            console::section(std::cout, "normalized set");
            console::success(std::cout, "set normalized gain to 0.25");
            printParameterState(state.circuit, state.gainNode->id, frame);
            printCircuit(state.circuit);
            break;

        case Stage::ResetAll:
        {
            const auto resetCount = state.circuit.resetAllParameterValues();
            console::section(std::cout, "reset all");
            std::cout << "reset count: "
                      << resetCount
                      << "\n";
            printParameterState(state.circuit, state.gainNode->id, frame);
            printCircuit(state.circuit);
        }
        break;

        case Stage::Snapshot:
            console::section(std::cout, "snapshot");
            printParameterState(state.circuit, state.gainNode->id, frame);
            printSnapshot(state.circuit);
            break;
    }

    std::cout.flush();
}

void runOnce(DemoState& state)
{
    runStage(state, Stage::Initial, false, 0);
    runStage(state, Stage::RawSet, false, 1);
    runStage(state, Stage::Reset, false, 2);
    runStage(state, Stage::NormalizedSet, false, 3);
    runStage(state, Stage::ResetAll, false, 4);
    runStage(state, Stage::Snapshot, false, 5);
}

void runLoop(DemoState& state)
{
    constexpr auto delay = std::chrono::milliseconds(900);
    int frame = 0;

    while (true)
    {
        runStage(state, Stage::Initial, true, frame++);
        std::this_thread::sleep_for(delay);
        runStage(state, Stage::RawSet, true, frame++);
        std::this_thread::sleep_for(delay);
        runStage(state, Stage::Reset, true, frame++);
        std::this_thread::sleep_for(delay);
        runStage(state, Stage::NormalizedSet, true, frame++);
        std::this_thread::sleep_for(delay);
        runStage(state, Stage::ResetAll, true, frame++);
        std::this_thread::sleep_for(delay);
        runStage(state, Stage::Snapshot, true, frame++);
        std::this_thread::sleep_for(delay);
    }
}

} // namespace

int main(int argc, char** argv)
{
    const bool loop = hasArg(argc, argv, "--loop");
    const bool color = !hasArg(argc, argv, "--no-color");

    console::setEnabled(color);

    std::cout << std::fixed << std::setprecision(2);

    auto state = createDemoState();

    if (loop)
    {
        runLoop(state);
    }

    runOnce(state);

    return 0;
}
