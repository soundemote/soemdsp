#include <soemdsp/runtime/dsp/PrintDspBinding.hpp>
#include <memory>

#include <soemdsp/runtime/dsp/PrintDspBindingTargetValidation.hpp>
#include <soemdsp/runtime/dsp/PrintDspBindingValidation.hpp>
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

    Parameter other;
    other.id = "other";
    other.name = "Other";
    node->parameters.push_back(other);

    circuit.nodes.push_back(std::move(node));
    circuit.prepare();

    return circuit;
}

} // namespace

int main()
{
    const auto circuit = createCircuit();
    DspObjectBinding binding;
    binding.nodeId = 1;

    binding.parameterBindings.push_back({
      1,
      "seed",
      DspBindingTargetKind::MemorySlot,
      "seed",
      1 });

    binding.parameterBindings.push_back({
      1,
      "seed",
      DspBindingTargetKind::MemorySlot,
      "seed_duplicate",
      2 });

    binding.parameterBindings.push_back({
      1,
      "",
      DspBindingTargetKind::SemanticSetter,
      "",
      0 });

    printDspObjectBinding(binding);
    printDspBindingValidation(validateDspObjectBinding(binding));
    printDspBindingTargetValidation(
      validateDspObjectBindingTargets(binding, circuit));

    return 0;
}
