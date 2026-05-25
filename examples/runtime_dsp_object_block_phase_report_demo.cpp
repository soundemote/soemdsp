#include <array>
#include <iostream>
#include <memory>

#include <soemdsp/runtime/dsp/ApplyDspBinding.hpp>
#include <soemdsp/runtime/dsp/DspBlockPhaseReport.hpp>
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
      createNodeWithParameter(1, "gain", "Gain", 4.0f));
    circuit.nodes.push_back(
      createNodeWithParameter(2, "bias", "Bias", 0.5f));
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
    const auto applyResult =
      applyDspParameterBindings(binding, circuit);

    report.applyOk =
      report.applyOk && applyResult.ok;
    report.parametersApplied +=
      applyResult.parametersApplied;
    report.applyMessages +=
      applyResult.messages.size();
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

    DspBlockPhaseReport report;

    std::cout << "[DSP OBJECT BLOCK PHASE REPORT]\n";

    preflightBinding(report, gainBinding, circuit);
    preflightBinding(report, biasBinding, circuit);

    if (report.preflightOk)
    {
        applyBinding(report, gainBinding, circuit);
        applyBinding(report, biasBinding, circuit);
    }

    std::array<float, inputBlock.size()> outputBlock{};
    if (report.preflightOk && report.applyOk)
    {
        outputBlock =
          processBlock(report, gain, bias, inputBlock);
    }

    printDspBlockPhaseReport(report);
    const auto wroteReport =
      writeDspBlockPhaseReportTextFile(
        report,
        "runtime_dsp_object_block_phase_report_demo.txt");
    std::cout << "phase report file written: "
              << (wroteReport ? "true" : "false")
              << "\n";
    std::cout << "gainMemory: "
              << gainMemory
              << "\n";
    std::cout << "biasMemory: "
              << biasMemory
              << "\n";
    printBlock("input block:", inputBlock);
    printBlock("output block:", outputBlock);

    return 0;
}
