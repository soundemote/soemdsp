#pragma once

#include <cstddef>
#include <soemdsp/runtime/validation/ValidationGate.hpp>

namespace soemdsp::runtime
{

struct ValidationSummary
{
    std::size_t infoCount{};
    std::size_t warningCount{};
    std::size_t errorCount{};
    std::size_t messageCount{};
    ValidationGateResult gate{};
    bool allowsExecution{};
};

inline ValidationSummary validationSummary(
  const CircuitValidationReport& report)
{
    ValidationSummary summary;
    summary.infoCount       = report.infoCount();
    summary.warningCount    = report.warningCount();
    summary.errorCount      = report.errorCount();
    summary.messageCount    = report.messageCount();
    summary.gate            = validationGateResult(report);
    summary.allowsExecution = validationGateAllowsExecution(report);
    return summary;
}

} // namespace soemdsp::runtime
