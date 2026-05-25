#include <iostream>
#include <memory>

#include <soemdsp/runtime/dsp/ApplyDspBinding.hpp>
#include <soemdsp/runtime/dsp/PrintDspBindingApplyResult.hpp>
#include <soemdsp/runtime/dsp/PrintDspBindingReport.hpp>
#include <soemdsp/runtime/dsp/WriteDspBindingApplyResult.hpp>
#include <soemdsp/runtime/dsp/WriteDspBindingReport.hpp>
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

DspObjectBinding createBinding(float& gainMemory)
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

} // namespace

int main()
{
    auto circuit = createCircuit();
    float gainMemory = 0.0f;
    TinyGainDsp dsp{ &gainMemory };
    const auto binding = createBinding(gainMemory);

    std::cout << "[DSP OBJECT PARAMETER SYNC]\n";
    std::cout << "gain memory before: "
              << gainMemory
              << "\n";

    const auto report = makeDspBindingReport(binding, circuit);
    printDspBindingReport(report);

    const auto wroteReport =
      writeDspBindingReportTextFile(
        report,
        "runtime_dsp_object_parameter_sync_demo.dsp_binding_report.txt");
    std::cout << "binding report file written: "
              << (wroteReport ? "true" : "false")
              << "\n";

    const auto applyResult =
      applyDspParameterBindings(binding, circuit);
    printDspBindingApplyResult(applyResult);

    const auto wroteApplyResult =
      writeDspBindingApplyResultTextFile(
        applyResult,
        "runtime_dsp_object_parameter_sync_demo.dsp_binding_apply_result.txt");
    std::cout << "apply result file written: "
              << (wroteApplyResult ? "true" : "false")
              << "\n";

    std::cout << "gain memory after: "
              << gainMemory
              << "\n";
    std::cout << "processSample(0.25): "
              << dsp.processSample(0.25f)
              << "\n";

    return 0;
}
