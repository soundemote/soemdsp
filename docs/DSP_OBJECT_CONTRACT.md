# DSP Object Contract

`reference/dsp/noise.hpp` is the current north-star example for low-level soemdsp DSP object design.

This file is reference-only. It is not part of the build, runtime graph, editor, plugin API, or production API yet.

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
- expose clear behavior methods like next(), reset(), process(), etc.
- be easy to wrap with runtime/editor metadata from outside
- remain compatible with BYOD DSP

In the reference file, the name `wires` currently means low-level external DSP memory, not graph-editor connections. This naming may be refined later if it causes confusion.
