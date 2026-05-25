#pragma once

#include <cstddef>
#include <soemdsp/runtime/dsp/ApplyDspBinding.hpp>

namespace soemdsp::runtime
{

struct DspBindingApplySummary
{
    bool ok{ true };
    std::size_t parametersApplied{ 0 };
    std::size_t messageCount{ 0 };
};

inline DspBindingApplySummary makeDspBindingApplySummary(
  const DspBindingApplyResult& result)
{
    return {
      result.ok,
      result.parametersApplied,
      result.messages.size()
    };
}

} // namespace soemdsp::runtime
