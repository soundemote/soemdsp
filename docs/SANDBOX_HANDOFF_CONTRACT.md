# Sandbox Handoff Contract

This document describes the current demo-local artifact handoff between `soemdsp` proofs and a future `soemdsp-sandbox`.

The handoff is read-only inspection metadata. It is not a project format, patch format, scheduler contract, audio engine contract, plugin API, or promise that the sandbox should own DSP object execution.

## Current Contract

The current manifest contract is:

```text
soemdsp-demo-local-sandbox-handoff
```

Current version:

```text
1
```

The first producer is:

```text
runtime_dsp_object_bound_wav_resync_demo
```

It writes:

```text
runtime_dsp_object_bound_wav_resync_demo.manifest.json
```

## Required Fields

The `sandboxHandoff` object records:

- `contract`
- `contractVersion`
- `entryPoint`
- `primaryAudioArtifact`
- `inspectionMode`
- `callerOwnsProcessingOrder`
- `callerOwnsDspObjects`
- `circuitOwnsDspObjects`
- `dspObjectsKnowCircuit`
- `serializesPatch`
- `ownsAudioEngine`
- `ownsScheduler`

## Meaning

`entryPoint` names the local HTML inspection entry point.

`primaryAudioArtifact` names the local WAV artifact intended for listening.

`inspectionMode` names the intended human inspection style. The current value is:

```text
mouse-and-ears
```

The ownership flags must preserve the current boundary:

```text
Circuit does not own concrete DSP objects.
DSP objects do not know Circuit.
Binding is the bridge.
```

## Non-Meanings

A future sandbox may read this manifest to find local inspection artifacts.

A future sandbox must not infer that this manifest is:

- a patch/project serialization format
- a scheduler plan
- an audio engine API
- a plugin binding API
- a runtime node ownership model
- a command to execute DSP processing

## Current Safe Consumer Behavior

A safe first sandbox consumer may:

- read the JSON manifest
- display `allOk`
- link or open `sandboxHandoff.entryPoint`
- show `sandboxHandoff.primaryAudioArtifact`
- render `artifactLinks` as a display-only artifact list
- display parameter changes from `parameterResync`
- display phase status from `phases`
- display phase ranges from `startFrame` and `endFrame`
- display artifact status from `wav`

A safe first sandbox consumer should not:

- instantiate DSP objects from the manifest
- schedule processing from the manifest
- mutate Circuit from the manifest
- save the manifest as a project file
- treat demo-local paths as portable project paths

`docs/SANDBOX_HANDOFF_CONSUMER_CHECKLIST.md` records the current accept/display/reject checklist for a future read-only sandbox shell.

## Display-Ready Artifact Links

The manifest may include an `artifactLinks` array for UI shells that want to display local inspection artifacts without hardcoding every filename.

Each item records:

- `label`
- `kind`
- `path`

These links are still local inspection metadata. They do not describe execution order, patch contents, ownership, plugin wiring, or scheduling.

## Phase Display Ranges

Phase objects may include `startFrame` and `endFrame` fields.

These are display ranges for read-only inspection shells. They let a sandbox draw phase spans against the generated WAV without deriving those spans from processing order.

They do not define a scheduler, graph execution order, or reusable processing API.

## Parameter Resync Values

The manifest may include a `parameterResync` object.

This object records demo-local before/after parameter values already used by the generated artifact.

It is read-only display metadata. It does not authorize a sandbox to mutate Circuit, instantiate DSP objects, or treat the manifest as preset/project state.
