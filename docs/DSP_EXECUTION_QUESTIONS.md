# DSP Execution Questions

This document captures the questions that must be answered before `soemdsp` grows a production scheduler, executor, or batch processing API.

The current DSP binding demos are caller-owned proofs. They show useful behavior, but they are not scheduler design.

## Current Proofs

The current proof ladder shows:

- Circuit parameters can sync into externally owned DSP memory.
- Low-level DSP objects can read externally owned memory without knowing Circuit.
- Multiple `DspObjectBinding` objects can be applied sequentially by a caller.
- Caller-owned DSP object chains can process single samples.
- Caller-owned DSP object chains can resync changed Circuit parameters.
- Caller-owned DSP object chains can process fixed blocks.
- Caller-owned block passes can preflight intended bindings before writing memory.
- Caller-owned block passes can produce compact phase reports for preflight, apply, and process counts.
- Caller-owned block phase reports can be printed and written as text files for inspection.
- Caller-owned block preflight failures can report skipped processing without mutating external DSP memory.

These proofs are deliberately local.

They do not create:

- a scheduler
- a production batch API
- graph-owned DSP object state
- plugin binding
- audio engine ownership
- a reusable executor

## Non-Negotiable Boundaries

Always preserve:

```text
Circuit does not own concrete DSP objects.
DSP objects do not know Circuit.
Binding is the bridge.
```

DSP binding apply remains raw runtime sync.

ControlGraph owns musical/perceptual shaping.

Circuit normalized parameter transport remains raw linear transport.

## Scheduler Questions

Before adding a scheduler, answer:

- What exactly is being scheduled: runtime graph nodes, DSP object calls, binding sync points, or all of them?
- Who owns concrete DSP object instances?
- Who owns external DSP memory?
- How does a runtime node map to one or more DSP objects?
- Are processing order and binding order always the same?
- When does parameter sync happen relative to audio processing?
- How are control-rate, audio-rate, and trigger-rate paths separated?
- How does validation gate processing before any memory is written?
- How are partial failures represented without hiding successful bindings?
- What status does a future editor/sandbox need to display?
- How does the design remain realtime-safe?
- How does the design avoid plugin, UI, or host assumptions?

## Batch API Questions

Before adding a production batch API, answer:

- Is batch apply only parameter sync, or does it include processing?
- Does a batch fail all-or-nothing, per binding, or per object?
- How are per-binding diagnostics exposed?
- How are aggregate diagnostics exposed?
- Can callers choose strict preflight before apply?
- Can callers inspect failed bindings without side effects?
- Does the API own any memory or DSP object lifetime?
- Does the API imply a scheduler?

Until these answers are clear, keep batch behavior demo-local.

## Sandbox Questions

Before `soemdsp-sandbox` depends on this layer, answer:

- What should a sandbox node own?
- What should a sandbox patch/project serialize?
- How should users see binding status, preflight status, apply status, and process status?
- What is the smallest visible sandbox proof that benefits from the current binding demos?
- Which parts of the phase report are user-facing and which are debug-only?
- How should sandbox undo/preset state interact with raw Circuit parameter values?
- Where does ControlGraph shaping enter before runtime parameter sync?

## Current Safe Next Moves

Allowed next moves:

- more demo-local reports
- more caller-owned processing proofs
- clearer docs around ownership and phase boundaries
- read-only inspection of sandbox/editor needs
- small value types only if repeated demos make them unavoidable
- reporting/export helpers that do not own or execute DSP phases

Blocked without explicit Architect/Vision approval:

- scheduler
- production batch API
- graph-owned DSP object storage
- plugin binding layer
- shared execution framework
- cross-repo dependency

## Minimal Rule

Keep proving behavior.

Name the phase boundaries.

Do not extract the executor until the shape is undeniable.
