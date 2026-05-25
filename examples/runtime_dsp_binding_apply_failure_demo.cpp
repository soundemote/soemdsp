#include <iostream>
#include <memory>

#include <soemdsp/runtime/dsp/ApplyDspBinding.hpp>
#include <soemdsp/runtime/dsp/PrintDspBindingApplyResult.hpp>
#include <soemdsp/soemdsp.hpp>

using namespace soemdsp::runtime;
using namespace soemdsp::runtime::nodes;

namespace
{

Circuit createCircuitWithParameter()
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

DspObjectBinding createBinding(
  std::string parameterId,
  float* memorySlot)
{
    DspObjectBinding binding;
    binding.nodeId = 1;
    binding.objectType = "FakeFilter";
    binding.objectName = "External Filter State";

    binding.parameterBindings.push_back({
      1,
      std::move(parameterId),
      DspBindingTargetKind::MemorySlot,
      "cutoff",
      0,
      memorySlot });

    return binding;
}

void printMemoryValue(const char* label, float value)
{
    std::cout << label
              << ": "
              << value
              << "\n";
}

} // namespace

int main()
{
    {
        std::cout << "[MISSING CIRCUIT PARAMETER TARGET]\n";

        auto circuit = createCircuitWithParameter();
        float externalCutoff = 77.0f;
        const auto binding = createBinding("missing_cutoff", &externalCutoff);

        printMemoryValue("external memory before", externalCutoff);
        const auto result = applyDspParameterBindings(binding, circuit);
        printDspBindingApplyResult(result);
        printMemoryValue("external memory after", externalCutoff);
        std::cout << "\n";
    }

    {
        std::cout << "[NULL DSP MEMORY SLOT]\n";

        auto circuit = createCircuitWithParameter();
        const auto binding = createBinding("cutoff", nullptr);

        const auto result = applyDspParameterBindings(binding, circuit);
        printDspBindingApplyResult(result);
    }

    return 0;
}
