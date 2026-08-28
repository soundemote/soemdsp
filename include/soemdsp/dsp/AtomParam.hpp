#pragma once

// Shared param-slot descriptor for DSP atoms.
// Lives next to the DSP class as a constexpr table (source of truth for
// keys / defaults / ranges). UI layers may mirror or generate from this.

#include <cstddef>
#include <string_view>

namespace soemdsp::dsp {

enum class ParamKind : int
{
    Live,    // read every sample / every process — no *Changed
    Control  // may need *Changed / sync when value changes
};

struct AtomParam
{
    std::size_t slot{};
    std::string_view key{};
    std::string_view label{};
    double def{};
    double min{};
    double max{};
    std::string_view unit{};
    ParamKind kind{ ParamKind::Control };
    std::string_view changed{}; // name of *Changed to call, or "" if Live
};

} // namespace soemdsp::dsp
