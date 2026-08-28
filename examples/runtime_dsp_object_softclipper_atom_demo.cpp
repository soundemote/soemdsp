#include <iostream>
#include <memory>

#include <soemdsp/dynamics/SoftClipperAtom.hpp>
#include <soemdsp/runtime/dsp/ApplyDspBinding.hpp>
#include <soemdsp/runtime/dsp/PrintDspBindingApplyResult.hpp>
#include <soemdsp/soemdsp.hpp>

using namespace soemdsp::runtime;
using namespace soemdsp::runtime::nodes;
using soemdsp::dynamics::atom::SoftClipper;

namespace
{

Circuit createCircuit()
{
    Circuit circuit;

    auto node = std::make_unique<FloatConstant>(0.0f);
    node->id = 1;

    Parameter center;
    center.id = "center";
    center.name = "Center";
    center.value = 0.0f;
    center.defaultValue = 0.0f;
    center.minValue = -1.0f;
    center.midValue = 0.0f;
    center.maxValue = 1.0f;
    node->parameters.push_back(center);

    Parameter width;
    width.id = "width";
    width.name = "Width";
    width.value = 2.0f;
    width.defaultValue = 2.0f;
    width.minValue = 0.1f;
    width.midValue = 2.0f;
    width.maxValue = 4.0f;
    node->parameters.push_back(width);

    circuit.nodes.push_back(std::move(node));
    circuit.prepare();
    return circuit;
}

DspObjectBinding createBinding(float& centerSlot, float& widthSlot)
{
    DspObjectBinding binding;
    binding.nodeId = 1;
    binding.objectType = "SoftClipperAtom";
    binding.objectName = "Soft Clipper Atom";
    binding.parameterBindings.push_back({
      1, "center", DspBindingTargetKind::MemorySlot, "center", 0, &centerSlot });
    binding.parameterBindings.push_back({
      1, "width", DspBindingTargetKind::MemorySlot, "width", 1, &widthSlot });
    return binding;
}

void copyFloatSlotsToAtomMem(SoftClipper& dsp, float center, float width)
{
    dsp.center_() = static_cast<double>(center);
    dsp.width_() = static_cast<double>(width);
    dsp.syncControlParams();
}

} // namespace

int main()
{
    auto circuit = createCircuit();
    float centerSlot = 0.0f;
    float widthSlot = 0.0f;
    double mem[2]{ 0.0, 2.0 };

    SoftClipper dsp{};
    dsp.mem = mem;
    dsp.base = 0;
    dsp.syncControlParams();

    const auto binding = createBinding(centerSlot, widthSlot);

    std::cout << "[DSP OBJECT SOFTCLIPPER ATOM]\n";

    auto apply = applyDspParameterBindings(binding, circuit);
    printDspBindingApplyResult(apply);
    copyFloatSlotsToAtomMem(dsp, centerSlot, widthSlot);

    const double before = dsp.process(3.0);
    std::cout << "process(3.0) width=2: " << before << "\n";
    std::cout << "scaleX cached: " << dsp.scaleX_ << "\n";

    // Change width once; sync should rebuild coeffs once.
    circuit.setParameterValue(1, "width", 0.5f);
    apply = applyDspParameterBindings(binding, circuit);
    copyFloatSlotsToAtomMem(dsp, centerSlot, widthSlot);
    const double afterNarrow = dsp.process(3.0);
    std::cout << "process(3.0) width=0.5: " << afterNarrow << "\n";
    std::cout << "scaleX cached: " << dsp.scaleX_ << "\n";

    // Same params again: syncControlParams is a no-op for coeffs.
    const double scaleBefore = dsp.scaleX_;
    copyFloatSlotsToAtomMem(dsp, centerSlot, widthSlot);
    std::cout << "scaleX unchanged on re-sync: "
              << (scaleBefore == dsp.scaleX_ ? "true" : "false")
              << "\n";

    return apply.ok ? 0 : 1;
}
