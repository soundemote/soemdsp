#include <cstdint>
#include <iostream>
#include <memory>

#include <soemdsp/random/NoiseAtoms.hpp>
#include <soemdsp/runtime/dsp/ApplyDspBinding.hpp>
#include <soemdsp/runtime/dsp/DspBindingReport.hpp>
#include <soemdsp/runtime/dsp/PrintDspBindingReport.hpp>
#include <soemdsp/runtime/dsp/WriteDspBindingReport.hpp>
#include <soemdsp/soemdsp.hpp>

using namespace soemdsp::runtime;
using namespace soemdsp::runtime::nodes;
using soemdsp::random::atom::SplitMix64;

namespace
{

Circuit createCircuit(float seedValue)
{
    Circuit circuit;

    auto node = std::make_unique<FloatConstant>(0.0f);
    node->id = 1;

    Parameter seed;
    seed.id = "seed";
    seed.name = "Seed";
    seed.value = seedValue;
    seed.defaultValue = 1.0f;
    seed.minValue = 1.0f;
    seed.midValue = 1000.0f;
    seed.maxValue = 1.0e9f;
    node->parameters.push_back(seed);

    circuit.nodes.push_back(std::move(node));
    circuit.prepare();
    return circuit;
}

// float MemorySlot mirror for binding metadata demos; uint64 seed is written by caller.
DspObjectBinding createBinding(float& seedMirror)
{
    DspObjectBinding binding;
    binding.nodeId = 1;
    binding.objectType = "SplitMix64";
    binding.objectName = "Noise Atom";
    binding.parameterBindings.push_back({
      1,
      "seed",
      DspBindingTargetKind::MemorySlot,
      "seed_mirror",
      0,
      &seedMirror });
    return binding;
}

void applySeedToAtom(SplitMix64& rng, float seedMirror)
{
    const auto seed = static_cast<std::uint64_t>(seedMirror);
    rng.seed_() = seed == 0 ? 1ULL : seed;
    rng.syncSeed();
}

} // namespace

int main()
{
    auto circuit = createCircuit(42.0f);
    float seedMirror = 0.0f;
    std::uint64_t mem[2]{};
    SplitMix64 rng{};
    rng.mem = mem;
    rng.base = 0;

    const auto binding = createBinding(seedMirror);
    const auto report = makeDspBindingReport(binding, circuit);

    std::cout << "[DSP OBJECT NOISE ATOM]\n";
    printDspBindingReport(report);
    writeDspBindingReportTextFile(
      report,
      "runtime_dsp_object_noise_atom_demo.dsp_binding_report.txt");

    // Binding writes the float mirror (existing apply path).
    const auto applyResult = applyDspParameterBindings(binding, circuit);
    std::cout << "binding apply ok: " << (applyResult.ok ? "true" : "false") << "\n";
    std::cout << "seed mirror after apply: " << seedMirror << "\n";

    // Caller maps float Circuit value into uint64 mem slot, then selective reset.
    applySeedToAtom(rng, seedMirror);
    std::cout << "atom seed slot: " << rng.seed_() << "\n";
    std::cout << "atom state slot: " << rng.state_() << "\n";

    const double a = rng.nextBipolar();
    const double b = rng.nextBipolar();
    std::cout << "nextBipolar #1: " << a << "\n";
    std::cout << "nextBipolar #2: " << b << "\n";

    // Same seed again → syncSeed should not reset (state keeps advancing).
    applySeedToAtom(rng, seedMirror);
    const double c = rng.nextBipolar();
    std::cout << "nextBipolar #3 (no re-seed): " << c << "\n";

    // Changed Circuit seed → write + syncSeed resets state.
    circuit.setParameterValue(1, "seed", 99.0f);
    const auto apply2 = applyDspParameterBindings(binding, circuit);
    std::cout << "rebind ok: " << (apply2.ok ? "true" : "false") << "\n";
    applySeedToAtom(rng, seedMirror);
    std::cout << "reseeding seed slot: " << rng.seed_() << "\n";
    std::cout << "nextBipolar after reseed: " << rng.nextBipolar() << "\n";

    return applyResult.ok && apply2.ok ? 0 : 1;
}
