#include <iostream>
#include <memory>

#include <soemdsp/runtime/dsp/ApplyDspBinding.hpp>
#include <soemdsp/runtime/dsp/PrintDspBindingApplySummary.hpp>
#include <soemdsp/soemdsp.hpp>

using namespace soemdsp::runtime;
using namespace soemdsp::runtime::nodes;

namespace
{

struct TinyGainDsp
{
    float* gain{};

    float processSample(float x) const noexcept
    {
        return gain != nullptr ? x * (*gain) : x;
    }
};

struct TinyBiasDsp
{
    float* bias{};

    float processSample(float x) const noexcept
    {
        return x + (bias != nullptr ? *bias : 0.0f);
    }
};

Circuit createCircuit()
{
    Circuit circuit;

    auto node = std::make_unique<FloatConstant>(0.0f);
    node->id = 1;
    node->name = "Gain";

    Parameter gain;
    gain.id = "gain";
    gain.name = "Gain";
    gain.value = 2.0f;
    gain.defaultValue = 2.0f;
    gain.minValue = 0.0f;
    gain.midValue = 1.0f;
    gain.maxValue = 4.0f;
    node->parameters.push_back(gain);

    circuit.nodes.push_back(std::move(node));
    circuit.prepare();

    return circuit;
}

DspObjectBinding createGainBinding(float& gainMemory)
{
    DspObjectBinding binding;
    binding.nodeId = 1;
    binding.objectType = "TinyGainDsp";
    binding.objectName = "Tiny Gain DSP";

    binding.parameterBindings.push_back({
      1,
      "gain",
      DspBindingTargetKind::MemorySlot,
      "gain",
      0,
      &gainMemory });

    return binding;
}

DspObjectBinding createBiasBinding(float& biasMemory)
{
    DspObjectBinding binding;
    binding.nodeId = 2;
    binding.objectType = "TinyBiasDsp";
    binding.objectName = "Tiny Bias DSP";

    binding.parameterBindings.push_back({
      2,
      "bias",
      DspBindingTargetKind::MemorySlot,
      "bias",
      0,
      &biasMemory });

    return binding;
}

void printSummary(
  const char* label,
  const DspBindingApplyResult& result)
{
    std::cout << label
              << "\n";
    printDspBindingApplySummary(
      makeDspBindingApplySummary(result));
}

} // namespace

int main()
{
    auto circuit = createCircuit();
    float gainMemory = 0.0f;
    float biasMemory = 99.0f;
    TinyGainDsp gainDsp{ &gainMemory };
    TinyBiasDsp biasDsp{ &biasMemory };

    const auto gainBinding = createGainBinding(gainMemory);
    const auto biasBinding = createBiasBinding(biasMemory);

    std::cout << "[DSP BINDING APPLY BATCH FAILURE]\n";
    std::cout << "gain memory before: "
              << gainMemory
              << "\n";
    std::cout << "bias memory before: "
              << biasMemory
              << "\n";

    const auto gainApply =
      applyDspParameterBindings(gainBinding, circuit);
    printSummary("[GAIN APPLY]", gainApply);

    const auto biasApply =
      applyDspParameterBindings(biasBinding, circuit);
    printSummary("[BIAS APPLY]", biasApply);

    std::cout << "gain memory after: "
              << gainMemory
              << "\n";
    std::cout << "bias memory after: "
              << biasMemory
              << "\n";
    std::cout << "gain processSample(0.5): "
              << gainDsp.processSample(0.5f)
              << "\n";
    std::cout << "bias processSample(0.5): "
              << biasDsp.processSample(0.5f)
              << "\n";

    return 0;
}
