#include <iostream>
#include <memory>

#include <soemdsp/runtime/dsp/ApplyDspBinding.hpp>
#include <soemdsp/runtime/dsp/PrintDspBindingApplyResult.hpp>
#include <soemdsp/runtime/dsp/PrintDspBindingReport.hpp>
#include <soemdsp/soemdsp.hpp>

using namespace soemdsp::runtime;
using namespace soemdsp::runtime::nodes;

namespace
{

struct TinyModulatedGainDsp
{
    float* gain{};
    float* modulation{};

    float processSample(float x) const noexcept
    {
        const float g = gain != nullptr ? *gain : 1.0f;
        const float m = modulation != nullptr ? *modulation : 0.0f;
        return x * (g + m);
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

DspObjectBinding createBinding(
  float& gainMemory,
  float& modulationMemory)
{
    DspObjectBinding binding;
    binding.nodeId = 1;
    binding.objectType = "TinyModulatedGainDsp";
    binding.objectName = "Tiny Modulated Gain DSP";

    binding.parameterBindings.push_back({
      1,
      "gain",
      DspBindingTargetKind::MemorySlot,
      "gain",
      0,
      &gainMemory });

    binding.parameterBindings.push_back({
      2,
      "modulation",
      DspBindingTargetKind::MemorySlot,
      "modulation",
      1,
      &modulationMemory });

    return binding;
}

} // namespace

int main()
{
    auto circuit = createCircuit();
    float gainMemory = 77.0f;
    float modulationMemory = 88.0f;
    TinyModulatedGainDsp dsp{ &gainMemory, &modulationMemory };
    const auto binding = createBinding(gainMemory, modulationMemory);

    std::cout << "[DSP OBJECT MULTI NODE FAILURE]\n";
    std::cout << "gain memory before: "
              << gainMemory
              << "\n";
    std::cout << "modulation memory before: "
              << modulationMemory
              << "\n";
    std::cout << "processSample(0.25) before: "
              << dsp.processSample(0.25f)
              << "\n";

    const auto report = makeDspBindingReport(binding, circuit);
    printDspBindingReport(report);

    const auto applyResult =
      applyDspParameterBindings(binding, circuit);
    printDspBindingApplyResult(applyResult);

    std::cout << "gain memory after: "
              << gainMemory
              << "\n";
    std::cout << "modulation memory after: "
              << modulationMemory
              << "\n";
    std::cout << "processSample(0.25) after: "
              << dsp.processSample(0.25f)
              << "\n";

    return 0;
}
