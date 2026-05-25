#include <iostream>
#include <memory>

#include <soemdsp/runtime/dsp/ApplyDspBinding.hpp>
#include <soemdsp/runtime/dsp/PrintDspBindingApplyResult.hpp>
#include <soemdsp/soemdsp.hpp>

using namespace soemdsp::runtime;
using namespace soemdsp::runtime::nodes;

namespace
{

struct TinyModulatedGainDsp
{
    float* gain{};
    float* modulation{};

    float processSample(float x) const noexcept
    {
        const float g = gain != nullptr ? *gain : 1.0f;
        const float m = modulation != nullptr ? *modulation : 0.0f;
        return x * (g + m);
    }
};

std::unique_ptr<FloatConstant> createNodeWithParameter(
  NodeId nodeId,
  const char* parameterId,
  const char* name,
  float value)
{
    auto node = std::make_unique<FloatConstant>(0.0f);
    node->id = nodeId;
    node->name = name;

    Parameter parameter;
    parameter.id = parameterId;
    parameter.name = name;
    parameter.value = value;
    parameter.defaultValue = value;
    parameter.minValue = 0.0f;
    parameter.midValue = 1.0f;
    parameter.maxValue = 4.0f;
    node->parameters.push_back(parameter);

    return node;
}

Circuit createCircuit()
{
    Circuit circuit;

    circuit.nodes.push_back(
      createNodeWithParameter(1, "gain", "Gain", 2.0f));
    circuit.nodes.push_back(
      createNodeWithParameter(2, "modulation", "Modulation", 0.5f));
    circuit.prepare();

    return circuit;
}

DspObjectBinding createBinding(
  float& gainMemory,
  float& modulationMemory)
{
    DspObjectBinding binding;
    binding.nodeId = 1;
    binding.objectType = "TinyModulatedGainDsp";
    binding.objectName = "Tiny Modulated Gain DSP";

    binding.parameterBindings.push_back({
      1,
      "gain",
      DspBindingTargetKind::MemorySlot,
      "gain",
      0,
      &gainMemory });

    binding.parameterBindings.push_back({
      2,
      "modulation",
      DspBindingTargetKind::MemorySlot,
      "modulation",
      1,
      &modulationMemory });

    return binding;
}

} // namespace

int main()
{
    auto circuit = createCircuit();
    float gainMemory = 0.0f;
    float modulationMemory = 0.0f;
    TinyModulatedGainDsp dsp{ &gainMemory, &modulationMemory };
    const auto binding = createBinding(gainMemory, modulationMemory);

    std::cout << "[DSP OBJECT MULTI NODE SYNC]\n";
    std::cout << "gain memory before: "
              << gainMemory
              << "\n";
    std::cout << "modulation memory before: "
              << modulationMemory
              << "\n";

    const auto applyResult =
      applyDspParameterBindings(binding, circuit);
    printDspBindingApplyResult(applyResult);

    std::cout << "gain memory after: "
              << gainMemory
              << "\n";
    std::cout << "modulation memory after: "
              << modulationMemory
              << "\n";
    std::cout << "processSample(0.25): "
              << dsp.processSample(0.25f)
              << "\n";

    return 0;
}
