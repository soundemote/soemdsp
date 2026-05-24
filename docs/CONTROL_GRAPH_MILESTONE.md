# ControlGraph Milestone: Macro Control to Runtime Parameter

## What now works
A macro/control value can enter a ControlGraph, move through a simple control chain, and be shaped with curve metadata. Parameter min/mid/max metadata can generate midpoint curve settings, so musical midpoint intent can be represented explicitly in the control graph.

ControlGraph can validate its own structure, validate intended Circuit parameter targets, and safely apply shaped values into Circuit parameters. Apply reports can also be generated and exported as text for diagnostics.

## Why this matters
This proves the core sandbox idea: average-user controls can drive programmer-authored behavior graphs.

Raw Circuit normalized transport remains linear and predictable. Musical and perceptual shaping lives in ControlGraph. `midValue` now has a demonstrated purpose without contaminating raw parameter assignment.

## Current limitations
- evaluator is intentionally minimal and linear-chain oriented
- Curve and Scale are basic
- no full graph scheduler yet
- no UI
- no DSP binding
- no plugin automation binding
- no save/load format yet
- no scripting yet

## Next likely steps
- formalize ControlGraph execution later
- add save/load after snapshot/report foundations
- connect to real DSP object bindings
- eventually expose macro controls in sandbox UI
- eventually let Asciiscope, Prettyscope, and Void visualize/control these signals
