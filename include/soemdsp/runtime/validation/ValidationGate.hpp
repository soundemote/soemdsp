#pragma once

#include <soemdsp/runtime/validation/CircuitValidation.hpp>

namespace soemdsp::runtime
{

enum class ValidationGateResult
{
    Pass,
    Warning,
    Fail
};

inline ValidationGateResult validationGateResult(
  const CircuitValidationReport& report)
{
    if (report.hasErrors())
    {
        return ValidationGateResult::Fail;
    }

    if (report.hasWarnings())
    {
        return ValidationGateResult::Warning;
    }

    return ValidationGateResult::Pass;
}

inline const char* toString(ValidationGateResult result)
{
    switch (result)
    {
        case ValidationGateResult::Pass:
            return "pass";
        case ValidationGateResult::Warning:
            return "warning";
        case ValidationGateResult::Fail:
            return "fail";
    }

    return "fail";
}

inline bool validationGateAllowsExecution(
  const CircuitValidationReport& report)
{
    return validationGateResult(report) != ValidationGateResult::Fail;
}

} // namespace soemdsp::runtime
