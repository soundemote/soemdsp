# soemdsp Status

## Current State
soemdsp currently has a runtime circuit with nodes, ports, and connections.

Implemented runtime capabilities include:
- control, audio, and trigger port types
- stable node IDs, local port IDs, and connection IDs
- prepare/reset/process lifecycle
- persistent audio buffers
- audio-rate modulation
- trigger routing
- disconnect support
- parameter lookup helpers
- parameter value setter helpers
- parameter values clamp to minValue/maxValue
- parameter reset helpers
- parameter reset uses defaultValue clamped to minValue/maxValue
- normalized parameter helpers
- normalized parameter mapping is currently linear
- parameter dirty/default helpers
- parameter dirty/default comparison uses exact float comparison for now
- parameter dirty/default helpers support future editor reset, highlight, and preset diff behavior
- parameter count helpers
- dirty parameter count supports future editor badges, preset diff UI, and save-state indicators
- resetAllParameterValues helper
- resetAllParameterValues returns the number of changed parameters reset
- resetAllParameterValues supports future global editor reset, preset cleanup, and save-state cleanup behavior
- parameter midValue is preserved as metadata and is not used for clamping
- parameter midValue remains UI/perceptual metadata only
- parameter midValue is intentionally not used by normalized helpers yet
- runtime parameter setter demo
- parameter setter demo proves clamped metadata mutation through Circuit helpers
- reference/dsp/noise.hpp exists as the low-level DSP object north-star reference
- docs/DSP_OBJECT_CONTRACT.md documents low-level DSP object design principles
- DSP object reference files are reference-only and not part of the build/runtime yet
- duplicate reference/noise.hpp upload copy was removed after moving the reference to reference/dsp/noise.hpp
- DSP_OBJECT_CONTRACT distinguishes low-level DSP objects, runtime graph nodes, and a future binding layer
- docs/DSP_BINDING_NOTES.md documents the future runtime-to-DSP binding layer
- DSP binding layer is documented but intentionally not implemented
- reference/dsp/README.md documents the DSP reference folder
- reference/dsp is documented as reference-only DSP north-star material

Reflection metadata exists for nodes:
- name
- category
- description
- displayName()
- summary()

Port reflection exists:
- name
- description
- displayName()
- summary()

Debug helpers exist:
- printNodes
- printPorts
- printConnections
- printCircuit
- printLayout
- printParameters
- printAudio
- validateCircuit
- node search/sort helpers

## Next Direction
- move toward editor-ready graph metadata
- then serialization
- then playable audio sandbox
