# DSP Binding Milestone Plan

## Current foundation
The DSP object contract exists, and `reference/dsp/noise.hpp` is the north-star low-level DSP object reference. Circuit graph metadata exists, and parameters can be reflected, set, reset, snapshotted, validated, reported, and exported.

ControlGraph can now safely shape macro controls into Circuit parameter changes. That gives the runtime/editor side a practical control path before DSP execution binding is introduced.

## Binding goal
Runtime graph nodes should eventually wrap, describe, bind, and schedule low-level DSP objects without forcing those DSP objects to own graph, editor, plugin, or UI state.

## First prototype target
The first practical prototype should be a tiny binding around a simple object inspired by `reference/dsp/noise.hpp`.

The first value model layer now exists as `DspObjectBinding` and `DspParameterBinding`. It describes intended runtime-parameter-to-DSP-object targets, and it now has basic validation diagnostics. Binding validation is split into structural binding validation and Circuit target validation. Execution, apply, and scheduling remain future work.

DSP binding now mirrors the Circuit and ControlGraph report pattern with a combined binding report for metadata plus validation diagnostics.

Metadata validation has advanced to a safe parameter value sync proof: Circuit parameter values can be copied through validated binding metadata into externally owned DSP memory slots. Execution, scheduling, and DSP processing remain future work.

The apply path also has a focused failure demo for missing Circuit parameter targets and null DSP memory slots. These cases fail safely without applying parameters or mutating external memory.

DSP binding apply results now mirror the text export and reporting pattern used by Circuit, ControlGraph, and binding reports.

The boundary now has a contract-style DSP object proof: a Circuit parameter can sync into externally owned memory, and a low-level DSP object can read that memory without knowing about Circuit or owning graph/editor state.

The one-shot sync proof has advanced to a repeated resync proof: after a Circuit parameter changes through the runtime setter path, the same binding can be applied again to refresh external DSP memory.

The binding proof now covers multiple Circuit parameters syncing into multiple external DSP memory slots consumed by one low-level DSP object.

One DSP object binding can also pull parameter values from multiple Circuit nodes into separate external memory slots for the same low-level DSP object.

DSP binding apply validates all targets before writing memory, so invalid multi-node bindings fail without partially updating external DSP state.

DSP binding apply reporting now has both full result text export and compact summary output for tools, demos, and future editor status surfaces.

Multiple `DspObjectBinding` objects can be applied sequentially by a caller today. This is a demo-level batch proof only; final scheduler design remains deferred.

Callers can also inspect mixed per-binding results when applying multiple bindings sequentially, so a successful binding does not hide a later failed binding.

Per-binding apply summaries can be aggregated by callers for demo-level batch status while final scheduler and batch API design remain deferred.

Synced external DSP memory can now drive a caller-owned manual processing chain of low-level DSP objects. This proves parameter sync can feed actual object processing while scheduler design remains deferred.

Changed Circuit parameters can be resynced into external DSP memory and then drive updated caller-owned manual processing output, still without introducing scheduler ownership.

Synced external DSP memory can also drive caller-owned manual block processing through low-level DSP objects. The caller owns object instances, processing order, and the sample loop while scheduler design remains deferred.

Changed Circuit parameters can also be resynced into external DSP memory before another caller-owned block pass. This proves block processing can react to runtime parameter changes without adding a scheduler, production batch API, or graph-owned DSP state.

Example milestone:
- a runtime node represents a small DSP object
- the DSP object owns no editor state
- external memory/state is provided separately
- runtime parameters configure seed, rate, amplitude, or similar
- a small process/evaluate function produces sample or control output
- a demo proves parameter changes affect the DSP object through an explicit binding step

## Important boundaries
- Low-level DSP objects are not graph nodes.
- Runtime graph nodes are metadata/control wrappers.
- The binding layer connects them.
- Do not merge the layers.
- Do not make DSP objects depend on Circuit.
- Do not make Circuit depend on any one DSP object family.

## Suggested first implementation phases
1. define tiny DSP binding descriptor/value model
2. define external DSP memory/state holder
3. create one demo binding for a noise or constant object
4. apply Circuit parameter values to DSP object state at a safe sync point
5. produce output samples/control values in an example
6. report/export binding metadata later

## Non-goals
- no full scheduler yet
- no audio engine yet
- no plugin binding yet
- no UI
- no scripting
- no automatic code generation
- no general BYOD system yet
