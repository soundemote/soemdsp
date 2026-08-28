# DSP Atom Paradigm (Iteration 1 — copy this)

Reference modules:
- `delay/ModulatedDelay.hpp`, `delay/Reverb.hpp` — full process atoms
- `delay/PingPongDelay.hpp` — full process atom
- `delay/SabrinaReverb.hpp` — **kParams meta** (SIMD runtime still in sandbox)
- `dsp/AtomParam.hpp`

## Rules

1. **One DSP class per file**, header-only when practical.
2. **`kParams[]` lives in the DSP file** — keys, label, def/min/max, unit, `Live` vs `Control`, and which `*Changed` to call.
3. **`mem[base + slot]`** — host/control storage. Caller owns the buffer.
4. **`Live` params** — read every `process()` (or passed as args into children). **No `connect()` / `pointTo` / per-sample push.**
5. **`Control` params** — only rebuild in `fooChanged()` / `syncControlParams()` when the value actually changed.
6. **Fixed caps** — `kMaxDelays`, `kMaxBufferSamples`, etc. Contiguous slabs. No `vector` resize on audio thread.
7. **Children** — leaf atoms (`ModulatedDelay`, filters). Parent passes **live** values as function arguments.

## Live vs Control (Reverb)

| Live (args / read in process) | Control (*Changed) |
|-------------------------------|--------------------|
| mix, volume, recycle | echoTime → `echoTimeChanged` |
| diffusionAmount | diffusionSize → `diffusionSizeChanged` |
| lfoAmp | seed → `diffusionSeedChanged` |
| echoMode, pingPong | numDelays → `numDelaysChanged` |
| | lfoFrequency → `lfoFrequencyChanged` |
| | lfoVariation → `lfoVariationChanged` |
| | doModulateEcho → `doModulateEchoChanged` (topology flag) |
| | saturate → `clippingThresholdChanged` |
| | sampleRate / filters → matching `*Changed` |

## Skeleton to copy

```cpp
struct MyAtom {
  static constexpr soemdsp::dsp::AtomParam kParams[] = { /* ... */ };
  static constexpr uint32_t kControlCount = /* ... */;

  double* mem = nullptr;
  uint32_t base{};

  double& foo_() noexcept { return mem[base + 0]; }

  void fooChanged() noexcept { /* rebuild coeffs */ }
  void syncControlParams() noexcept { /* if changed → *Changed */ }
  void reset() noexcept;
  void process(/* audio */) noexcept; // read Live; use cached Control
};
```

## Explicit non-goals (iteration 1)

- No op-list / calculation bytecode interpreter
- No patch-wide global memory arena
- No DirtyUpdater `std::function` tables on atoms
- No baking circuit→module yet (later; dump can use `kParams` + process structure)

## Sandbox

- UI meta may still live in `node-graph-module-definitions.js` until codegen exists.
- Native `set_params` must gate the same way as `syncControlParams`.
- Native `process` must pass Live values into lines each sample (see `soem_reverb`).

## Migration checklist (per module)

- [ ] Add `kParams[]` with Live/Control marks
- [ ] Split `*Changed` to match original ownership (SoEmReverb-style)
- [ ] Remove connect/pushLiveWires; pass Live as args
- [ ] Fixed slab for state
- [ ] Sandbox `set_params` field-gated; `process` live args
- [ ] Demo or smoke that Control rebuilds once, Live does not
