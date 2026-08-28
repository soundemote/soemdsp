# DSP Binding Notes

These notes describe how runtime graph metadata may eventually bind to low-level DSP objects such as `reference/dsp/noise.hpp`.

This is design documentation only. No binding layer is implemented yet.

## Current Separation
Low-level DSP object:
- code-shaped signal/state atom
- plain struct
- external memory / base offset
- realtime-safe
- no graph/editor/plugin ownership

Runtime graph node:
- stable id
- typeName
- display name
- ports
- parameters
- layout
- debug/serialization metadata

Binding layer:
- not implemented yet
- future layer between graph metadata and DSP execution objects

## Possible Binding Targets
A runtime Parameter may later bind to:
- a DSP memory slot
- a semantic setter
- a constructor/init value
- a reset seed/default
- an execution-time control input

A runtime Port may later bind to:
- audio buffer input/output
- control-rate value
- trigger/event edge
- external signal memory

## Important Rule
Runtime parameters are editor-facing/control-facing metadata.

DSP memory slots are execution-facing state/control storage.

They may correspond, but they are not automatically the same thing.

## SplitMix64 Example

Production atom: `soemdsp::random::atom::SplitMix64` in `include/soemdsp/random/NoiseAtoms.hpp`
(`reference/dsp/noise.hpp` includes that header).

Operates over externally owned memory through **`mem` + `base`**:

- `base + 0` is state
- `base + 1` is seed
- a runtime node may expose seed as a Parameter
- `reset()` copies seed into state
- `syncSeed()` resets only when `seed_()` differs from the last applied seed
- editor changes should not directly mutate hidden execution state unless the binding explicitly says so

Current `DspParameterBinding` apply writes `float` slots. For `uint64_t` seed slots, the caller reads the Circuit parameter and writes the integer seed into `mem`, then calls `syncSeed()` / `reset()`.

Selective derived updates use explicit `*Changed()` / `sync*()` methods. Do not default to `DirtyUpdater` `std::function` tables on atoms; see `docs/DSP_OBJECT_CONTRACT.md`.

## Not Decided Yet

- Whether binding descriptors live beside DSP objects or outside them
- Whether binding is compile-time, runtime, or hybrid
- Whether float-only MemorySlot apply grows typed slots
- Whether parameters push immediately to DSP memory or are applied at safe sync points
- How audio-thread safety is handled
- How scripting sees bound objects
- How CLAP/web/export layers consume bindings

## Non-Goals For Now

- No production scheduler
- No audio engine ownership in the library demos
- No plugin parameter integration
- No serialization format lock-in
- No UI implementation
