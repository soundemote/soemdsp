#include <iostream>
#include <memory>

#include <soemdsp/runtime/dsp/ApplyDspBinding.hpp>
#include <soemdsp/runtime/dsp/PrintDspBindingApplySummary.hpp>
#include <soemdsp/runtime/dsp/WriteDspBindingApplySummary.hpp>
#include <soemdsp/soemdsp.hpp>

using namespace soemdsp::runtime;
using namespace soemdsp::runtime::nodes;

namespace
{

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

    circuit.nodes.push_back(std::move(node));
    circuit.prepare();

    return circuit;
}

DspObjectBinding createBinding(
  std::string parameterId,
  float* memorySlot)
{
    DspObjectBinding binding;
    binding.nodeId = 1;
    binding.objectType = "TinyGainDsp";
    binding.objectName = "Tiny Gain DSP";

    binding.parameterBindings.push_back({
      1,
      std::move(parameterId),
      DspBindingTargetKind::MemorySlot,
      "gain",
      0,
      memorySlot });

    return binding;
}

void printAndWriteSummary(
  const char* label,
  const DspBindingApplyResult& result,
  const char* path)
{
    const auto summary = makeDspBindingApplySummary(result);

    std::cout << label
              << "\n";
    printDspBindingApplySummary(summary);

    const auto wroteSummary =
      writeDspBindingApplySummaryTextFile(summary, path);
    std::cout << "summary file written: "
              << (wroteSummary ? "true" : "false")
              << "\n";
}

} // namespace

int main()
{
    auto circuit = createCircuit();

    float okMemory = 0.0f;
    const auto okBinding = createBinding("gain", &okMemory);
    const auto okResult =
      applyDspParameterBindings(okBinding, circuit);
    printAndWriteSummary(
      "[APPLY SUMMARY OK]",
      okResult,
      "runtime_dsp_binding_apply_summary_demo.ok.txt");

    float failMemory = 77.0f;
    const auto failBinding = createBinding("missing_gain", &failMemory);
    const auto failResult =
      applyDspParameterBindings(failBinding, circuit);
    printAndWriteSummary(
      "[APPLY SUMMARY FAIL]",
      failResult,
      "runtime_dsp_binding_apply_summary_demo.fail.txt");

    return 0;
}
