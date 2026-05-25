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
