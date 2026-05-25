#pragma once

#include <cstddef>

namespace soemdsp::runtime
{

struct DspBlockPhaseReport
{
    bool preflightOk{ true };
    bool applyOk{ true };
    bool processOk{ false };
    std::size_t bindingsChecked{ 0 };
    std::size_t preflightMessages{ 0 };
    std::size_t parametersApplied{ 0 };
    std::size_t applyMessages{ 0 };
    std::size_t samplesProcessed{ 0 };
};

} // namespace soemdsp::runtime

