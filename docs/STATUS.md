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
