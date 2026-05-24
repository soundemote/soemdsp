#pragma once

#include <string>
#include <utility>
#include <vector>
#include <soemdsp/runtime/dsp/DspBinding.hpp>

namespace soemdsp::runtime
{

enum class DspBindingValidationSeverity
{
    Info,
    Warning,
    Error
};

struct DspBindingValidationMessage
{
    DspBindingValidationSeverity severity{};
    std::string message;
};

struct DspBindingValidationReport
{
    std::vector<DspBindingValidationMessage> messages;

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

inline void addDspBindingValidationMessage(
  DspBindingValidationReport& report,
  DspBindingValidationSeverity severity,
  std::string message)
{
    report.messages.push_back({ severity, std::move(message) });
}

inline DspBindingValidationReport validateDspObjectBinding(
  const DspObjectBinding& binding)
{
    DspBindingValidationReport report;

    if (binding.objectType.empty())
    {
        addDspBindingValidationMessage(
          report,
          DspBindingValidationSeverity::Error,
          "dsp object binding has an empty objectType");
    }

    if (binding.objectName.empty())
    {
        addDspBindingValidationMessage(
          report,
          DspBindingValidationSeverity::Warning,
          "dsp object binding has an empty objectName");
    }

    for (std::size_t i = 0; i < binding.parameterBindings.size(); ++i)
    {
        const auto& parameterBinding = binding.parameterBindings[i];

        if (parameterBinding.parameterId.empty())
        {
            addDspBindingValidationMessage(
              report,
              DspBindingValidationSeverity::Error,
              "dsp parameter binding has an empty parameterId");
        }

        if (parameterBinding.targetName.empty())
        {
            addDspBindingValidationMessage(
              report,
              DspBindingValidationSeverity::Warning,
              "dsp parameter binding for node " +
                std::to_string(parameterBinding.nodeId) +
                " param " +
                parameterBinding.parameterId +
                " has an empty targetName");
        }

        for (std::size_t j = i + 1; j < binding.parameterBindings.size(); ++j)
        {
            const auto& other = binding.parameterBindings[j];
            if (parameterBinding.nodeId == other.nodeId &&
                parameterBinding.parameterId == other.parameterId)
            {
                addDspBindingValidationMessage(
                  report,
                  DspBindingValidationSeverity::Error,
                  "duplicate dsp parameter binding for node " +
                    std::to_string(parameterBinding.nodeId) +
                    " param " +
                    parameterBinding.parameterId);
            }
        }
    }

    // TODO: validate against an actual DSP object descriptor/memory layout.
    return report;
}

} // namespace soemdsp::runtime
