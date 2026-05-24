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
- parameter midValue is preserved as metadata and is not used for clamping
- parameter midValue remains UI/perceptual metadata only
- runtime parameter setter demo
- parameter setter demo proves clamped metadata mutation through Circuit helpers

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
