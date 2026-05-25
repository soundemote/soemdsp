#include <iostream>
#include <memory>

#include <soemdsp/runtime/dsp/ApplyDspBinding.hpp>
#include <soemdsp/runtime/dsp/PrintDspBindingApplyResult.hpp>
#include <soemdsp/runtime/dsp/PrintDspBindingReport.hpp>
#include <soemdsp/runtime/dsp/WriteDspBindingApplyResult.hpp>
#include <soemdsp/soemdsp.hpp>

using namespace soemdsp::runtime;
using namespace soemdsp::runtime::nodes;

namespace
{

struct FakeDspState
{
    float cutoff{ 0.0f };
};

Circuit createCircuit()
{
    Circuit circuit;

    auto node = std::make_unique<FloatConstant>(0.0f);
    node->id = 1;

    Parameter cutoff;
    cutoff.id = "cutoff";
    cutoff.name = "Cutoff";
    cutoff.value = 1234.0f;
    cutoff.defaultValue = 1000.0f;
    cutoff.minValue = 20.0f;
    cutoff.midValue = 1000.0f;
    cutoff.maxValue = 20000.0f;
    node->parameters.push_back(cutoff);

    circuit.nodes.push_back(std::move(node));
    circuit.prepare();

    return circuit;
}

DspObjectBinding createBinding(FakeDspState& state)
{
    DspObjectBinding binding;
    binding.nodeId = 1;
    binding.objectType = "FakeFilter";
    binding.objectName = "External Filter State";

    binding.parameterBindings.push_back({
      1,
      "cutoff",
      DspBindingTargetKind::MemorySlot,
      "cutoff",
      0,
      &state.cutoff });

    return binding;
}

} // namespace

int main()
{
    auto circuit = createCircuit();
    FakeDspState state;
    auto binding = createBinding(state);

    std::cout << "fake dsp cutoff before: "
              << state.cutoff
              << "\n";

    const auto report = makeDspBindingReport(binding, circuit);
    printDspBindingReport(report);

    const auto applyResult =
      applyDspParameterBindings(binding, circuit);
    printDspBindingApplyResult(applyResult);

    const auto wroteApplyResult =
      writeDspBindingApplyResultTextFile(
        applyResult,
        "runtime_dsp_binding_apply_demo.dsp_binding_apply_result.txt");
    std::cout << "apply result file written: "
              << (wroteApplyResult ? "true" : "false")
              << "\n";

    std::cout << "fake dsp cutoff after: "
              << state.cutoff
              << "\n";

    return 0;
}
