#include <soemdsp/runtime/dsp/PrintDspBinding.hpp>

using namespace soemdsp::runtime;

int main()
{
    DspObjectBinding binding;
    binding.nodeId     = 1;
    binding.objectType = "SplitMix64";
    binding.objectName = "Noise Source";

    binding.parameterBindings.push_back({
      1,
      "seed",
      DspBindingTargetKind::MemorySlot,
      "seed",
      1 });

    printDspObjectBinding(binding);

    return 0;
}
