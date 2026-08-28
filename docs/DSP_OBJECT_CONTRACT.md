# DSP Object Contract

Low-level DSP objects are not graph nodes. They are code-shaped signal/state atoms that graph nodes may wrap, describe, bind, and schedule.

**Copy/paste rulebook:** `docs/DSP_ATOM_PARADIGM.md` (iteration 1).

Production atoms (accessors → `*Changed()` → `syncControlParams()` → `reset()` / `process*()`, with `kParams[]` meta in-file):

- `include/soemdsp/dsp/AtomParam.hpp`
- `include/soemdsp/random/NoiseAtoms.hpp`
- `include/soemdsp/dynamics/SoftClipperAtom.hpp`
- `include/soemdsp/dynamics/LookaheadLimiterAtom.hpp`
- `include/soemdsp/filter/OnePoleHpAtom.hpp`
- `include/soemdsp/filter/BiquadCascadeAtom.hpp`
- `include/soemdsp/delay/ModulatedDelay.hpp` — **leaf reference**
- `include/soemdsp/delay/Reverb.hpp` — **composite reference**

`reference/dsp/noise.hpp` points at the noise atoms. Legacy Wire-based delay/reverb sources were removed. Live params are function args (no `connect()`).

## Low-Level DSP Object

Low-level DSP objects should:

- be plain structs
- be realtime-safe
- avoid heap allocation
- avoid exceptions
- avoid virtual dispatch in the hot path
- avoid owning editor, graph, plugin, or UI state
- operate over externally owned memory when useful
- use base offsets into external memory when useful
- expose small semantic accessors for their state
- expose clear behavior methods like `next()`, `reset()`, `process()`, `*Changed()`, etc.
- be easy to wrap with runtime/editor metadata from outside
- remain compatible with BYOD DSP

### External memory naming

Prefer **`mem` + `base`** for externally owned DSP memory slots.

Do not call these graph/editor cables. Sandbox cable routing is a separate layer and stays as-is.

Legacy objects may still use `Wire` (`pointTo` / local storage). New atoms should use `mem` + `base`.

### Selective updates (`*Changed`)

Derived state that is expensive to recompute (`exp`, tables, bank rebuilds) belongs in explicit `fooChanged()` methods, not in the per-sample hot path.

Who calls them (best → good enough):

1. Host/setter already knows the param changed → call `fooChanged()` directly.
2. Host only writes `mem` slots → atom `syncControlParams()` / `syncSeed()` with plain last-value compares.
3. Avoid as default: per-object `std::function` callback tables (`DirtyUpdater` style).

`plugin/DirtyUpdater.hpp` remains available for polyphonic deferral (idle voices). It is not the default update mechanism for mono atoms or sandbox modules.

## Runtime Graph Node

Runtime graph nodes are the reflected/editor-facing layer. They may own or expose:

- stable node IDs
- ports and connections
- parameters
- display metadata
- editor layout metadata
- debug and serialization support

Graph nodes can describe, schedule, and connect DSP behavior without forcing low-level DSP objects to own graph/editor concerns.

## Binding Layer

Binding maps runtime graph ports and parameters to low-level DSP object memory, semantic setters, or processing calls.

Externally owned parameter memory slots are valid binding targets. Binding code may copy already-resolved runtime parameter values into those slots at explicit sync points without making the DSP object own graph/editor state.

Current apply path: `DspObjectBinding` / `applyDspParameterBindings` (float `memorySlot` targets). Non-float slots (e.g. `uint64_t` seeds) may be synced by the caller after reading Circuit parameter values.

Ownership boundary:

```text
Circuit does not own concrete DSP objects.
DSP objects do not know Circuit.
Binding is the bridge.
```

## Sandbox

Sandbox any-to-any cable routing is unchanged by this contract.

Sandbox modules should consume atoms by writing control slots only when values change, then calling `process()` / `next()` every sample.
