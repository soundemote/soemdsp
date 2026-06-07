# Forward/Backward PolyBLEP

Status: design note from the continuity workbench.

## Purpose

Forward/backward PolyBLEP is the oscillator-aware version of the Soundemote
"nothing teleports" edge rule.

It is not a final-output cleanup pass. It belongs inside an oscillator or other
generator that knows exactly when a discontinuity occurs.

The target use case is:

- saw wrap;
- pulse duty edge;
- pulse wrap edge;
- any future phase-driven generator with known discontinuities.

## Core Idea

Classic PolyBLEP already works because it knows the oscillator phase and the
phase increment. Around a discontinuity, it applies a tiny polynomial correction
over the edge width.

Forward/backward PolyBLEP formalizes this as:

```text
If the generator knows the edge, it may correct both before and after the edge.
If the system only sees arbitrary final audio, use a separate repair algorithm.
```

The important distinction is knowledge. A source-agnostic stream repair cannot
know a future edge without lookahead. A phase-driven oscillator can know the
edge at generation time.

## Realtime Classification

Forward/backward PolyBLEP is realtime if implemented with a tiny edge buffer.

It may require delaying output by the correction radius so samples just before
the edge can be corrected symmetrically. For small windows this latency is very
small.

For strict zero-lookahead oscillator output, use ordinary PolyBLEP.

## Reference Behavior

The current correctness target is ordinary phase-known PolyBLEP.

In the continuity workbench, the `F/B PolyBLEP` lane was made equivalent to
`Basic PolyBLEP` after an experimental centered edge-window version failed at
high frequency.

At 44.1 kHz and a 13.5 kHz saw, the corrected workbench version nulls against
Basic PolyBLEP:

```text
diff RMS: 0
max diff: 0
```

This gives us a stable reference before trying another noncausal centered
kernel.

## Basic PolyBLEP Kernel

The standard PolyBLEP correction:

```cpp
inline double polyBlep(double t, double dt)
{
  if (dt <= 0.0)
    return 0.0;

  if (t < dt) {
    const double x = t / dt;
    return x + x - x * x - 1.0;
  }

  if (t > 1.0 - dt) {
    const double x = (t - 1.0) / dt;
    return x * x + x + x + 1.0;
  }

  return 0.0;
}
```

Saw reference:

```cpp
double sawPolyBlep(double phase, double dt)
{
  double y = phase * 2.0 - 1.0;
  y -= polyBlep(phase, dt);
  return y;
}
```

Pulse reference:

```cpp
double pulsePolyBlep(double phase, double dt, double duty = 0.5)
{
  double y = phase < duty ? 1.0 : -1.0;
  y += polyBlep(phase, dt);

  double dutyPhase = phase - duty;
  if (dutyPhase < 0.0)
    dutyPhase += 1.0;

  y -= polyBlep(dutyPhase, dt);
  return y;
}
```

## Forward/Backward Extension

The forward/backward extension should keep the same bandlimited target as
ordinary PolyBLEP, but allow a symmetric edge buffer when latency is acceptable.

Candidate structure:

```text
1. Generate raw phase-driven samples into a small ring buffer.
2. Detect known edge events from oscillator phase, not from output samples.
3. Store edge position, jump size, and local phase increment.
4. Apply a correction kernel centered on the fractional edge position.
5. Emit samples after the correction window is complete.
```

Edge event:

```cpp
struct KnownEdge
{
  double samplePosition; // fractional sample where discontinuity occurs
  double jump;           // after - before
  double dt;             // phase increment at the edge
};
```

The correction kernel must not introduce new discontinuities at its boundaries.
This was the failure mode of the first prototype.

## Failed Prototype Note

The first workbench attempt used a generic centered `smootherstep` transition
around the known discontinuity.

That sounded worse than Basic PolyBLEP around 13.5 kHz and also sounded quieter.
The cause was:

```text
bad centered edge ramp -> amplitude spike -> normalizeIfNeeded() scales whole signal down
```

The lesson:

```text
Do not use arbitrary smoothing ramps as an anti-aliasing kernel.
Use a bandlimited correction target.
```

## Relationship To Other Repairs

Forward/backward PolyBLEP:

- oscillator-aware;
- phase-driven;
- anti-aliasing during generation;
- realtime with small edge latency;
- not appropriate for arbitrary final output.

Three-point repair:

- source-agnostic;
- one-sample lookahead;
- final-output continuity repair;
- not as informed as oscillator-side PolyBLEP.

Forward/backward acceleration cleanup:

- source-agnostic;
- block/render oriented;
- useful for experiments and export quality;
- not a replacement for oscillator-side anti-aliasing.

4x brickwall oversampling:

- generator/render baseline;
- prevents aliasing by generating high and filtering down;
- FIR latency and CPU cost matter for realtime use.

RAPT elliptic subband filtering:

- 12th-order elliptic IIR lowpass intended for steep subband splitting;
- useful as a very steep anti-aliasing / anti-imaging comparison;
- selected as the current preferred realtime oversampling filter candidate;
- not a mathematically perfect brickwall;
- not linear phase;
- can be much cheaper than long FIR filtering, but phase response and ringing
  should be listened to carefully.

The 129-tap FIR/windowed-sinc filter should stay as a reference/render comparison
unless it is redesigned with more taps or a more appropriate transition band.

## Implementation Requirements

Before adding this to the C++ DSP library:

- keep Basic PolyBLEP as the reference target;
- add a 44.1 kHz high-frequency saw/pulse test around 8 kHz to 16 kHz;
- compare RMS, peak, and residual difference against Basic PolyBLEP;
- avoid automatic normalization inside the oscillator test path;
- test sweep and fixed-frequency hold modes separately;
- include pulse duty edges, not only saw wraps.

## Open Question

The useful invention may not be "PolyBLEP but backwards" if it exactly nulls
against ordinary PolyBLEP. The useful direction may be:

```text
known-edge correction kernel + tiny edge latency + SIMD-friendly batch layout
```

That gives Soundemote a clean framework for phase-known discontinuities while
leaving source-agnostic universe repair as a separate algorithm family.
