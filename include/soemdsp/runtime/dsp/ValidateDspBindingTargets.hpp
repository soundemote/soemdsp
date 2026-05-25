#pragma once

#include <string>
#include <utility>
#include <vector>
#include <soemdsp/runtime/Circuit.hpp>
#include <soemdsp/runtime/dsp/ValidateDspBinding.hpp>

namespace soemdsp::runtime
{

struct DspBindingTargetValidationMessage
{
    DspBindingValidationSeverity severity{};
    std::string message;
};

struct DspBindingTargetValidationReport
{
    std::vector<DspBindingTargetValidationMessage> messages;

    bool hasErrors() const
    {
        for (const auto& message : messages)
        {
            if (message.severity == DspBindingValidationSeverity::Error)
            {
                return true;
            }
        }

        return false;
    }

    bool hasWarnings() const
    {
        for (const auto& message : messages)
        {
            if (message.severity == DspBindingValidationSeverity::Warning)
            {
                return true;
            }
        }

        return false;
    }

    bool ok() const
    {
        return messages.empty();
    }

    std::size_t infoCount() const
    {
        std::size_t count = 0;

        for (const auto& message : messages)
        {
            if (message.severity == DspBindingValidationSeverity::Info)
            {
                ++count;
            }
        }

        return count;
    }

    std::size_t warningCount() const
    {
        std::size_t count = 0;

        for (const auto& message : messages)
        {
            if (message.severity == DspBindingValidationSeverity::Warning)
            {
                ++count;
            }
        }

        return count;
    }

    std::size_t errorCount() const
    {
        std::size_t count = 0;

        for (const auto& message : messages)
        {
            if (message.severity == DspBindingValidationSeverity::Error)
            {
                ++count;
            }
        }

        return count;
    }

    std::size_t messageCount() const
    {
        return messages.size();
    }
};

inline void addDspBindingTargetValidationMessage(
  DspBindingTargetValidationReport& report,
  DspBindingValidationSeverity severity,
  std::string message)
{
    report.messages.push_back({ severity, std::move(message) });
}

inline DspBindingTargetValidationReport validateDspObjectBindingTargets(
  const DspObjectBinding& binding,
  const Circuit& circuit)
{
    DspBindingTargetValidationReport report;

    for (const auto& parameterBinding : binding.parameterBindings)
    {
        if (parameterBinding.parameterId.empty())
        {
            continue;
        }

        if (circuit.findParameter(
              parameterBinding.nodeId,
              parameterBinding.parameterId) == nullptr)
        {
            addDspBindingTargetValidationMessage(
              report,
              DspBindingValidationSeverity::Error,
              "dsp parameter binding points to missing circuit parameter node " +
                std::to_string(parameterBinding.nodeId) +
                " param " +
                parameterBinding.parameterId);
        }

        if (parameterBinding.targetKind != DspBindingTargetKind::MemorySlot)
        {
            addDspBindingTargetValidationMessage(
              report,
              DspBindingValidationSeverity::Error,
              "dsp parameter binding for node " +
                std::to_string(parameterBinding.nodeId) +
                " param " +
                parameterBinding.parameterId +
                " uses unsupported target kind");
            continue;
        }

        if (parameterBinding.memorySlot == nullptr)
        {
            addDspBindingTargetValidationMessage(
              report,
              DspBindingValidationSeverity::Error,
              "dsp parameter binding for node " +
                std::to_string(parameterBinding.nodeId) +
                " param " +
                parameterBinding.parameterId +
                " has null memory slot");
        }
    }

    return report;
}

} // namespace soemdsp::runtime
