#pragma once
#include <memory>
#include <vector>
#include <soemdsp/runtime/Connection.hpp>
#include <soemdsp/runtime/Node.hpp>
namespace soemdsp::runtime
{
struct Graph
{
    std::vector<std::unique_ptr<Node>> nodes;
    std::vector<Connection> connections;
    void process()
{
    for (auto& node : nodes)
    {
        // Pull connected values into this node before it runs
        for (auto& connection : connections)
        {
            if (connection.destinationNode == node.get() &&
                connection.sourcePort &&
                connection.destinationPort)
            {
                connection.destinationPort->value =
                    connection.sourcePort->value;
            }
        }

        node->process();
    }
}
};
} //namespace soemdsp::runtime
