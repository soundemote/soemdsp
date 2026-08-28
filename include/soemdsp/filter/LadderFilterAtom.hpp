#pragma once

// Atom meta: LadderFilter
// Sandbox native owns the process kernel; this header owns kParams.
// See docs/DSP_ATOM_PARADIGM.md.

#include <cstdint>
#include <soemdsp/dsp/AtomParam.hpp>

namespace soemdsp::filter::atom {

struct LadderFilter
{
    static constexpr soemdsp::dsp::AtomParam kParams[] = {
      { 0, "mode", "Mode", 1.0, 0.0, 3.0, "", soemdsp::dsp::ParamKind::Control, "mixChanged" },
      { 1, "frequency", "Frequency", 1000.0, 0.0, 20000.0, "Hz", soemdsp::dsp::ParamKind::Control, "coeffsChanged" },
      { 2, "resonance", "Resonance", 0.2, 0.0, 0.999, "", soemdsp::dsp::ParamKind::Control, "coeffsChanged" },
      { 3, "stages", "Stages", 4.0, 1.0, 4.0, "", soemdsp::dsp::ParamKind::Control, "mixChanged" },
      { 4, "sampleRate", "Sample Rate", 48000.0, 8000.0, 192000.0, "Hz", soemdsp::dsp::ParamKind::Control, "coeffsChanged" },
    };
    static constexpr std::uint32_t kControlCount =
      sizeof(kParams) / sizeof(kParams[0]);
};

} // namespace soemdsp::filter::atom
