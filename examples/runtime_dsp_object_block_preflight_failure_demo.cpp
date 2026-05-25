#include <array>
#include <iostream>
#include <memory>

#include <soemdsp/runtime/dsp/ApplyDspBinding.hpp>
#include <soemdsp/runtime/dsp/PrintDspBindingApplySummary.hpp>
#include <soemdsp/runtime/dsp/ValidateDspBinding.hpp>
#include <soemdsp/runtime/dsp/ValidateDspBindingTargets.hpp>
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

struct DemoPreflightResult
{
    bool ok{ true };
    std::size_t bindingsChecked{};
    std::size_t messageCount{};
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

DspObjectBinding createBinding(
  NodeId nodeId,
  const char* objectType,
  const char* objectName,
  const char* parameterId,
  const char* targetName,
  float& memory)
{
    DspObjectBinding binding;
    binding.nodeId = nodeId;
    binding.objectType = objectType;
    binding.objectName = objectName;

    binding.parameterBindings.push_back({
      nodeId,
      parameterId,
      DspBindingTargetKind::MemorySlot,
      targetName,
      0,
      &memory });

    return binding;
}

DemoPreflightResult preflightBinding(
  const DspObjectBinding& binding,
  const Circuit& circuit)
{
    DemoPreflightResult result;
    result.bindingsChecked = 1;

    const auto structuralValidation =
      validateDspObjectBinding(binding);
    const auto targetValidation =
      validateDspObjectBindingTargets(binding, circuit);

    result.ok =
      structuralValidation.ok() &&
      targetValidation.ok();
    result.messageCount =
      structuralValidation.messageCount() +
      targetValidation.messageCount();

    return result;
}

DemoPreflightResult combinePreflight(
  const DemoPreflightResult& a,
  const DemoPreflightResult& b)
{
    DemoPreflightResult result;
    result.ok = a.ok && b.ok;
    result.bindingsChecked =
      a.bindingsChecked + b.bindingsChecked;
    result.messageCount =
      a.messageCount + b.messageCount;
    return result;
}

void printPreflight(
  const char* label,
  const DemoPreflightResult& result)
{
    std::cout << label
              << "\n";
    std::cout << "ok: "
              << (result.ok ? "true" : "false")
              << "\n";
    std::cout << "bindings checked: "
              << result.bindingsChecked
              << "\n";
    std::cout << "messages: "
              << result.messageCount
              << "\n";
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

template <std::size_t Size>
std::array<float, Size> processBlock(
  const TinyGainDsp& gain,
  const TinyBiasDsp& bias,
  const std::array<float, Size>& inputBlock)
{
    std::array<float, Size> outputBlock{};

    for (std::size_t i = 0; i < inputBlock.size(); ++i)
    {
        const auto afterGain = gain.processSample(inputBlock[i]);
        outputBlock[i] = bias.processSample(afterGain);
    }

    return outputBlock;
}

template <std::size_t Size>
void printBlock(
  const char* label,
  const std::array<float, Size>& block)
{
    std::cout << label;
    for (const auto sample : block)
    {
        std::cout << " "
                  << sample;
    }
    std::cout << "\n";
}

} // namespace

int main()
{
    auto circuit = createCircuit();
    float gainMemory = 0.0f;
    float biasMemory = 0.0f;
    TinyGainDsp gain{ &gainMemory };
    TinyBiasDsp bias{ &biasMemory };

    const auto gainBinding =
      createBinding(
        1,
        "TinyGainDsp",
        "Tiny Gain DSP",
        "gain",
        "gain",
        gainMemory);
    const auto biasBinding =
      createBinding(
        2,
        "TinyBiasDsp",
        "Tiny Bias DSP",
        "bias",
        "bias",
        biasMemory);

    constexpr std::array<float, 4> inputBlock{
      0.0f,
      0.25f,
      0.5f,
      1.0f
    };

    std::cout << "[DSP OBJECT BLOCK PREFLIGHT FAILURE]\n";

    const auto firstGainApply =
      applyDspParameterBindings(gainBinding, circuit);
    printSummary("[FIRST GAIN APPLY]", firstGainApply);

    const auto firstBiasApply =
      applyDspParameterBindings(biasBinding, circuit);
    printSummary("[FIRST BIAS APPLY]", firstBiasApply);

    const auto firstOutputBlock =
      processBlock(gain, bias, inputBlock);

    printBlock("first output block:", firstOutputBlock);

    const auto gainChanged =
      circuit.setParameterValue(1, "gain", 4.0f);
    const auto biasChanged =
      circuit.setParameterValue(2, "bias", 0.5f);

    std::cout << "set gain returned: "
              << (gainChanged ? "true" : "false")
              << "\n";
    std::cout << "set bias returned: "
              << (biasChanged ? "true" : "false")
              << "\n";

    const auto invalidBiasBinding =
      createBinding(
        2,
        "TinyBiasDsp",
        "Tiny Bias DSP",
        "missing_bias",
        "bias",
        biasMemory);

    const auto gainPreflight =
      preflightBinding(gainBinding, circuit);
    const auto biasPreflight =
      preflightBinding(invalidBiasBinding, circuit);
    const auto combinedPreflight =
      combinePreflight(gainPreflight, biasPreflight);

    printPreflight("[GAIN PREFLIGHT]", gainPreflight);
    printPreflight("[BIAS PREFLIGHT]", biasPreflight);
    printPreflight("[COMBINED PREFLIGHT]", combinedPreflight);

    if (!combinedPreflight.ok)
    {
        std::cout << "second block skipped: true\n";
    }
    else
    {
        const auto secondGainApply =
          applyDspParameterBindings(gainBinding, circuit);
        printSummary("[SECOND GAIN APPLY]", secondGainApply);

        const auto secondBiasApply =
          applyDspParameterBindings(invalidBiasBinding, circuit);
        printSummary("[SECOND BIAS APPLY]", secondBiasApply);

        const auto secondOutputBlock =
          processBlock(gain, bias, inputBlock);
        printBlock("second output block:", secondOutputBlock);
    }

    std::cout << "gainMemory after preflight: "
              << gainMemory
              << "\n";
    std::cout << "biasMemory after preflight: "
              << biasMemory
              << "\n";

    return 0;
}
