#pragma once

#include <memory>
#include <vector>
#include <soemdsp/runtime/Connection.hpp>
#include <soemdsp/runtime/Node.hpp>

namespace soemdsp::runtime {

struct Graph {
    std::vector<std::unique_ptr<Node>> nodes;
    std::vector<Connection> connections;

    void process() {
    //1. Push values across connections
        for (auto& connection : connections) {
            if (!connection.sourcePort || !connection.destinationPort) {
                continue;
            }

            connection.destinationPort->value =
              connection.sourcePort->value;
        }

    //2. Process nodes
        for (auto& node : nodes) {
            node->process();
        }
    }
};

} //namespace soemdsp::runtime
