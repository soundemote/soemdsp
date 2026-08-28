# DSP Reference Objects

This folder points at north-star low-level DSP object examples.

Current entry:

- `noise.hpp` → includes production `<soemdsp/random/NoiseAtoms.hpp>`

Those atoms demonstrate:

- plain structs
- externally owned memory (`mem` + `base`)
- small semantic accessors
- reset / selective sync (`syncSeed`)
- realtime-safe style
- BYOD-friendly DSP atoms

Runtime graph nodes may wrap, describe, bind, and schedule these objects. Low-level DSP objects are not graph nodes.

See `docs/DSP_OBJECT_CONTRACT.md` for the frozen contract (including selective `*Changed` updates and why `DirtyUpdater` is not the default).
