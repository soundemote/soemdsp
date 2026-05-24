#include <soemdsp/runtime/dsp/PrintDspBinding.hpp>
#include <soemdsp/runtime/dsp/PrintDspBindingValidation.hpp>

using namespace soemdsp::runtime;

int main()
{
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

    return 0;
}
