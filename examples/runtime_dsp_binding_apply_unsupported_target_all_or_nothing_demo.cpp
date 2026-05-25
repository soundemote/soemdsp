#include <iostream>
#include <memory>

#include <soemdsp/runtime/dsp/ApplyDspBinding.hpp>
#include <soemdsp/runtime/dsp/PrintDspBindingApplyResult.hpp>
#include <soemdsp/soemdsp.hpp>

using namespace soemdsp::runtime;
using namespace soemdsp::runtime::nodes;

namespace
{

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
    parameter.minValue = -8.0f;
    parameter.midValue = 0.0f;
    parameter.maxValue = 8.0f;
    node->parameters.push_back(parameter);

    return node;
}

Circuit createCircuit()
{
    Circuit circuit;

    circuit.nodes.push_back(
      createNodeWithParameter(1, "gain", "Gain", 2.0f));
    circuit.nodes.push_back(
      createNodeWithParameter(2, "bias", "Bias", 0.25f));
    circuit.prepare();

    return circuit;
}

DspObjectBinding createBinding(float& gainMemory, float& biasMemory)
{
    DspObjectBinding binding;
    binding.nodeId = 1;
    binding.objectType = "TinyGainBiasDsp";
    binding.objectName = "Tiny Gain Bias DSP";

    binding.parameterBindings.push_back({
      1,
      "gain",
      DspBindingTargetKind::MemorySlot,
      "gain",
      0,
      &gainMemory });

    binding.parameterBindings.push_back({
      2,
      "bias",
      DspBindingTargetKind::SemanticSetter,
      "setBias",
      1,
      &biasMemory });

    return binding;
}

} // namespace

int main()
{
    auto circuit = createCircuit();
    float gainMemory = 77.0f;
    float biasMemory = 88.0f;
    const auto binding = createBinding(gainMemory, biasMemory);

    std::cout << "[DSP BINDING UNSUPPORTED TARGET ALL OR NOTHING]\n";
    std::cout << "gainMemory before apply: "
              << gainMemory
              << "\n";
    std::cout << "biasMemory before apply: "
              << biasMemory
              << "\n";

    const auto result =
      applyDspParameterBindings(binding, circuit);
    printDspBindingApplyResult(result);

    std::cout << "gainMemory after apply: "
              << gainMemory
              << "\n";
    std::cout << "biasMemory after apply: "
              << biasMemory
              << "\n";

    return 0;
}

