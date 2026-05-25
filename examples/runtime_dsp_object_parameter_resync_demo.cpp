#include <iostream>
#include <memory>

#include <soemdsp/runtime/dsp/ApplyDspBinding.hpp>
#include <soemdsp/runtime/dsp/PrintDspBindingApplyResult.hpp>
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

void printSyncState(
  const char* label,
  const TinyGainDsp& dsp,
  float gainMemory)
{
    std::cout << label
              << " gain memory: "
              << gainMemory
              << "\n";
    std::cout << label
              << " processSample(0.25): "
              << dsp.processSample(0.25f)
              << "\n";
}

} // namespace

int main()
{
    auto circuit = createCircuit();
    float gainMemory = 0.0f;
    TinyGainDsp dsp{ &gainMemory };
    const auto binding = createBinding(gainMemory);

    std::cout << "[DSP OBJECT PARAMETER RESYNC]\n";
    std::cout << "initial gain memory: "
              << gainMemory
              << "\n";

    const auto firstApply =
      applyDspParameterBindings(binding, circuit);
    printDspBindingApplyResult(firstApply);
    printSyncState("after first apply", dsp, gainMemory);

    const auto changed =
      circuit.setParameterValue(1, "gain", 4.0f);
    std::cout << "set circuit gain to 4.0: "
              << (changed ? "true" : "false")
              << "\n";

    const auto secondApply =
      applyDspParameterBindings(binding, circuit);
    printDspBindingApplyResult(secondApply);
    printSyncState("after second apply", dsp, gainMemory);

    return 0;
}
