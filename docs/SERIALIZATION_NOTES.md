# Serialization Notes

Future serialization will likely need:
- node ids
- node typeName()
- editor positions
- port metadata
- connections
- parameter values
- graph versioning

Runtime parameters store minValue, midValue, and maxValue. The midpoint is intended to preserve musical/perceptual control feel for future UI mapping and serialization.

Local port IDs are intentional. A NodeID + PortID uniquely identifies a port.

CircuitSnapshot exists as a plain value-model bridge for future serialization and export work. Circuit::snapshot() captures graph metadata into plain structs, including nodes, ports, parameters, connections, layout metadata, and parameter minValue/midValue/maxValue.

PrintCircuitSnapshot.hpp provides compact text inspection for CircuitSnapshot data. This is debug/export groundwork, not a committed serialization format.

WriteCircuitSnapshot.hpp can dump CircuitSnapshot text to a file for inspection, sharing, and early export workflows. This is not a committed serialization or save format, and no load path exists yet.

CircuitSnapshot is groundwork only. It is not a committed file format, parser, or load/save system.

Serialization is not implemented yet.
