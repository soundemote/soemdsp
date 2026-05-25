#include <iostream>
#include <memory>

#include <soemdsp/runtime/dsp/ApplyDspBinding.hpp>
#include <soemdsp/runtime/dsp/PrintDspBindingApplyResult.hpp>
#include <soemdsp/soemdsp.hpp>

using namespace soemdsp::runtime;
using namespace soemdsp::runtime::nodes;

namespace
{

struct TinyAffineDsp
{
    float* gain{};
    float* bias{};

    float processSample(float x) const noexcept
    {
        const float g = gain != nullptr ? *gain : 1.0f;
        const float b = bias != nullptr ? *bias : 0.0f;
        return x * g + b;
    }
};

Circuit createCircuit()
{
    Circuit circuit;

    auto node = std::make_unique<FloatConstant>(0.0f);
    node->id = 1;

    Parameter gain;
    gain.id = "gain";
    gain.name = "Gain";
    gain.value = 2.0f;
    gain.defaultValue = 1.0f;
    gain.minValue = 0.0f;
    gain.midValue = 1.0f;
    gain.maxValue = 4.0f;
    node->parameters.push_back(gain);

    Parameter bias;
    bias.id = "bias";
    bias.name = "Bias";
    bias.value = 0.25f;
    bias.defaultValue = 0.0f;
    bias.minValue = -1.0f;
    bias.midValue = 0.0f;
    bias.maxValue = 1.0f;
    node->parameters.push_back(bias);

    circuit.nodes.push_back(std::move(node));
    circuit.prepare();

    return circuit;
}

DspObjectBinding createBinding(
  float& gainMemory,
  float& biasMemory)
{
    DspObjectBinding binding;
    binding.nodeId = 1;
    binding.objectType = "TinyAffineDsp";
    binding.objectName = "Tiny Affine DSP";

    binding.parameterBindings.push_back({
      1,
      "gain",
      DspBindingTargetKind::MemorySlot,
      "gain",
      0,
      &gainMemory });

    binding.parameterBindings.push_back({
      1,
      "bias",
      DspBindingTargetKind::MemorySlot,
      "bias",
      1,
      &biasMemory });

    return binding;
}

} // namespace

int main()
{
    auto circuit = createCircuit();
    float gainMemory = 0.0f;
    float biasMemory = 0.0f;
    TinyAffineDsp dsp{ &gainMemory, &biasMemory };
    const auto binding = createBinding(gainMemory, biasMemory);

    std::cout << "[DSP OBJECT MULTI PARAMETER SYNC]\n";
    std::cout << "gain memory before: "
              << gainMemory
              << "\n";
    std::cout << "bias memory before: "
              << biasMemory
              << "\n";

    const auto applyResult =
      applyDspParameterBindings(binding, circuit);
    printDspBindingApplyResult(applyResult);

    std::cout << "gain memory after: "
              << gainMemory
              << "\n";
    std::cout << "bias memory after: "
              << biasMemory
              << "\n";
    std::cout << "processSample(0.5): "
              << dsp.processSample(0.5f)
              << "\n";

    return 0;
}
