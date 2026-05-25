#pragma once

#include <iostream>
#include <ostream>
#include <soemdsp/runtime/dsp/DspBindingReport.hpp>
#include <soemdsp/runtime/dsp/PrintDspBinding.hpp>
#include <soemdsp/runtime/dsp/PrintDspBindingTargetValidation.hpp>
#include <soemdsp/runtime/dsp/PrintDspBindingValidation.hpp>

namespace soemdsp::runtime
{

inline void printDspBindingReport(
  const DspBindingReport& report,
  std::ostream& os = std::cout)
{
    os << "[DSP BINDING REPORT]\n";
    printDspObjectBinding(report.binding, os);
    printDspBindingValidation(report.structuralValidation, os);
    printDspBindingTargetValidation(report.targetValidation, os);
}

} // namespace soemdsp::runtime
