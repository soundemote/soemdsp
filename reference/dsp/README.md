# DSP Reference Objects

This folder contains reference-only DSP object examples.

These files are not part of the public runtime API yet, and they are not automatically built. They act as north-star examples for low-level soemdsp DSP object design.

Current reference:
- `noise.hpp`

`noise.hpp` demonstrates:
- plain structs
- externally owned memory
- base offsets
- small semantic accessors
- reset behavior
- realtime-safe style
- BYOD-friendly DSP atoms

Runtime graph nodes may eventually wrap, describe, bind, and schedule these objects. Low-level DSP objects are not graph nodes.

Reference files should stay small, readable, and code-shaped. They should not become framework-heavy examples.
