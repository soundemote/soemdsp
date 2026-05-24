#pragma once

#include <soemdsp/runtime/Circuit.hpp>
#include <soemdsp/runtime/serialization/CircuitSnapshot.hpp>
#include <soemdsp/runtime/validation/CircuitValidation.hpp>
#include <soemdsp/runtime/validation/ValidationSummary.hpp>

namespace soemdsp::runtime
{

struct CircuitReport
{
    CircuitSnapshot snapshot;
    CircuitValidationReport validation;
    ValidationSummary validationSummary;
};

inline CircuitReport makeCircuitReport(const Circuit& circuit)
{
    CircuitReport report;
    report.snapshot           = circuit.snapshot();
    report.validation         = validateCircuit(circuit);
    report.validationSummary  = validationSummary(report.validation);
    return report;
}

} // namespace soemdsp::runtime
