#pragma once

#include <string>
#include <vector>
#include <soemdsp/runtime/Circuit.hpp>
#include <soemdsp/runtime/dsp/DspBinding.hpp>
#include <soemdsp/runtime/dsp/ValidateDspBinding.hpp>
#include <soemdsp/runtime/dsp/ValidateDspBindingTargets.hpp>

namespace soemdsp::runtime
{

struct DspBindingApplyResult
{
    bool ok{ true };
    std::vector<std::string> messages;
    std::size_t parametersApplied{ 0 };
};

inline void appendDspBindingValidationMessages(
  DspBindingApplyResult& result,
  const DspBindingValidationReport& report)
{
    for (const auto& message : report.messages)
    {
        result.messages.push_back(message.message);
    }
}

inline void appendDspBindingTargetValidationMessages(
  DspBindingApplyResult& result,
  const DspBindingTargetValidationReport& report)
{
    for (const auto& message : report.messages)
    {
        result.messages.push_back(message.message);
    }
}

inline DspBindingApplyResult applyDspParameterBindings(
  const DspObjectBinding& binding,
  const Circuit& circuit)
{
    DspBindingApplyResult result;

    const auto structuralValidation =
      validateDspObjectBinding(binding);
    const auto targetValidation =
      validateDspObjectBindingTargets(binding, circuit);

    if (!structuralValidation.ok() || !targetValidation.ok())
    {
        result.ok = false;
        appendDspBindingValidationMessages(result, structuralValidation);
        appendDspBindingTargetValidationMessages(result, targetValidation);
        return result;
    }

    for (const auto& parameterBinding : binding.parameterBindings)
    {
        const auto* parameter =
          circuit.findParameter(
            parameterBinding.nodeId,
            parameterBinding.parameterId);
        if (parameter == nullptr)
        {
            result.ok = false;
            result.messages.push_back(
              "missing circuit parameter for dsp binding");
            return result;
        }

        if (parameterBinding.targetKind != DspBindingTargetKind::MemorySlot)
        {
            result.ok = false;
            result.messages.push_back(
              "unsupported dsp binding target kind for apply");
            return result;
        }

        if (parameterBinding.memorySlot == nullptr)
        {
            result.ok = false;
            result.messages.push_back(
              "dsp binding memory slot is null");
            return result;
        }

        *parameterBinding.memorySlot = parameter->value;
        ++result.parametersApplied;
    }

    return result;
}

} // namespace soemdsp::runtime
