#pragma once

#include <soemdsp/runtime/Circuit.hpp>
#include <soemdsp/runtime/dsp/DspBinding.hpp>
#include <soemdsp/runtime/dsp/ValidateDspBinding.hpp>
#include <soemdsp/runtime/dsp/ValidateDspBindingTargets.hpp>

namespace soemdsp::runtime
{

struct DspBindingReport
{
    DspObjectBinding binding;
    DspBindingValidationReport structuralValidation;
    DspBindingTargetValidationReport targetValidation;
};

inline DspBindingReport makeDspBindingReport(
  const DspObjectBinding& binding,
  const Circuit& circuit)
{
    DspBindingReport report;
    report.binding = binding;
    report.structuralValidation = validateDspObjectBinding(binding);
    report.targetValidation = validateDspObjectBindingTargets(binding, circuit);
    return report;
}

} // namespace soemdsp::runtime
