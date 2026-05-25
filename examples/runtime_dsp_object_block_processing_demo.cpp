#include <array>
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
    parameter.minValue = -4.0f;
    parameter.midValue = 0.0f;
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
      createNodeWithParameter(2, "bias", "Bias", 0.25f));
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
    float biasMemory = 0.0f;
    TinyGainDsp gain{ &gainMemory };
    TinyBiasDsp bias{ &biasMemory };

    const auto gainBinding = createGainBinding(gainMemory);
    const auto biasBinding = createBiasBinding(biasMemory);

    std::cout << "[DSP OBJECT BLOCK PROCESSING]\n";

    const auto gainApply =
      applyDspParameterBindings(gainBinding, circuit);
    printSummary("[GAIN APPLY]", gainApply);

    const auto biasApply =
      applyDspParameterBindings(biasBinding, circuit);
    printSummary("[BIAS APPLY]", biasApply);

    constexpr std::array<float, 4> inputBlock{
      0.0f,
      0.25f,
      0.5f,
      1.0f
    };

    std::array<float, inputBlock.size()> outputBlock{};

    for (std::size_t i = 0; i < inputBlock.size(); ++i)
    {
        const auto afterGain = gain.processSample(inputBlock[i]);
        outputBlock[i] = bias.processSample(afterGain);
    }

    std::cout << "gainMemory after apply: "
              << gainMemory
              << "\n";
    std::cout << "biasMemory after apply: "
              << biasMemory
              << "\n";

    std::cout << "input block:";
    for (const auto sample : inputBlock)
    {
        std::cout << " "
                  << sample;
    }
    std::cout << "\n";

    std::cout << "output block:";
    for (const auto sample : outputBlock)
    {
        std::cout << " "
                  << sample;
    }
    std::cout << "\n";

    return 0;
}
