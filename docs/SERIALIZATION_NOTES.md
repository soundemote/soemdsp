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

Serialization is not implemented yet.
