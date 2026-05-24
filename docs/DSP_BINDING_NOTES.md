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
`reference/dsp/noise.hpp` shows `SplitMix64` operating over externally owned memory through `wires` plus `base`.

Conceptually:
- `base + 0` is state
- `base + 1` is seed
- a future runtime node might expose seed as a Parameter
- `reset()` copies seed into state
- editor changes should not directly mutate hidden execution state unless the binding explicitly says so

## Not Decided Yet
- Whether binding descriptors live beside DSP objects or outside them
- Whether binding is compile-time, runtime, or hybrid
- Whether memory slots should keep the name `wires`
- Whether parameters push immediately to DSP memory or are applied at safe sync points
- How audio-thread safety is handled
- How scripting sees bound objects
- How CLAP/web/export layers consume bindings

## Non-Goals For Now
- No active DSP execution binding
- No audio engine changes
- No plugin parameter integration
- No serialization format lock-in
- No UI implementation
