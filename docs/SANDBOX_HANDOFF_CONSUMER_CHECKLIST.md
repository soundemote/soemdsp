# Sandbox Handoff Consumer Checklist

This checklist describes the smallest safe read-only consumer for the current demo-local sandbox handoff manifest.

It is for a future `soemdsp-sandbox` shell. It is not an implementation requirement for `soemdsp` runtime code, and it does not introduce a JSON parser, scheduler, audio engine, plugin layer, or project format.

## Input

The current producer writes:

```text
runtime_dsp_object_bound_wav_resync_demo.manifest.json
```

The current expected contract is:

```text
soemdsp-demo-local-sandbox-handoff
```

The current expected contract version is:

```text
1
```

## Accept

A read-only consumer may accept the manifest when:

- `allOk` is `true`
- `sandboxHandoff.contract` is `soemdsp-demo-local-sandbox-handoff`
- `sandboxHandoff.contractVersion` is `1`
- `sandboxHandoff.inspectionMode` is `mouse-and-ears`
- `sandboxHandoff.entryPoint` is present
- `sandboxHandoff.primaryAudioArtifact` is present
- `sandboxHandoff.callerOwnsProcessingOrder` is `true`
- `sandboxHandoff.callerOwnsDspObjects` is `true`
- `sandboxHandoff.circuitOwnsDspObjects` is `false`
- `sandboxHandoff.dspObjectsKnowCircuit` is `false`
- `sandboxHandoff.serializesPatch` is `false`
- `sandboxHandoff.ownsAudioEngine` is `false`
- `sandboxHandoff.ownsScheduler` is `false`
- `artifactLinks` contains at least one `entry-point`
- `artifactLinks` contains at least one `audio`
- `phases` contains at least one phase report

## Display

A read-only consumer may display:

- overall status from `allOk`
- the HTML entry point from `sandboxHandoff.entryPoint`
- the primary WAV from `sandboxHandoff.primaryAudioArtifact`
- display-only artifact rows from `artifactLinks`
- phase status from `phases`
- WAV status from `wav`

## Reject Or Warn

A read-only consumer should reject or warn when:

- contract name is unknown
- contract version is unsupported
- ownership flags imply Circuit owns DSP objects
- ownership flags imply DSP objects know Circuit
- scheduler ownership is true
- audio engine ownership is true
- patch serialization is true
- expected entry point or audio artifact paths are missing

## Must Not Do

A read-only consumer must not:

- instantiate DSP objects from the manifest
- schedule processing from the manifest
- mutate Circuit from the manifest
- write project files from the manifest
- treat demo-local paths as portable project paths
- infer plugin or host behavior from artifact links

## Current Verification Shape

Until `soemdsp-sandbox` exists locally, verification can parse the generated manifest with external tooling and assert the fields above. That verifies the artifact packet without adding a production JSON dependency to `soemdsp`.
