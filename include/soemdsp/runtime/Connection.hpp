#pragma once

#include <cstddef>

namespace soemdsp::runtime {

struct Node;
struct Port;

struct Connection {
    Node* sourceNode { nullptr };
    Port* sourcePort { nullptr };

    Node* destinationNode { nullptr };
    Port* destinationPort { nullptr };
};

} // namespace soemdsp::runtime
