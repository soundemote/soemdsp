#include <array>
#include <iostream>
#include <memory>

#include <soemdsp/runtime/dsp/ApplyDspBinding.hpp>
#include <soemdsp/runtime/dsp/DspBlockPhaseReport.hpp>
#include <soemdsp/runtime/dsp/PrintDspBindingApplySummary.hpp>
#include <soemdsp/runtime/dsp/PrintDspBlockPhaseReport.hpp>
#include <soemdsp/runtime/dsp/ValidateDspBinding.hpp>
#include <soemdsp/runtime/dsp/ValidateDspBindingTargets.hpp>
#include <soemdsp/runtime/dsp/WriteDspBlockPhaseReport.hpp>
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

void preflightBinding(
  DspBlockPhaseReport& report,
  const DspObjectBinding& binding,
  const Circuit& circuit)
{
    const auto structuralValidation =
      validateDspObjectBinding(binding);
    const auto targetValidation =
      validateDspObjectBindingTargets(binding, circuit);

    report.preflightOk =
      report.preflightOk &&
      structuralValidation.ok() &&
      targetValidation.ok();
    ++report.bindingsChecked;
    report.preflightMessages +=
      structuralValidation.messageCount() +
      targetValidation.messageCount();
}

void applyBinding(
  DspBlockPhaseReport& report,
  const DspObjectBinding& binding,
  const Circuit& circuit)
{
    const auto result =
      applyDspParameterBindings(binding, circuit);

    report.applyOk =
      report.applyOk && result.ok;
    report.parametersApplied +=
      result.parametersApplied;
    report.applyMessages +=
      result.messages.size();
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
  DspBlockPhaseReport& report,
  const TinyGainDsp& gain,
  const TinyBiasDsp& bias,
  const std::array<float, Size>& inputBlock)
{
    std::array<float, Size> outputBlock{};

    for (std::size_t i = 0; i < inputBlock.size(); ++i)
    {
        const auto afterGain = gain.processSample(inputBlock[i]);
        outputBlock[i] = bias.processSample(afterGain);
        ++report.samplesProcessed;
    }

    report.processOk = true;
    return outputBlock;
}

template <std::size_t Size>
std::array<float, Size> processBlock(
  const TinyGainDsp& gain,
  const TinyBiasDsp& bias,
  const std::array<float, Size>& inputBlock)
{
    DspBlockPhaseReport ignoredReport;
    return processBlock(ignoredReport, gain, bias, inputBlock);
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

    DspBlockPhaseReport secondPassReport;
    preflightBinding(secondPassReport, gainBinding, circuit);
    preflightBinding(secondPassReport, invalidBiasBinding, circuit);

    if (!secondPassReport.preflightOk)
    {
        std::cout << "second block skipped: true\n";
    }
    else
    {
        applyBinding(secondPassReport, gainBinding, circuit);
        applyBinding(secondPassReport, invalidBiasBinding, circuit);

        const auto secondOutputBlock =
          processBlock(secondPassReport, gain, bias, inputBlock);
        printBlock("second output block:", secondOutputBlock);
    }

    printDspBlockPhaseReport(secondPassReport);
    const auto wroteReport =
      writeDspBlockPhaseReportTextFile(
        secondPassReport,
        "runtime_dsp_object_block_preflight_failure_demo.txt");
    std::cout << "phase report file written: "
              << (wroteReport ? "true" : "false")
              << "\n";

    std::cout << "gainMemory after preflight: "
              << gainMemory
              << "\n";
    std::cout << "biasMemory after preflight: "
              << biasMemory
              << "\n";

    return 0;
}
