# Control Graph Notes

ControlGraph is future groundwork for macro controls and control-shaping graphs.

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
