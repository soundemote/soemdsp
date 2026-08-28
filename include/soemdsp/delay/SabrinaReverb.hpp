#pragma once

// Atom meta: SabrinaReverb
// Runtime currently lives in sandbox native_modules/sabrina_reverb (SIMD).
// This header owns kParams — source of truth for Live vs Control + ranges.
// See docs/DSP_ATOM_PARADIGM.md.

#include <cstdint>

#include <soemdsp/dsp/AtomParam.hpp>

namespace soemdsp::delay {

struct SabrinaReverb
{
    static constexpr soemdsp::dsp::AtomParam kParams[] = {
      { 0, "mix", "Mix", 0.5, 0.0, 1.0, "", soemdsp::dsp::ParamKind::Live, "" },
      { 1, "diffusionSize", "Diffuse Size", 0.35, 0.0, 1.0, "", soemdsp::dsp::ParamKind::Control, "geometryChanged" },
      { 2, "diffusionAmount", "Diffuse Amt", 0.7, 0.0, 0.98, "", soemdsp::dsp::ParamKind::Live, "" },
      { 3, "delaySize", "Delay Size", 0.35, 0.0, 1.0, "", soemdsp::dsp::ParamKind::Control, "geometryChanged" },
      { 4, "recycle", "Recycle", 0.5, 0.0, 0.98, "", soemdsp::dsp::ParamKind::Live, "" },
      { 5, "lfoAmplitude", "LFO Amp", 0.1, 0.0, 1.0, "", soemdsp::dsp::ParamKind::Control, "geometryChanged" },
      { 6, "lfoBaseSpeed", "LFO Speed", 0.5, 0.0, 1.0, "", soemdsp::dsp::ParamKind::Control, "geometryChanged" },
      { 7, "lfoVariation", "LFO Vary", 0.5, 0.0, 1.0, "", soemdsp::dsp::ParamKind::Control, "geometryChanged" },
      { 8, "seed", "Seed", 0.0, 0.0, 999.0, "", soemdsp::dsp::ParamKind::Control, "diffusionSeedChanged" },
      { 9, "sampleRate", "Sample Rate", 48000.0, 8000.0, 192000.0, "Hz", soemdsp::dsp::ParamKind::Control, "sampleRateChanged" },
    };
    static constexpr std::uint32_t kControlCount =
      sizeof(kParams) / sizeof(kParams[0]);
};

} // namespace soemdsp::delay
