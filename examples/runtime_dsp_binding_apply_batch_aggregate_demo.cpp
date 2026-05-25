#include <iostream>
#include <memory>
#include <vector>

#include <soemdsp/runtime/dsp/ApplyDspBinding.hpp>
#include <soemdsp/runtime/dsp/DspBindingApplySummary.hpp>
#include <soemdsp/soemdsp.hpp>

using namespace soemdsp::runtime;
using namespace soemdsp::runtime::nodes;

namespace
{

struct DemoBatchAggregate
{
    std::size_t totalBindings{};
    std::size_t successfulBindings{};
    std::size_t failedBindings{};
    std::size_t totalParametersApplied{};
    std::size_t totalMessages{};
};

std::unique_ptr<FloatConstant> createNodeWithParameter(
  NodeId nodeId,
  const char* parameterId,
  const char* name,
  float value)
{
    auto node = std::make_unique<FloatConstant>(0.0f);
    node->id = nodeId;
    node->name = name;

    Parameter parameter;
    parameter.id = parameterId;
    parameter.name = name;
    parameter.value = value;
    parameter.defaultValue = value;
    parameter.minValue = -4.0f;
    parameter.midValue = 0.0f;
    parameter.maxValue = 4.0f;
    node->parameters.push_back(parameter);

    return node;
}

Circuit createCircuit()
{
    Circuit circuit;

    circuit.nodes.push_back(
      createNodeWithParameter(1, "gain", "Gain", 2.0f));
    circuit.nodes.push_back(
      createNodeWithParameter(2, "bias", "Bias", 0.25f));
    circuit.prepare();

    return circuit;
}

DspObjectBinding createBinding(
  NodeId nodeId,
  const char* objectType,
  const char* objectName,
  const char* parameterId,
  const char* targetName,
  float* memorySlot)
{
    DspObjectBinding binding;
    binding.nodeId = nodeId;
    binding.objectType = objectType;
    binding.objectName = objectName;

    binding.parameterBindings.push_back({
      nodeId,
      parameterId,
      DspBindingTargetKind::MemorySlot,
      targetName,
      0,
      memorySlot });

    return binding;
}

DemoBatchAggregate aggregateSummaries(
  const std::vector<DspBindingApplySummary>& summaries)
{
    DemoBatchAggregate aggregate;
    aggregate.totalBindings = summaries.size();

    for (const auto& summary : summaries)
    {
        if (summary.ok)
        {
            ++aggregate.successfulBindings;
        }
        else
        {
            ++aggregate.failedBindings;
        }

        aggregate.totalParametersApplied += summary.parametersApplied;
        aggregate.totalMessages += summary.messageCount;
    }

    return aggregate;
}

void printAggregate(const DemoBatchAggregate& aggregate)
{
    std::cout << "total bindings: "
              << aggregate.totalBindings
              << "\n";
    std::cout << "successful bindings: "
              << aggregate.successfulBindings
              << "\n";
    std::cout << "failed bindings: "
              << aggregate.failedBindings
              << "\n";
    std::cout << "total parameters applied: "
              << aggregate.totalParametersApplied
              << "\n";
    std::cout << "total messages: "
              << aggregate.totalMessages
              << "\n";
}

} // namespace

int main()
{
    auto circuit = createCircuit();
    float gainMemory = 0.0f;
    float biasMemory = 0.0f;
    float missingMemory = 123.0f;

    const auto gainBinding =
      createBinding(1, "TinyGainDsp", "Tiny Gain DSP", "gain", "gain", &gainMemory);
    const auto biasBinding =
      createBinding(2, "TinyBiasDsp", "Tiny Bias DSP", "bias", "bias", &biasMemory);
    const auto missingBinding =
      createBinding(3, "MissingDsp", "Missing DSP", "missing", "missing", &missingMemory);

    const std::vector<DspBindingApplySummary> summaries{
      makeDspBindingApplySummary(
        applyDspParameterBindings(gainBinding, circuit)),
      makeDspBindingApplySummary(
        applyDspParameterBindings(biasBinding, circuit)),
      makeDspBindingApplySummary(
        applyDspParameterBindings(missingBinding, circuit))
    };

    const auto aggregate = aggregateSummaries(summaries);

    std::cout << "[DSP BINDING APPLY BATCH AGGREGATE DEMO]\n";
    printAggregate(aggregate);

    std::cout << "gainMemory after: "
              << gainMemory
              << "\n";
    std::cout << "biasMemory after: "
              << biasMemory
              << "\n";
    std::cout << "missingMemory after: "
              << missingMemory
              << "\n";

    return 0;
}
