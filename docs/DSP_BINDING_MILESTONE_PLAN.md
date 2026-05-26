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

DSP binding target validation now rejects null memory slots before apply writes any external memory. This preserves all-or-nothing behavior when an invalid later parameter binding appears after a valid earlier one.

DSP binding target validation also rejects unsupported target kinds before apply writes external memory. `SemanticSetter`, `InitValue`, `ResetValue`, and `ControlInput` remain metadata-only until explicit support is designed.

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

The block resync demo now reports each caller-owned block pass with a separate `DspBlockPhaseReport`, so both the initial pass and the resynced pass expose preflight/apply/process status.

Caller-owned demos can preflight intended block resync bindings before writing any external memory. If validation fails, the caller can skip the block pass instead of partially updating DSP state. This remains demo-local safety proof, not a production batch API.

After a failed preflight, a caller can correct the binding and run a later block pass successfully. This proves failed preflight status does not poison caller-owned DSP memory or future processing attempts.

The same recovery shape now covers null DSP memory slots: a caller-owned block pass can fail preflight before any memory write, preserve existing external memory, then recover when the memory slot is corrected.

A demo-local block phase report can summarize the caller-owned preflight, apply, and process phases. This points toward future sandbox/editor status surfaces without adding a scheduler or reusable executor.

The block phase report now has a tiny reusable `DspBlockPhaseReport` value type and print helper. It reports caller-owned phase counts only; it does not introduce an executor, scheduler, or production batch API.

`DspBlockPhaseReport` can also be written as a text file for inspection, matching the existing runtime report/export pattern while staying reporting-only.

The preflight failure block demo now uses the same report value to show a failed preflight and skipped process phase without applying external memory changes.

The binding proof can now produce an audible artifact: `runtime_dsp_object_bound_wav_demo` syncs Circuit frequency and amplitude parameters into caller-owned external memory, lets a caller-owned `TinySineDsp` render samples from that memory, and writes a short demo-local WAV file. This still does not introduce a scheduler, audio engine, production batch API, graph-owned DSP state, or plugin/UI layer.

The audible binding proof can also resync: `runtime_dsp_object_bound_wav_resync_demo` renders one half of a WAV from initially synced Circuit parameters, changes those parameters through Circuit setters, reapplies the binding, and renders the second half from updated external DSP memory. Each half now reports preflight, apply, and process counts through `DspBlockPhaseReport`, keeping the future sandbox status surface explicit without introducing execution ownership.

The audible demos now share an examples-only mono WAV writer helper. This removes repeated demo code while keeping file rendering outside the runtime API. The helper can also emit a compact write report with path, frame count, sample rate, channel count, bit depth, data bytes, file bytes, and success status for future sandbox-style status inspection.

The resync WAV demo now writes a combined demo-local render summary that joins setter status, first/second phase reports, and WAV artifact metadata. This is a sandbox-status proof only; it is not a scheduler, executor, or project serialization format.

The resync WAV demo manifest now also carries structured `parameterResync` values for frequency and amplitude. This lets read-only sandbox shells display parameter changes without scraping the human text summary, while still keeping the manifest inspection-only.

The same resync WAV demo also writes a demo-local HTML audio report with a browser-native audio control and compact phase/artifact status. This is the first mouse-and-ears oriented artifact, while still avoiding a sandbox app, web server, scheduler, audio engine, or production UI.

The resync WAV demo now writes a demo-local JSON artifact manifest that ties together the generated WAV, WAV metadata report, phase reports, text summary, and HTML report. This manifest is for inspection only and is not a patch/project serialization format.

The HTML audio report now links the generated WAV, manifest, text summary, WAV metadata report, and phase reports, making the demo output behave like one local inspection packet rather than a loose set of files.

The artifact manifest now includes a demo-local sandbox handoff contract. It names the HTML report as the mouse-and-ears entry point, names the WAV as the primary audio artifact, and records that the demo does not own scheduling, audio engine behavior, patch serialization, Circuit-owned DSP objects, or DSP-object knowledge of Circuit. `docs/SANDBOX_HANDOFF_CONTRACT.md` documents the current read-only contract and the inferences a sandbox must not make from it.

The manifest also includes a display-ready `artifactLinks` array so a sandbox shell can render the local inspection packet without hardcoding artifact filenames. These links remain inspection metadata only.

Phase report artifact links now include a demo-local `phase` field so read-only consumers can prove each manifest phase has exactly one matching report artifact without inferring phase identity from filenames.

The manifest writer now keeps nested phase and artifact link objects consistently indented. This is artifact readability only; it does not change the manifest contract or introduce a serialization layer.

The manifest also carries the primary WAV channel count, bit depth, and data byte count from the demo-local WAV write report so a read-only sandbox can validate basic audio metadata without inventing an audio engine.

The manifest now carries explicit `startFrame` and `endFrame` values for each render phase. These are read-only display ranges for sandbox waveform overlays, not scheduler order, graph execution, or project serialization.

The manifest now also carries demo-local `phaseAudioMeasurements` for each render phase: measured frequency, peak, RMS, min, max, and DC offset derived from the samples the caller produced. This lets a read-only sandbox compare producer-side measurements with its own decoded WAV measurements without giving the manifest execution authority.

`docs/SANDBOX_HANDOFF_CONSUMER_CHECKLIST.md` records the smallest safe read-only consumer behavior for a sandbox shell, including required boundary flags and warning conditions.

`docs/DSP_EXECUTION_QUESTIONS.md` records the scheduler, batch API, and sandbox questions that must be answered before the demo-local proofs become production execution machinery.

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
