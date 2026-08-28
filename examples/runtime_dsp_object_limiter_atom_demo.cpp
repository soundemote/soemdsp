#include <iostream>
#include <memory>

#include <soemdsp/dynamics/LookaheadLimiterAtom.hpp>
#include <soemdsp/runtime/dsp/ApplyDspBinding.hpp>
#include <soemdsp/soemdsp.hpp>

using namespace soemdsp::runtime;
using namespace soemdsp::runtime::nodes;
using soemdsp::dynamics::atom::LookaheadLimiter;

namespace
{

Circuit createCircuit()
{
    Circuit circuit;
    auto node = std::make_unique<FloatConstant>(0.0f);
    node->id = 1;

    auto addParam = [&](const char* id, const char* name, float value, float minV, float maxV) {
        Parameter p;
        p.id = id;
        p.name = name;
        p.value = value;
        p.defaultValue = value;
        p.minValue = minV;
        p.midValue = value;
        p.maxValue = maxV;
        node->parameters.push_back(p);
    };

    addParam("ceilingDb", "Ceiling", -0.3f, -24.0f, 0.0f);
    addParam("attackMs", "Attack", 1.0f, 0.0f, 100.0f);
    addParam("releaseMs", "Release", 100.0f, 1.0f, 1000.0f);
    addParam("sampleRate", "Sample Rate", 48000.0f, 8000.0f, 192000.0f);
    addParam("dipGain", "Dip", 1.0f, 0.5f, 4.0f);

    circuit.nodes.push_back(std::move(node));
    circuit.prepare();
    return circuit;
}

} // namespace

int main()
{
    auto circuit = createCircuit();
    double mem[5]{};
    float mirrors[5]{};

    LookaheadLimiter lim{};
    lim.mem = mem;
    lim.base = 0;
    lim.reset();

    DspObjectBinding binding;
    binding.nodeId = 1;
    binding.objectType = "LookaheadLimiterAtom";
    binding.objectName = "Lookahead Limiter Atom";
    const char* ids[] = { "ceilingDb", "attackMs", "releaseMs", "sampleRate", "dipGain" };
    for (int i = 0; i < 5; ++i)
    {
        binding.parameterBindings.push_back({
          1, ids[i], DspBindingTargetKind::MemorySlot, ids[i],
          static_cast<std::uint32_t>(i), &mirrors[i] });
    }

    std::cout << "[DSP OBJECT LIMITER ATOM]\n";
    auto apply = applyDspParameterBindings(binding, circuit);
    std::cout << "apply ok: " << (apply.ok ? "true" : "false") << "\n";
    for (int i = 0; i < 5; ++i)
    {
        mem[i] = static_cast<double>(mirrors[i]);
    }
    lim.syncControlParams();
    const double att1 = lim.attCoeff_;
    const double out1 = lim.process(0.9, 0.9, 0, false, false);
    lim.syncControlParams(); // unchanged controls
    const double att2 = lim.attCoeff_;
    std::cout << "attCoeff stable on re-sync: " << (att1 == att2 ? "true" : "false") << "\n";
    std::cout << "process out: " << out1 << "\n";

    circuit.setParameterValue(1, "attackMs", 10.0f);
    apply = applyDspParameterBindings(binding, circuit);
    for (int i = 0; i < 5; ++i)
    {
        mem[i] = static_cast<double>(mirrors[i]);
    }
    lim.syncControlParams();
    std::cout << "attCoeff changed after attackMs: "
              << (lim.attCoeff_ != att1 ? "true" : "false")
              << "\n";
    std::cout << "attCoeff now: " << lim.attCoeff_ << "\n";

    return apply.ok ? 0 : 1;
}
