# Control Graph Notes

ControlGraph is future groundwork for macro controls and control-shaping graphs.

The first ControlGraph value model exists in `include/soemdsp/runtime/control/ControlGraph.hpp`. It captures control nodes and connections only. Execution, evaluation, parameter binding, serialization, and editor behavior remain future work.

The value model also has basic validation diagnostics for duplicate node IDs, empty node names, missing connection endpoints, and empty connection port IDs. It now has snapshot, text export, combined report helpers, and a first limited evaluator for inspection. Binding remains future work.

The first evaluator is intentionally narrow. It follows a single-output linear chain, clamps incoming macro values to 0.0-1.0, treats Curve, Scale, and Smooth as pass-through placeholders for now, and emits a ParameterTarget output without mutating Circuit parameters.

ParameterTarget metadata is part of the value model. A ParameterTarget node can record the intended Circuit node id and parameter id it may drive later, but actual Circuit binding and mutation remain future work.

The first apply helper uses ParameterTarget metadata and `Circuit::setParameterNormalizedValue()` to prove macro knob -> ControlGraph -> normalized parameter set. This remains a narrow proof helper, not the final execution or binding architecture. Mapping is still raw linear transport and does not use `midValue` yet.

Target validation is separate from ControlGraph structural validation. It checks intended Circuit node/parameter targets before apply, and it does not execute, evaluate, or mutate anything.

Safe apply is the preferred helper for demos and tools that want to mutate Circuit parameters. It gates structural validation and target validation before applying; warnings currently do not block apply, but errors do.

The core idea is that an editor/user knob can output normalized 0.0-1.0 control, pass through a graph of shaping nodes, and drive one or more target parameters:

```text
Knob 0.0-1.0
-> curve/ease/midpoint shaping
-> clamp 0.0-1.0
-> scale/map
-> parameter target
```

Existing `setParameterNormalizedValue()` helpers are raw linear transport. ControlGraph is where musical and perceptual shaping belongs. `midValue` remains preserved metadata for future shaping and mapping, but existing normalized parameter helpers should remain unchanged.

Likely future node concepts:
- ControlValue / MacroKnob
- Curve
- Clamp01
- Scale
- Invert
- Smooth
- ParameterTarget
- Split / fanout

Possible future uses:
- macro knobs
- MIDI CC mapping
- automation shaping
- UI control feel
- multi-parameter performance controls
- average-user musical controls over programmer-authored graphs

Non-goals for now:
- no implementation yet
- no DSP execution changes
- no UI editor changes
- no plugin automation binding yet
- no scripting integration yet
